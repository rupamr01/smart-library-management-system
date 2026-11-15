#ifndef BOOK_H
#define BOOK_H

#include <string>
#include "../common/models.h"
using namespace std;

bool addBook(int id, const string& title, const string& author,
             const string& publisher, int publicationYear, int totalCopies,
             string& error);
bool updateBook(int id, const string& title, const string& author,
                const string& publisher, int publicationYear, int totalCopies,
                string& error);
bool deleteBook(int id, string& error);
const Book* findBookById(int id);
void displayAllBooks();
int getTotalBookTitles();
int getTotalCopies();
int getAvailableCopies();
int getIssuedCopies();
bool changeAvailableCopies(int bookId, int delta, string& error);
int collectBooks(BookInfo* output, int capacity);
bool restoreBook(const BookInfo& info, string& error);
void clearBooks();

#endif
