#include "book.h"
#include "../common/validation.h"
#include <iostream>
#include <iomanip>
using namespace std;

static Book* bookHead = nullptr;
static int bookCount = 0;

static bool validateBookFields(int id, const string& title, const string& author,
                               int publicationYear, int totalCopies,
                               int availableCopies, string& error)
{
    if (id <= 0)
    {
        error = "Book ID must be a positive integer.";
        return false;
    }

    if (trim(title).empty())
    {
        error = "Book title cannot be empty.";
        return false;
    }

    if (trim(author).empty())
    {
        error = "Book author cannot be empty.";
        return false;
    }

    if (publicationYear <= 0 || publicationYear > 2100)
    {
        error = "Publication year must be between 1 and 2100.";
        return false;
    }

    if (totalCopies < 0)
    {
        error = "Total copies cannot be negative.";
        return false;
    }

    if (availableCopies < 0 || availableCopies > totalCopies)
    {
        error = "Available copies must be between 0 and total copies.";
        return false;
    }

    return true;
}

bool restoreBook(const BookInfo& info, string& error)
{
    if (!validateBookFields(info.id, info.title, info.author,
                            info.publicationYear, info.totalCopies,
                            info.availableCopies, error))
    {
        return false;
    }

    if (findBookById(info.id) != nullptr)
    {
        error = "A book with this ID already exists.";
        return false;
    }

    Book* node = new Book();
    node->id = info.id;
    node->title = trim(info.title);
    node->author = trim(info.author);
    node->publisher = trim(info.publisher);
    node->publicationYear = info.publicationYear;
    node->totalCopies = info.totalCopies;
    node->availableCopies = info.availableCopies;
    node->next = bookHead;
    bookHead = node;
    ++bookCount;

    return true;
}

bool addBook(int id, const string& title, const string& author,
             const string& publisher, int publicationYear, int totalCopies,
             string& error)
{
    BookInfo info;
    info.id = id;
    info.title = title;
    info.author = author;
    info.publisher = publisher;
    info.publicationYear = publicationYear;
    info.totalCopies = totalCopies;
    info.availableCopies = totalCopies;

    return restoreBook(info, error);
}

bool updateBook(int id, const string& title, const string& author,
                const string& publisher, int publicationYear, int totalCopies,
                string& error)
{
    Book* book = nullptr;
    Book* current = bookHead;

    while (current != nullptr)
    {
        if (current->id == id)
        {
            book = current;
            break;
        }

        current = current->next;
    }

    if (book == nullptr)
    {
        error = "Book not found.";
        return false;
    }

    int issuedCopies = book->totalCopies - book->availableCopies;

    if (totalCopies < 0)
    {
        error = "Total copies cannot be negative.";
        return false;
    }

    if (totalCopies < issuedCopies)
    {
        error = "Total copies cannot be less than the number of issued copies.";
        return false;
    }

    int newAvailable = totalCopies - issuedCopies;

    if (!validateBookFields(id, title, author, publicationYear,
                            totalCopies, newAvailable, error))
    {
        return false;
    }

    book->title = trim(title);
    book->author = trim(author);
    book->publisher = trim(publisher);
    book->publicationYear = publicationYear;
    book->totalCopies = totalCopies;
    book->availableCopies = newAvailable;

    return true;
}

bool deleteBook(int id, string& error)
{
    Book* previous = nullptr;
    Book* current = bookHead;

    while (current != nullptr && current->id != id)
    {
        previous = current;
        current = current->next;
    }

    if (current == nullptr)
    {
        error = "Book not found.";
        return false;
    }

    if (previous == nullptr)
    {
        bookHead = current->next;
    }
    else
    {
        previous->next = current->next;
    }

    delete current;
    --bookCount;
    return true;
}

const Book* findBookById(int id)
{
    const Book* current = bookHead;

    while (current != nullptr)
    {
        if (current->id == id)
        {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

void displayAllBooks()
{
    if (bookHead == nullptr)
    {
        cout << "No books in the library." << endl;
        return;
    }

    cout << left
         << setw(5) << "ID"
         << setw(30) << "Title"
         << setw(22) << "Author"
         << setw(10) << "Year"
         << setw(7) << "Total"
         << setw(10) << "Available" << endl;

    cout << string(84, '-') << endl;

    const Book* current = bookHead;

    while (current != nullptr)
    {
        cout << left
             << setw(5) << current->id
             << setw(30) << fitText(current->title, 29)
             << setw(22) << fitText(current->author, 21)
             << setw(10) << current->publicationYear
             << setw(7) << current->totalCopies
             << setw(10) << current->availableCopies << endl;

        current = current->next;
    }
}

int getTotalBookTitles()
{
    return bookCount;
}

int getTotalCopies()
{
    int total = 0;
    const Book* current = bookHead;

    while (current != nullptr)
    {
        total += current->totalCopies;
        current = current->next;
    }

    return total;
}

int getAvailableCopies()
{
    int total = 0;
    const Book* current = bookHead;

    while (current != nullptr)
    {
        total += current->availableCopies;
        current = current->next;
    }

    return total;
}

int getIssuedCopies()
{
    return getTotalCopies() - getAvailableCopies();
}

bool changeAvailableCopies(int bookId, int delta, string& error)
{
    Book* current = bookHead;

    while (current != nullptr)
    {
        if (current->id == bookId)
        {
            int updated = current->availableCopies + delta;

            if (updated < 0 || updated > current->totalCopies)
            {
                error = "Available copies would go outside the valid range.";
                return false;
            }

            current->availableCopies = updated;
            return true;
        }

        current = current->next;
    }

    error = "Book not found.";
    return false;
}

int collectBooks(BookInfo* output, int capacity)
{
    int count = 0;
    const Book* current = bookHead;

    while (current != nullptr && count < capacity)
    {
        output[count].id = current->id;
        output[count].title = current->title;
        output[count].author = current->author;
        output[count].publisher = current->publisher;
        output[count].publicationYear = current->publicationYear;
        output[count].totalCopies = current->totalCopies;
        output[count].availableCopies = current->availableCopies;
        ++count;
        current = current->next;
    }

    return count;
}

void clearBooks()
{
    while (bookHead != nullptr)
    {
        Book* next = bookHead->next;
        delete bookHead;
        bookHead = next;
    }

    bookCount = 0;
}
