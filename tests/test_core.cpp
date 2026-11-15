#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include "../common/date_utils.h"
#include "../common/validation.h"
#include "../book/book.h"
#include "../student/student.h"
#include "../transaction/transaction.h"
#include "../search/search.h"
#include "../storage/storage.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
using namespace std;

static int testsPassed = 0;
static int testsFailed = 0;

#define CHECK(condition, name)                                      \
    do                                                              \
    {                                                               \
        if (condition)                                              \
        {                                                           \
            ++testsPassed;                                          \
        }                                                           \
        else                                                        \
        {                                                           \
            ++testsFailed;                                          \
            cout << "FAIL: " << name << endl;                       \
        }                                                           \
    } while (0)

static void resetAll()
{
    clearBooks();
    clearStudents();
    clearTransactions();
}

static void removeIfExists(const string& path)
{
    remove(path.c_str());
}

static void cleanupStorage()
{
    removeIfExists("tests/tmpdata/books.txt");
    removeIfExists("tests/tmpdata/students.txt");
    removeIfExists("tests/tmpdata/transactions.txt");
    removeIfExists("tests/tmpdata/reservations.txt");
    removeIfExists("tests/tmpdata/books.txt.bak");
    removeIfExists("tests/tmpdata/students.txt.bak");
    removeIfExists("tests/tmpdata/transactions.txt.bak");
    removeIfExists("tests/tmpdata/reservations.txt.bak");
    removeIfExists("tests/tmpdata/books.txt.tmp");
    removeIfExists("tests/tmpdata/students.txt.tmp");
    removeIfExists("tests/tmpdata/transactions.txt.tmp");
    removeIfExists("tests/tmpdata/reservations.txt.tmp");
#ifdef _WIN32
    _rmdir("tests/tmpdata");
#else
    rmdir("tests/tmpdata");
#endif
}

static void testDates()
{
    int year = 0;
    int month = 0;
    int day = 0;

    CHECK(parseDate("2026-11-15", year, month, day), "valid date parses");
    CHECK(year == 2026 && month == 11 && day == 15, "parsed date values");
    CHECK(!parseDate("2026-02-30", year, month, day), "invalid day rejected");
    CHECK(!parseDate("2026/11/15", year, month, day), "invalid format rejected");
    CHECK(addDays("2024-02-28", 1) == "2024-02-29", "leap day");
    CHECK(addDays("2023-02-28", 1) == "2023-03-01", "non-leap year");
    CHECK(addDays("2026-12-31", 1) == "2027-01-01", "year rollover");
    CHECK(daysBetween("2026-01-01", "2026-01-08") == 7, "days between");
    CHECK(daysBetween("2026-01-08", "2026-01-01") == -7, "negative days between");
}

static void testBooks()
{
    resetAll();
    string error;

    CHECK(addBook(1, "Book One", "Author One", "Publisher", 2020, 2, error), "add first book");
    CHECK(addBook(2, "Book Two", "Author Two", "Publisher", 2021, 1, error), "add second book");
    CHECK(addBook(3, "Book Three", "Author Three", "Publisher", 2022, 3, error), "add third book");
    CHECK(!addBook(1, "Duplicate", "Author", "Publisher", 2020, 1, error), "reject duplicate book ID");
    CHECK(getTotalBookTitles() == 3, "book count");
    CHECK(getTotalCopies() == 6, "total copies");
    CHECK(getAvailableCopies() == 6, "available copies");

    CHECK(changeAvailableCopies(1, -1, error), "decrease available copies");
    CHECK(getIssuedCopies() == 1, "issued copies");
    CHECK(!changeAvailableCopies(1, -10, error), "reject negative availability");

    CHECK(updateBook(2, "Book Two Updated", "Author Two", "Publisher", 2023, 2, error),
          "update book");
    CHECK(findBookById(2)->totalCopies == 2, "updated total copies");
    CHECK(findBookById(2)->availableCopies == 2, "updated available copies");
    CHECK(!updateBook(1, "Book One", "Author One", "Publisher", 2020, 0, error),
          "reject total below issued copies");

    CHECK(deleteBook(2, error), "delete middle book");
    CHECK(findBookById(2) == nullptr, "deleted middle not found");
    CHECK(deleteBook(1, error), "delete tail book");
    CHECK(deleteBook(3, error), "delete head book");
    CHECK(getTotalBookTitles() == 0, "empty book list");
    CHECK(!deleteBook(99, error), "delete missing book");
}

