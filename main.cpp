#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "book/book.h"
#include "student/student.h"
#include "transaction/transaction.h"
#include "search/search.h"
#include "storage/storage.h"
#include "common/config.h"
#include "common/validation.h"
#include "common/date_utils.h"
using namespace std;

static bool dataWarnings = false;

static string money(double value)
{
    ostringstream output;
    output << fixed << setprecision(2) << value;
    return output.str();
}

static bool readIntWithDefault(const string& prompt, int current, int& output)
{
    while (true)
    {
        string line;

        if (!readLine(prompt + " [" + to_string(current) + "]: ", line))
        {
            return false;
        }

        if (line.empty())
        {
            output = current;
            return true;
        }

        if (parseInt(line, output))
        {
            return true;
        }

        cout << "Invalid number. Please try again." << endl;
    }
}

static bool readTextWithDefault(const string& prompt, const string& current, string& output)
{
    if (!readLine(prompt + " [" + current + "]: ", output))
    {
        return false;
    }

    if (output.empty())
    {
        output = current;
    }

    return true;
}

static void showMainMenu()
{
    cout << "\n========================================" << endl;
    cout << "       SMART LIBRARY MANAGEMENT" << endl;
    cout << "========================================" << endl;
    cout << "1. Book Management" << endl;
    cout << "2. Student Management" << endl;
    cout << "3. Issue Book" << endl;
    cout << "4. Return Book" << endl;
    cout << "5. Search Books" << endl;
    cout << "6. Sort Books" << endl;
    cout << "7. Reservation Management" << endl;
    cout << "8. Transaction History" << endl;
    cout << "9. Library Reports" << endl;
    cout << "10. Save Data" << endl;
    cout << "0. Save and Exit" << endl;
    cout << "========================================" << endl;
}

static void displayBookDetails(const Book& book)
{
    cout << "\nBook ID        : " << book.id << endl;
    cout << "Title          : " << book.title << endl;
    cout << "Author         : " << book.author << endl;
    cout << "Publisher      : " << book.publisher << endl;
    cout << "Publication year: " << book.publicationYear << endl;
    cout << "Total copies   : " << book.totalCopies << endl;
    cout << "Available      : " << book.availableCopies << endl;
}

static void displayStudentDetails(const Student& student)
{
    cout << "\nStudent ID : " << student.id << endl;
    cout << "Name       : " << student.name << endl;
    cout << "Course     : " << student.course << endl;
    cout << "Phone      : " << student.phone << endl;
}

static void addBookFlow()
{
    int id = 0;
    string title;
    string author;
    string publisher;

    if (!readInt("Book ID: ", id) ||
        !readNonEmptyLine("Title: ", title) ||
        !readNonEmptyLine("Author: ", author) ||
        !readLine("Publisher (optional): ", publisher))
    {
        return;
    }

    int year = 0;
    int copies = 0;

    if (!readInt("Publication year: ", year) ||
        !readInt("Total copies: ", copies))
    {
        return;
    }

    string error;

    if (addBook(id, title, author, publisher, year, copies, error))
    {
        cout << "Book added successfully." << endl;
    }
    else
    {
        cout << "Error: " << error << endl;
    }
}

static void updateBookFlow()
{
    int id = 0;

    if (!readInt("Book ID to update: ", id))
    {
        return;
    }

    const Book* book = findBookById(id);

    if (book == nullptr)
    {
        cout << "Book not found." << endl;
        return;
    }

    string title;
    string author;
    string publisher;
    int year = 0;
    int copies = 0;

    if (!readTextWithDefault("Title", book->title, title) ||
        !readTextWithDefault("Author", book->author, author) ||
        !readTextWithDefault("Publisher", book->publisher, publisher) ||
        !readIntWithDefault("Publication year", book->publicationYear, year) ||
        !readIntWithDefault("Total copies", book->totalCopies, copies))
    {
        return;
    }

    string error;

    if (updateBook(id, title, author, publisher, year, copies, error))
    {
        cout << "Book updated successfully." << endl;
    }
    else
    {
        cout << "Error: " << error << endl;
    }
}

