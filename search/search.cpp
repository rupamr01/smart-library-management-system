#include "search.h"
#include "../book/book.h"
#include "../student/student.h"
#include "../transaction/transaction.h"
#include "../common/config.h"
#include "../common/validation.h"
#include <iostream>
#include <iomanip>
using namespace std;

static void printBookHeader()
{
    cout << left
         << setw(5) << "ID"
         << setw(30) << "Title"
         << setw(22) << "Author"
         << setw(10) << "Year"
         << setw(7) << "Total"
         << setw(10) << "Available" << endl;

    cout << string(84, '-') << endl;
}

static void printBookRow(const BookInfo& book)
{
    cout << left
         << setw(5) << book.id
         << setw(30) << fitText(book.title, 29)
         << setw(22) << fitText(book.author, 21)
         << setw(10) << book.publicationYear
         << setw(7) << book.totalCopies
         << setw(10) << book.availableCopies << endl;
}

static bool bookComesBefore(const BookInfo& first, const BookInfo& second,
                            BookSortKey key, bool ascending)
{
    int comparison = 0;

    if (key == BookSortKey::Title)
    {
        comparison = toLower(first.title).compare(toLower(second.title));
    }
    else if (key == BookSortKey::Author)
    {
        comparison = toLower(first.author).compare(toLower(second.author));
    }
    else if (key == BookSortKey::Year)
    {
        if (first.publicationYear < second.publicationYear)
        {
            comparison = -1;
        }
        else if (first.publicationYear > second.publicationYear)
        {
            comparison = 1;
        }
    }
    else
    {
        if (first.id < second.id)
        {
            comparison = -1;
        }
        else if (first.id > second.id)
        {
            comparison = 1;
        }
    }

    if (comparison == 0)
    {
        return first.id < second.id;
    }

    return ascending ? comparison < 0 : comparison > 0;
}

static void mergeSortRecursive(BookInfo* books, int left, int right,
                               BookInfo* temp, BookSortKey key, bool ascending)
{
    if (left >= right)
    {
        return;
    }

    int middle = left + (right - left) / 2;
    mergeSortRecursive(books, left, middle, temp, key, ascending);
    mergeSortRecursive(books, middle + 1, right, temp, key, ascending);

    int i = left;
    int j = middle + 1;
    int k = left;

    while (i <= middle && j <= right)
    {
        if (bookComesBefore(books[i], books[j], key, ascending))
        {
            temp[k++] = books[i++];
        }
        else
        {
            temp[k++] = books[j++];
        }
    }

    while (i <= middle)
    {
        temp[k++] = books[i++];
    }

    while (j <= right)
    {
        temp[k++] = books[j++];
    }

    for (int index = left; index <= right; ++index)
    {
        books[index] = temp[index];
    }
}

void mergeSortBooks(BookInfo* books, int count, BookSortKey key, bool ascending)
{
    if (books == nullptr || count <= 1)
    {
        return;
    }

    BookInfo* temp = new BookInfo[count];
    mergeSortRecursive(books, 0, count - 1, temp, key, ascending);
    delete[] temp;
}

int binarySearchBookById(const BookInfo* sortedBooks, int count, int id)
{
    int low = 0;
    int high = count - 1;

    while (low <= high)
    {
        int middle = low + (high - low) / 2;

        if (sortedBooks[middle].id == id)
        {
            return middle;
        }

        if (id < sortedBooks[middle].id)
        {
            high = middle - 1;
        }
        else
        {
            low = middle + 1;
        }
    }

    return -1;
}

int collectMatchingBooks(const BookInfo* books, int count, const string& query,
                         bool matchTitle, int* indices, int maxIndices)
{
    int matches = 0;

    for (int i = 0; i < count; ++i)
    {
        const string& field = matchTitle ? books[i].title : books[i].author;

        if (containsIgnoreCase(field, query))
        {
            if (matches < maxIndices)
            {
                indices[matches] = i;
            }

            ++matches;
        }
    }

    return matches;
}

