#ifndef COMMON_MODELS_H
#define COMMON_MODELS_H

#include <string>
using namespace std;

// Live book record used by the linked list in the book module.
struct Book
{
    int id = 0;
    string title;
    string author;
    string publisher;
    int publicationYear = 0;
    int totalCopies = 0;
    int availableCopies = 0;
    Book* next = nullptr;
};

// Live student record used by the binary search tree in the student module.
struct Student
{
    int id = 0;
    string name;
    string course;
    string phone;
    Student* left = nullptr;
    Student* right = nullptr;
};

// Live loan record used by the transaction module.
struct Transaction
{
    int transactionId = 0;
    int studentId = 0;
    int bookId = 0;
    string issueDate;
    string dueDate;
    string returnDate;
    double fine = 0.0;
    bool active = false;
    Transaction* next = nullptr;
};

// Read-only copies shared with search, sort, reports and storage.
struct BookInfo
{
    int id = 0;
    string title;
    string author;
    string publisher;
    int publicationYear = 0;
    int totalCopies = 0;
    int availableCopies = 0;
};

struct StudentInfo
{
    int id = 0;
    string name;
    string course;
    string phone;
};

struct TransactionInfo
{
    int transactionId = 0;
    int studentId = 0;
    int bookId = 0;
    string issueDate;
    string dueDate;
    string returnDate;
    double fine = 0.0;
    bool active = false;
};

struct ReservationInfo
{
    int bookId = 0;
    int studentId = 0;
};

struct HistoryInfo
{
    string action;
    int transactionId = 0;
    int studentId = 0;
    int bookId = 0;
    string date;
    double fine = 0.0;
};

#endif