static void deleteBookFlow()
{
    int id = 0;

    if (!readInt("Book ID to delete: ", id))
    {
        return;
    }

    if (findBookById(id) == nullptr)
    {
        cout << "Book not found." << endl;
        return;
    }

    if (hasActiveLoanForBook(id))
    {
        cout << "Cannot delete: this book has active loans." << endl;
        return;
    }

    if (hasPendingReservationForBook(id))
    {
        cout << "Cannot delete: this book has pending reservations." << endl;
        return;
    }

    if (hasAnyTransactionForBook(id))
    {
        cout << "Cannot delete: transaction history references this book." << endl;
        return;
    }

    string error;

    if (deleteBook(id, error))
    {
        cout << "Book deleted successfully." << endl;
    }
    else
    {
        cout << "Error: " << error << endl;
    }
}

static void findBookFlow()
{
    int id = 0;

    if (!readInt("Book ID: ", id))
    {
        return;
    }

    const Book* book = findBookById(id);

    if (book == nullptr)
    {
        cout << "Book not found." << endl;
        return;
    }

    displayBookDetails(*book);
}

static void bookMenu()
{
    while (true)
    {
        cout << "\n--- Book Management ---" << endl;
        cout << "1. Add book" << endl;
        cout << "2. Update book" << endl;
        cout << "3. Delete book" << endl;
        cout << "4. Display all books" << endl;
        cout << "5. Find book by ID" << endl;
        cout << "0. Back" << endl;

        int choice = 0;

        if (!readInt("Enter your choice: ", choice) || choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            addBookFlow();
        }
        else if (choice == 2)
        {
            updateBookFlow();
        }
        else if (choice == 3)
        {
            deleteBookFlow();
        }
        else if (choice == 4)
        {
            displayAllBooks();
        }
        else if (choice == 5)
        {
            findBookFlow();
        }
        else
        {
            cout << "Invalid choice." << endl;
        }
    }
}

static void addStudentFlow()
{
    int id = 0;
    string name;
    string course;
    string phone;

    if (!readInt("Student ID: ", id) ||
        !readNonEmptyLine("Name: ", name) ||
        !readLine("Course (optional): ", course) ||
        !readLine("Phone (optional): ", phone))
    {
        return;
    }

    string error;

    if (addStudent(id, name, course, phone, error))
    {
        cout << "Student registered successfully." << endl;
    }
    else
    {
        cout << "Error: " << error << endl;
    }
}

static void updateStudentFlow()
{
    int id = 0;

    if (!readInt("Student ID to update: ", id))
    {
        return;
    }

    const Student* student = findStudentById(id);

    if (student == nullptr)
    {
        cout << "Student not found." << endl;
        return;
    }

    string name;
    string course;
    string phone;

    if (!readTextWithDefault("Name", student->name, name) ||
        !readTextWithDefault("Course", student->course, course) ||
        !readTextWithDefault("Phone", student->phone, phone))
    {
        return;
    }

    string error;

    if (updateStudent(id, name, course, phone, error))
    {
        cout << "Student updated successfully." << endl;
    }
    else
    {
        cout << "Error: " << error << endl;
    }
}

static void deleteStudentFlow()
{
    int id = 0;

    if (!readInt("Student ID to delete: ", id))
    {
        return;
    }

    if (findStudentById(id) == nullptr)
    {
        cout << "Student not found." << endl;
        return;
    }

    if (hasActiveLoanForStudent(id))
    {
        cout << "Cannot delete: this student has active loans." << endl;
        return;
    }

    if (hasPendingReservationForStudent(id))
    {
        cout << "Cannot delete: this student has pending reservations." << endl;
        return;
    }

    string error;

    if (deleteStudent(id, error))
    {
        cout << "Student deleted successfully." << endl;
    }
    else
    {
        cout << "Error: " << error << endl;
    }
}

