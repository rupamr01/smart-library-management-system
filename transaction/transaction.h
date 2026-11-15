#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include "../common/models.h"
using namespace std;

struct ReturnOutcome
{
    double fine = 0.0;
    int nextStudentId = 0;
    int newTransactionId = 0;
    int skippedReservations = 0;
};

bool issueBook(int studentId, int bookId, const string& today, string& error);
bool returnBook(int transactionId, const string& returnDate,
                ReturnOutcome& outcome, string& error);
bool reserveBook(int studentId, int bookId, string& error);
bool cancelReservation(int studentId, int bookId, string& error);

void displayActiveLoans();
void displayReservationQueues();
void displayRecentTransactions(int count);
void displayAllTransactions();

int getActiveLoanCount();
int getPendingReservationCount();
int getActiveLoanCountForStudent(int studentId);
bool hasActiveLoanForStudent(int studentId);
bool hasActiveLoanForBook(int bookId);
bool hasAnyTransactionForBook(int bookId);
bool hasAnyTransactionForStudent(int studentId);
bool hasPendingReservationForStudent(int studentId);
bool hasPendingReservationForBook(int bookId);
int findActiveLoanForStudentBook(int studentId, int bookId);

int getTransactionCount();
int collectTransactions(TransactionInfo* output, int capacity);
int getReservationCount();
int collectReservations(ReservationInfo* output, int capacity);
bool restoreTransaction(const TransactionInfo& info, string& error);
bool restoreReservation(int bookId, int studentId, string& error);
void rebuildHistory();
int getHistoryCount();
bool getHistoryEntry(int indexFromTop, HistoryInfo& output);
void clearTransactions();

#endif