void searchBooksMenu()
{
    int bookCount = getTotalBookTitles();

    if (bookCount == 0)
    {
        cout << "No books available to search." << endl;
        return;
    }

    BookInfo* books = new BookInfo[bookCount];
    collectBooks(books, bookCount);

    while (true)
    {
        cout << "\n--- Search Books ---" << endl;
        cout << "1. Search by ID (binary search)" << endl;
        cout << "2. Search by title" << endl;
        cout << "3. Search by author" << endl;
        cout << "4. Show available books" << endl;
        cout << "0. Back" << endl;

        int choice = 0;

        if (!readInt("Enter your choice: ", choice))
        {
            break;
        }

        if (choice == 0)
        {
            break;
        }

        if (choice == 1)
        {
            int id = 0;

            if (!readInt("Enter book ID: ", id))
            {
                break;
            }

            mergeSortBooks(books, bookCount, BookSortKey::Id, true);
            int index = binarySearchBookById(books, bookCount, id);

            if (index == -1)
            {
                cout << "Book not found." << endl;
            }
            else
            {
                printBookHeader();
                printBookRow(books[index]);
            }
        }
        else if (choice == 2 || choice == 3)
        {
            string query;

            if (!readNonEmptyLine(choice == 2 ? "Enter title text: " : "Enter author text: ", query))
            {
                break;
            }

            int* indices = new int[bookCount];
            int matches = collectMatchingBooks(books, bookCount, query, choice == 2, indices, bookCount);

            if (matches == 0)
            {
                cout << "No matching books found." << endl;
            }
            else
            {
                printBookHeader();

                for (int i = 0; i < matches && i < bookCount; ++i)
                {
                    printBookRow(books[indices[i]]);
                }
            }

            delete[] indices;
        }
        else if (choice == 4)
        {
            bool found = false;
            printBookHeader();

            for (int i = 0; i < bookCount; ++i)
            {
                if (books[i].availableCopies > 0)
                {
                    printBookRow(books[i]);
                    found = true;
                }
            }

            if (!found)
            {
                cout << "No copies are currently available." << endl;
            }
        }
        else
        {
            cout << "Invalid choice." << endl;
        }
    }

    delete[] books;
}

void sortBooksMenu()
{
    int bookCount = getTotalBookTitles();

    if (bookCount == 0)
    {
        cout << "No books available to sort." << endl;
        return;
    }

    BookInfo* books = new BookInfo[bookCount];
    collectBooks(books, bookCount);

    while (true)
    {
        cout << "\n--- Sort Books ---" << endl;
        cout << "1. Sort by title" << endl;
        cout << "2. Sort by author" << endl;
        cout << "3. Sort by publication year" << endl;
        cout << "4. Sort by ID" << endl;
        cout << "0. Back" << endl;

        int choice = 0;

        if (!readInt("Enter your choice: ", choice) || choice == 0)
        {
            break;
        }

        BookSortKey key = BookSortKey::Title;

        if (choice == 1)
        {
            key = BookSortKey::Title;
        }
        else if (choice == 2)
        {
            key = BookSortKey::Author;
        }
        else if (choice == 3)
        {
            key = BookSortKey::Year;
        }
        else if (choice == 4)
        {
            key = BookSortKey::Id;
        }
        else
        {
            cout << "Invalid choice." << endl;
            continue;
        }

        cout << "1. Ascending" << endl;
        cout << "2. Descending" << endl;

        int direction = 0;

        if (!readInt("Enter direction: ", direction))
        {
            break;
        }

        if (direction != 1 && direction != 2)
        {
            cout << "Invalid direction." << endl;
            continue;
        }

        mergeSortBooks(books, bookCount, key, direction == 1);
        printBookHeader();

        for (int i = 0; i < bookCount; ++i)
        {
            printBookRow(books[i]);
        }
    }

    delete[] books;
}

void displayLibraryReports()
{
    int totalTitles = getTotalBookTitles();
    int totalCopies = getTotalCopies();
    int availableCopies = getAvailableCopies();
    int issuedCopies = getIssuedCopies();
    int students = getStudentCount();
    int activeLoans = getActiveLoanCount();
    int reservations = getPendingReservationCount();

    cout << "\n========== LIBRARY REPORT ==========" << endl;
    cout << left << setw(28) << "Total book titles" << totalTitles << endl;
    cout << left << setw(28) << "Total physical copies" << totalCopies << endl;
    cout << left << setw(28) << "Available copies" << availableCopies << endl;
    cout << left << setw(28) << "Issued copies" << issuedCopies << endl;
    cout << left << setw(28) << "Registered students" << students << endl;
    cout << left << setw(28) << "Active loans" << activeLoans << endl;
    cout << left << setw(28) << "Pending reservations" << reservations << endl;

    if (issuedCopies != activeLoans)
    {
        cout << "\nWarning: issued copy count and active loan count differ." << endl;
    }

    cout << "\nRecent activity:" << endl;
    displayRecentTransactions(Config::RECENT_HISTORY_COUNT);
}