static void findStudentFlow()
{
    int id = 0;

    if (!readInt("Student ID: ", id))
    {
        return;
    }

    const Student* student = findStudentById(id);

    if (student == nullptr)
    {
        cout << "Student not found." << endl;
        return;
    }

    displayStudentDetails(*student);
}

static void studentMenu()
{
    while (true)
    {
        cout << "\n--- Student Management ---" << endl;
        cout << "1. Register student" << endl;
        cout << "2. Update student" << endl;
        cout << "3. Delete student" << endl;
        cout << "4. Display students in ID order" << endl;
        cout << "5. Find student by ID" << endl;
        cout << "0. Back" << endl;

        int choice = 0;

        if (!readInt("Enter your choice: ", choice) || choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            addStudentFlow();
        }
        else if (choice == 2)
        {
            updateStudentFlow();
        }
        else if (choice == 3)
        {
            deleteStudentFlow();
        }
        else if (choice == 4)
        {
            displayStudentsInOrder();
        }
        else if (choice == 5)
        {
            findStudentFlow();
        }
        else
        {
            cout << "Invalid choice." << endl;
        }
    }
}

static void issueBookFlow()
{
    int studentId = 0;
    int bookId = 0;

    if (!readInt("Student ID: ", studentId) || !readInt("Book ID: ", bookId))
    {
        return;
    }

    string today = todayString();
    string error;

    if (issueBook(studentId, bookId, today, error))
    {
        int transactionId = findActiveLoanForStudentBook(studentId, bookId);
        cout << "Book issued successfully." << endl;
        cout << "Transaction ID: " << transactionId << endl;
        cout << "Due date: " << addDays(today, Config::LOAN_PERIOD_DAYS) << endl;
        return;
    }

    cout << "Issue failed: " << error << endl;

    const Book* book = findBookById(bookId);

    if (error == "No copies are available." && book != nullptr && findStudentById(studentId) != nullptr)
    {
        if (readYesNo("Add this student to the reservation queue? (y/n): "))
        {
            string reserveError;

            if (reserveBook(studentId, bookId, reserveError))
            {
                cout << "Reservation added." << endl;
            }
            else
            {
                cout << "Reservation failed: " << reserveError << endl;
            }
        }
    }
}

static void returnBookFlow()
{
    displayActiveLoans();

    int transactionId = 0;

    if (!readInt("Transaction ID to return: ", transactionId))
    {
        return;
    }

    string returnDate;
    readLine("Return date (YYYY-MM-DD, blank for today): ", returnDate);

    if (returnDate.empty())
    {
        returnDate = todayString();
    }

    ReturnOutcome outcome;
    string error;

    if (!returnBook(transactionId, returnDate, outcome, error))
    {
        cout << "Return failed: " << error << endl;
        return;
    }

    cout << "Book returned successfully." << endl;
    cout << "Fine: Rs. " << money(outcome.fine) << endl;

    if (outcome.nextStudentId != 0)
    {
        cout << "Allocated to student " << outcome.nextStudentId
             << " (new transaction " << outcome.newTransactionId << ")." << endl;
    }

    if (outcome.skippedReservations > 0)
    {
        cout << "Skipped " << outcome.skippedReservations
             << " ineligible reservation(s)." << endl;
    }
}