static void testStudents()
{
    resetAll();
    string error;

    CHECK(addStudent(50, "Root", "CSE", "1", error), "insert root");
    CHECK(addStudent(30, "Left", "CSE", "2", error), "insert left child");
    CHECK(addStudent(70, "Right", "CSE", "3", error), "insert right child");
    CHECK(addStudent(20, "Leaf", "CSE", "4", error), "insert leaf");
    CHECK(addStudent(40, "Left Right", "CSE", "5", error), "insert inner node");
    CHECK(addStudent(60, "Right Left", "CSE", "6", error), "insert inner node 2");
    CHECK(addStudent(80, "Right Right", "CSE", "7", error), "insert leaf 2");
    CHECK(!addStudent(50, "Duplicate", "CSE", "8", error), "reject duplicate student ID");
    CHECK(getStudentCount() == 7, "student count");
    CHECK(findStudentById(40) != nullptr, "find existing student");
    CHECK(findStudentById(99) == nullptr, "find missing student");

    CHECK(deleteStudent(20, error), "delete leaf student");
    CHECK(deleteStudent(30, error), "delete one-child student");
    CHECK(findStudentById(30) == nullptr, "one-child student removed");
    CHECK(deleteStudent(50, error), "delete two-child root");
    CHECK(findStudentById(50) == nullptr, "two-child root removed");
    CHECK(findStudentById(60) != nullptr, "successor remains");
    CHECK(getStudentCount() == 4, "student count after deletes");
    CHECK(!deleteStudent(99, error), "delete missing student");
}

