#ifndef SEARCH_H
#define SEARCH_H

#include <string>
#include "../common/models.h"
using namespace std;

enum class BookSortKey
{
    Id,
    Title,
    Author,
    Year
};

void searchBooksMenu();
void sortBooksMenu();
void displayLibraryReports();

void mergeSortBooks(BookInfo* books, int count, BookSortKey key, bool ascending);
int binarySearchBookById(const BookInfo* sortedBooks, int count, int id);
int collectMatchingBooks(const BookInfo* books, int count, const string& query,
                         bool matchTitle, int* indices, int maxIndices);

#endif