static void reservationMenu()
{
    while (true)
    {
        cout << "\n--- Reservation Management ---" << endl;
        cout << "1. Add reservation" << endl;
        cout << "2. View reservation queues" << endl;
        cout << "3. Cancel reservation" << endl;
        cout << "0. Back" << endl;

        int choice = 0;

        if (!readInt("Enter your choice: ", choice) || choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            int studentId = 0;
            int bookId = 0;

            if (!readInt("Student ID: ", studentId) || !readInt("Book ID: ", bookId))
            {
                return;
            }

            string error;

            if (reserveBook(studentId, bookId, error))
            {
                cout << "Reservation added." << endl;
            }
            else
            {
                cout << "Error: " << error << endl;
            }
        }
        else if (choice == 2)
        {
            displayReservationQueues();
        }
        else if (choice == 3)
        {
            int studentId = 0;
            int bookId = 0;

            if (!readInt("Student ID: ", studentId) || !readInt("Book ID: ", bookId))
            {
                return;
            }

            string error;

            if (cancelReservation(studentId, bookId, error))
            {
                cout << "Reservation cancelled." << endl;
            }
            else
            {
                cout << "Error: " << error << endl;
            }
        }
        else
        {
            cout << "Invalid choice." << endl;
        }
    }
}

static void historyMenu()
{
    while (true)
    {
        cout << "\n--- Transaction History ---" << endl;
        cout << "1. Recent activity" << endl;
        cout << "2. All transactions" << endl;
        cout << "3. Active loans" << endl;
        cout << "0. Back" << endl;

        int choice = 0;

        if (!readInt("Enter your choice: ", choice) || choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            displayRecentTransactions(Config::RECENT_HISTORY_COUNT);
        }
        else if (choice == 2)
        {
            displayAllTransactions();
        }
        else if (choice == 3)
        {
            displayActiveLoans();
        }
        else
        {
            cout << "Invalid choice." << endl;
        }
    }
}

static bool saveData(bool confirmWarnings)
{
    if (confirmWarnings && dataWarnings)
    {
        if (!readYesNo("Data was loaded with warnings. Save anyway? (y/n): "))
        {
            return false;
        }
    }

    string error;

    if (!saveAllData(Config::DATA_DIR, error))
    {
        cout << "Save failed: " << error << endl;
        return false;
    }

    cout << "Data saved successfully." << endl;
    dataWarnings = false;
    return true;
}

int main()
{
    LoadReport report;

    if (!loadAllData(Config::DATA_DIR, report))
    {
        cout << "Could not load library data." << endl;

        if (!report.warnings.empty())
        {
            cout << report.warnings << endl;
        }

        return 1;
    }

    dataWarnings = report.hadWarnings;

    cout << "Loaded " << report.books << " book(s), "
         << report.students << " student(s), "
         << report.transactions << " transaction(s), "
         << report.reservations << " reservation(s)." << endl;

    if (dataWarnings)
    {
        cout << "\nData warnings:" << endl;
        cout << report.warnings << endl;
    }

    bool running = true;

    while (running)
    {
        showMainMenu();

        int choice = 0;

        if (!readInt("Enter your choice: ", choice))
        {
            cout << "\nInput closed." << endl;

            if (dataWarnings)
            {
                cout << "Data was loaded with warnings; not saving automatically." << endl;
            }
            else
            {
                saveData(false);
            }

            break;
        }

        if (choice == 1)
        {
            bookMenu();
        }
        else if (choice == 2)
        {
            studentMenu();
        }
        else if (choice == 3)
        {
            issueBookFlow();
        }
        else if (choice == 4)
        {
            returnBookFlow();
        }
        else if (choice == 5)
        {
            searchBooksMenu();
        }
        else if (choice == 6)
        {
            sortBooksMenu();
        }
        else if (choice == 7)
        {
            reservationMenu();
        }
        else if (choice == 8)
        {
            historyMenu();
        }
        else if (choice == 9)
        {
            displayLibraryReports();
        }
        else if (choice == 10)
        {
            saveData(true);
        }
        else if (choice == 0)
        {
            if (saveData(true))
            {
                running = false;
            }
            else if (readYesNo("Exit without saving? (y/n): "))
            {
                running = false;
            }
        }
        else
        {
            cout << "Invalid choice." << endl;
        }
    }

    clearBooks();
    clearStudents();
    clearTransactions();

    cout << "Goodbye." << endl;
    return 0;
}