static void testTransactions()
{
    resetAll();
    string error;

    CHECK(addBook(1, "Book A", "Author", "Publisher", 2020, 1, error), "transaction setup book");
    CHECK(addStudent(1, "Alice", "CSE", "1", error), "transaction setup student 1");
    CHECK(addStudent(2, "Bob", "CSE", "2", error), "transaction setup student 2");

    CHECK(issueBook(1, 1, "2026-01-01", error), "issue available book");
    int firstTransaction = findActiveLoanForStudentBook(1, 1);
    CHECK(firstTransaction > 0, "active loan found");
    CHECK(getActiveLoanCount() == 1, "active loan count");
    CHECK(!issueBook(2, 1, "2026-01-01", error), "reject issue with no copies");

    CHECK(reserveBook(2, 1, error), "reserve unavailable book");
    CHECK(!reserveBook(2, 1, error), "reject duplicate reservation");
    CHECK(getPendingReservationCount() == 1, "pending reservation count");

    ReturnOutcome outcome;
    CHECK(returnBook(firstTransaction, "2026-01-10", outcome, error), "return overdue book");
    CHECK(outcome.fine == 10.0, "overdue fine calculation");
    CHECK(outcome.nextStudentId == 2, "reservation auto-allocation");
    CHECK(outcome.newTransactionId > 0, "new transaction created");
    CHECK(getActiveLoanCount() == 1, "auto-issued loan active");
    CHECK(getPendingReservationCount() == 0, "queue emptied");
    CHECK(hasAnyTransactionForBook(1), "book transaction history detected");
    CHECK(hasAnyTransactionForStudent(1), "student transaction history detected");
    CHECK(!returnBook(firstTransaction, "2026-01-11", outcome, error), "reject double return");

    HistoryInfo history;
    CHECK(getHistoryCount() > 0, "history populated");
    CHECK(getHistoryEntry(0, history) && history.action == "AUTO-ISSUE", "newest history is auto-issue");

    resetAll();
    CHECK(addStudent(1, "Limit Student", "CSE", "1", error), "limit student setup");

    for (int i = 1; i <= 4; ++i)
    {
        CHECK(addBook(i, "Limit Book " + to_string(i), "Author", "Publisher", 2020, 1, error),
              "limit book setup");
    }

    CHECK(issueBook(1, 1, "2026-01-01", error), "limit loan 1");
    CHECK(issueBook(1, 2, "2026-01-01", error), "limit loan 2");
    CHECK(issueBook(1, 3, "2026-01-01", error), "limit loan 3");
    CHECK(!issueBook(1, 4, "2026-01-01", error), "reject borrowing limit");

    resetAll();
    CHECK(addBook(1, "Duplicate Loan Book", "Author", "Publisher", 2020, 2, error),
          "duplicate loan book setup");
    CHECK(addStudent(1, "Duplicate Loan Student", "CSE", "1", error),
          "duplicate loan student setup");
    CHECK(issueBook(1, 1, "2026-01-01", error), "first copy issued");
    CHECK(!issueBook(1, 1, "2026-01-01", error), "reject duplicate active loan");

    resetAll();
    CHECK(addBook(1, "Cancel Book", "Author", "Publisher", 2020, 1, error),
          "cancel book setup");
    CHECK(addStudent(1, "Owner", "CSE", "1", error), "cancel owner setup");
    CHECK(addStudent(2, "Waiter", "CSE", "2", error), "cancel waiter setup");
    CHECK(issueBook(1, 1, "2026-01-01", error), "cancel issue setup");
    CHECK(reserveBook(2, 1, error), "cancel reservation setup");
    CHECK(cancelReservation(2, 1, error), "cancel reservation");
    CHECK(getPendingReservationCount() == 0, "cancelled reservation removed");

    resetAll();
    CHECK(addBook(1, "Fine Book", "Author", "Publisher", 2020, 1, error), "fine book setup");
    CHECK(addStudent(1, "Fine Student", "CSE", "1", error), "fine student setup");
    CHECK(issueBook(1, 1, "2026-01-01", error), "fine issue setup");
    int fineTransaction = findActiveLoanForStudentBook(1, 1);
    ReturnOutcome fineOutcome;
    CHECK(returnBook(fineTransaction, "2026-01-08", fineOutcome, error), "return on due date");
    CHECK(fineOutcome.fine == 0.0, "zero fine on due date");
    CHECK(getAvailableCopies() == 1, "returned copy available");
}

static void testSearch()
{
    BookInfo books[3];

    books[0].id = 2;
    books[0].title = "Beta";
    books[0].author = "Zed";
    books[0].publicationYear = 2020;
    books[0].totalCopies = 1;
    books[0].availableCopies = 1;

    books[1].id = 1;
    books[1].title = "alpha";
    books[1].author = "Amy";
    books[1].publicationYear = 2019;
    books[1].totalCopies = 1;
    books[1].availableCopies = 0;

    books[2].id = 3;
    books[2].title = "Gamma";
    books[2].author = "Bob";
    books[2].publicationYear = 2021;
    books[2].totalCopies = 1;
    books[2].availableCopies = 1;

    mergeSortBooks(books, 3, BookSortKey::Title, true);
    CHECK(books[0].id == 1 && books[1].id == 2 && books[2].id == 3, "merge sort title ascending");

    mergeSortBooks(books, 3, BookSortKey::Year, false);
    CHECK(books[0].id == 3 && books[1].id == 2 && books[2].id == 1, "merge sort year descending");

    mergeSortBooks(books, 3, BookSortKey::Id, true);
    CHECK(binarySearchBookById(books, 3, 2) == 1, "binary search found");
    CHECK(binarySearchBookById(books, 3, 99) == -1, "binary search missing");

    int indices[3];
    int matches = collectMatchingBooks(books, 3, "amma", true, indices, 3);
    CHECK(matches == 1 && books[indices[0]].id == 3, "linear title search");
    matches = collectMatchingBooks(books, 3, "amy", false, indices, 3);
    CHECK(matches == 1 && books[indices[0]].id == 1, "linear author search");
    matches = collectMatchingBooks(books, 3, "none", true, indices, 3);
    CHECK(matches == 0, "linear search no match");

    BookInfo oneBook = books[0];
    mergeSortBooks(&oneBook, 1, BookSortKey::Title, true);
    CHECK(oneBook.id == books[0].id, "one-item sort safe");
}

static void testStorage()
{
    resetAll();
    cleanupStorage();
    string error;

    CHECK(addBook(1, "Storage Book", "Author", "Publisher", 2020, 2, error),
          "storage book setup");
    CHECK(addStudent(7, "Storage Student", "CSE", "123", error),
          "storage student setup");
    CHECK(issueBook(7, 1, "2026-01-01", error), "storage issue setup");
    CHECK(saveAllData("tests/tmpdata", error), "save all data");

    resetAll();
    LoadReport report;
    CHECK(loadAllData("tests/tmpdata", report), "load saved data");
    CHECK(report.books == 1, "loaded book count");
    CHECK(report.students == 1, "loaded student count");
    CHECK(report.transactions == 1, "loaded transaction count");
    CHECK(findBookById(1) != nullptr, "loaded book lookup");
    CHECK(findStudentById(7) != nullptr, "loaded student lookup");
    CHECK(getActiveLoanCount() == 1, "loaded active loan");
    CHECK(getAvailableCopies() == 1, "loaded availability");

    ofstream broken("tests/tmpdata/books.txt", ios::app);
    broken << "broken|line\n";
    broken.close();

    resetAll();
    CHECK(loadAllData("tests/tmpdata", report), "load with malformed line");
    CHECK(report.hadWarnings, "malformed line warning");
    CHECK(report.books == 1, "malformed line skipped");

    resetAll();
    LoadReport missingReport;
    CHECK(loadAllData("tests/tmpmissing", missingReport), "missing files start empty");
    CHECK(missingReport.books == 0 && missingReport.students == 0, "empty missing data");

    cleanupStorage();
    resetAll();

    CHECK(addBook(1, "Historical Book", "Author", "Publisher", 2020, 1, error),
          "historical book setup");
    CHECK(addStudent(1, "Historical Student", "CSE", "1", error),
          "historical student setup");
    CHECK(issueBook(1, 1, "2026-01-01", error), "historical issue setup");
    int historicalTransaction = findActiveLoanForStudentBook(1, 1);
    ReturnOutcome historicalOutcome;
    CHECK(returnBook(historicalTransaction, "2026-01-08", historicalOutcome, error),
          "historical return setup");
    CHECK(saveAllData("tests/tmpdata", error), "save historical data");
    CHECK(deleteBook(1, error), "delete book after returned loan");
    CHECK(deleteStudent(1, error), "delete student after returned loan");
    CHECK(saveAllData("tests/tmpdata", error), "save with historical references");

    resetAll();
    LoadReport historicalReport;
    CHECK(loadAllData("tests/tmpdata", historicalReport), "load historical references");
    CHECK(historicalReport.transactions == 1, "historical transaction preserved");
    CHECK(historicalReport.hadWarnings, "historical reference warning");
    CHECK(getTransactionCount() == 1, "historical transaction count");

    cleanupStorage();
}

int main()
{
    testDates();
    testBooks();
    testStudents();
    testTransactions();
    testSearch();
    testStorage();

    cout << "\nTests passed: " << testsPassed << ", failed: " << testsFailed << endl;
    return testsFailed == 0 ? 0 : 1;
}
