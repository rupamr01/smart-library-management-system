#include "transaction.h"
#include "../book/book.h"
#include "../student/student.h"
#include "../common/config.h"
#include "../common/date_utils.h"
#include "../common/validation.h"
#include <iostream>
#include <iomanip>
using namespace std;

struct ReservationNode
{
    int studentId = 0;
    ReservationNode* next = nullptr;
};

struct BookQueue
{
    int bookId = 0;
    ReservationNode* front = nullptr;
    ReservationNode* rear = nullptr;
    int count = 0;
    BookQueue* next = nullptr;
};

struct HistoryNode
{
    HistoryInfo info;
    HistoryNode* next = nullptr;
};

static Transaction* transactionHead = nullptr;
static Transaction* transactionTail = nullptr;
static int transactionCount = 0;
static int nextTransactionId = 1;

static BookQueue* queueHead = nullptr;
static int reservationCount = 0;

static HistoryNode* historyTop = nullptr;
static int historyCount = 0;

static Transaction* findTransactionNode(int transactionId)
{
    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (current->transactionId == transactionId)
        {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

static BookQueue* findBookQueue(int bookId)
{
    BookQueue* current = queueHead;

    while (current != nullptr)
    {
        if (current->bookId == bookId)
        {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

static BookQueue* getOrCreateBookQueue(int bookId)
{
    BookQueue* existing = findBookQueue(bookId);

    if (existing != nullptr)
    {
        return existing;
    }

    BookQueue* queue = new BookQueue();
    queue->bookId = bookId;
    queue->next = queueHead;
    queueHead = queue;
    return queue;
}

static void enqueueReservation(int bookId, int studentId)
{
    BookQueue* queue = getOrCreateBookQueue(bookId);
    ReservationNode* node = new ReservationNode();
    node->studentId = studentId;

    if (queue->rear == nullptr)
    {
        queue->front = node;
    }
    else
    {
        queue->rear->next = node;
    }

    queue->rear = node;
    ++queue->count;
    ++reservationCount;
}

static int dequeueReservation(int bookId)
{
    BookQueue* queue = findBookQueue(bookId);

    if (queue == nullptr || queue->front == nullptr)
    {
        return -1;
    }

    ReservationNode* node = queue->front;
    int studentId = node->studentId;
    queue->front = node->next;

    if (queue->front == nullptr)
    {
        queue->rear = nullptr;
    }

    delete node;
    --queue->count;
    --reservationCount;

    if (queue->count == 0)
    {
        BookQueue* previous = nullptr;
        BookQueue* current = queueHead;

        while (current != nullptr && current != queue)
        {
            previous = current;
            current = current->next;
        }

        if (previous == nullptr)
        {
            queueHead = queue->next;
        }
        else
        {
            previous->next = queue->next;
        }

        delete queue;
    }

    return studentId;
}

static bool isReservationDuplicate(int bookId, int studentId)
{
    BookQueue* queue = findBookQueue(bookId);

    if (queue == nullptr)
    {
        return false;
    }

    ReservationNode* current = queue->front;

    while (current != nullptr)
    {
        if (current->studentId == studentId)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}

static bool removeReservation(int bookId, int studentId)
{
    BookQueue* queue = findBookQueue(bookId);

    if (queue == nullptr)
    {
        return false;
    }

    ReservationNode* previous = nullptr;
    ReservationNode* current = queue->front;

    while (current != nullptr && current->studentId != studentId)
    {
        previous = current;
        current = current->next;
    }

    if (current == nullptr)
    {
        return false;
    }

    if (previous == nullptr)
    {
        queue->front = current->next;
    }
    else
    {
        previous->next = current->next;
    }

    if (queue->rear == current)
    {
        queue->rear = previous;
    }

    delete current;
    --queue->count;
    --reservationCount;

    if (queue->count == 0)
    {
        BookQueue* queuePrevious = nullptr;
        BookQueue* queueCurrent = queueHead;

        while (queueCurrent != nullptr && queueCurrent != queue)
        {
            queuePrevious = queueCurrent;
            queueCurrent = queueCurrent->next;
        }

        if (queuePrevious == nullptr)
        {
            queueHead = queue->next;
        }
        else
        {
            queuePrevious->next = queue->next;
        }

        delete queue;
    }

    return true;
}

static void pushHistory(const HistoryInfo& info)
{
    HistoryNode* node = new HistoryNode();
    node->info = info;
    node->next = historyTop;
    historyTop = node;
    ++historyCount;
}

static void clearHistory()
{
    while (historyTop != nullptr)
    {
        HistoryNode* next = historyTop->next;
        delete historyTop;
        historyTop = next;
    }

    historyCount = 0;
}

static bool historyComesBefore(const HistoryInfo& first, const HistoryInfo& second)
{
    if (first.date != second.date)
    {
        return first.date < second.date;
    }

    if (first.transactionId != second.transactionId)
    {
        return first.transactionId < second.transactionId;
    }

    return first.action < second.action;
}

static void mergeSortHistory(HistoryInfo* items, int left, int right, HistoryInfo* temp)
{
    if (left >= right)
    {
        return;
    }

    int middle = left + (right - left) / 2;
    mergeSortHistory(items, left, middle, temp);
    mergeSortHistory(items, middle + 1, right, temp);

    int i = left;
    int j = middle + 1;
    int k = left;

    while (i <= middle && j <= right)
    {
        if (historyComesBefore(items[i], items[j]))
        {
            temp[k++] = items[i++];
        }
        else
        {
            temp[k++] = items[j++];
        }
    }

    while (i <= middle)
    {
        temp[k++] = items[i++];
    }

    while (j <= right)
    {
        temp[k++] = items[j++];
    }

    for (int index = left; index <= right; ++index)
    {
        items[index] = temp[index];
    }
}

static bool issueBookInternal(int studentId, int bookId, const string& issueDate,
                              bool automatic, int& newTransactionId, string& error)
{
    int year = 0;
    int month = 0;
    int day = 0;

    if (!parseDate(issueDate, year, month, day))
    {
        error = "Invalid issue date.";
        return false;
    }

    if (studentId <= 0 || bookId <= 0)
    {
        error = "Student ID and book ID must be positive.";
        return false;
    }

    const Student* student = findStudentById(studentId);

    if (student == nullptr)
    {
        error = "Student not found.";
        return false;
    }

    const Book* book = findBookById(bookId);

    if (book == nullptr)
    {
        error = "Book not found.";
        return false;
    }

    if (getActiveLoanCountForStudent(studentId) >= Config::MAX_ACTIVE_LOANS)
    {
        error = "Borrowing limit reached.";
        return false;
    }

    if (findActiveLoanForStudentBook(studentId, bookId) != -1)
    {
        error = "This student already has an active loan for this book.";
        return false;
    }

    if (book->availableCopies <= 0)
    {
        error = "No copies are available.";
        return false;
    }

    if (!changeAvailableCopies(bookId, -1, error))
    {
        return false;
    }

    Transaction* transaction = new Transaction();
    transaction->transactionId = nextTransactionId++;
    transaction->studentId = studentId;
    transaction->bookId = bookId;
    transaction->issueDate = issueDate;
    transaction->dueDate = addDays(issueDate, Config::LOAN_PERIOD_DAYS);
    transaction->returnDate = "";
    transaction->fine = 0.0;
    transaction->active = true;

    if (transaction->dueDate.empty())
    {
        string ignored;
        changeAvailableCopies(bookId, 1, ignored);
        delete transaction;
        error = "Could not calculate the due date.";
        return false;
    }

    if (transactionTail == nullptr)
    {
        transactionHead = transaction;
    }
    else
    {
        transactionTail->next = transaction;
    }

    transactionTail = transaction;
    ++transactionCount;

    HistoryInfo info;
    info.action = automatic ? "AUTO-ISSUE" : "ISSUE";
    info.transactionId = transaction->transactionId;
    info.studentId = studentId;
    info.bookId = bookId;
    info.date = issueDate;
    info.fine = 0.0;
    pushHistory(info);

    newTransactionId = transaction->transactionId;
    return true;
}

bool issueBook(int studentId, int bookId, const string& today, string& error)
{
    int transactionId = 0;
    return issueBookInternal(studentId, bookId, today, false, transactionId, error);
}

bool returnBook(int transactionId, const string& returnDate,
                ReturnOutcome& outcome, string& error)
{
    outcome = ReturnOutcome();

    Transaction* transaction = findTransactionNode(transactionId);

    if (transaction == nullptr)
    {
        error = "Transaction not found.";
        return false;
    }

    if (!transaction->active)
    {
        error = "This loan has already been returned.";
        return false;
    }

    int year = 0;
    int month = 0;
    int day = 0;

    if (!parseDate(returnDate, year, month, day))
    {
        error = "Invalid return date.";
        return false;
    }

    if (daysBetween(transaction->issueDate, returnDate) < 0)
    {
        error = "Return date cannot be before the issue date.";
        return false;
    }

    const Book* book = findBookById(transaction->bookId);

    if (book == nullptr)
    {
        error = "The book for this loan no longer exists.";
        return false;
    }

    if (book->availableCopies >= book->totalCopies)
    {
        error = "Availability is already at total copies; data may be inconsistent.";
        return false;
    }

    int overdueDays = daysBetween(transaction->dueDate, returnDate);

    if (overdueDays < 0)
    {
        overdueDays = 0;
    }

    transaction->fine = overdueDays * Config::FINE_PER_DAY;
    transaction->returnDate = returnDate;
    transaction->active = false;
    outcome.fine = transaction->fine;

    HistoryInfo returned;
    returned.action = "RETURN";
    returned.transactionId = transaction->transactionId;
    returned.studentId = transaction->studentId;
    returned.bookId = transaction->bookId;
    returned.date = returnDate;
    returned.fine = transaction->fine;
    pushHistory(returned);

    if (!changeAvailableCopies(transaction->bookId, 1, error))
    {
        return false;
    }

    while (true)
    {
        BookQueue* queue = findBookQueue(transaction->bookId);

        if (queue == nullptr || queue->front == nullptr)
        {
            break;
        }

        int studentId = dequeueReservation(transaction->bookId);

        if (studentId <= 0)
        {
            break;
        }

        if (findStudentById(studentId) == nullptr)
        {
            ++outcome.skippedReservations;
            continue;
        }

        if (getActiveLoanCountForStudent(studentId) >= Config::MAX_ACTIVE_LOANS)
        {
            ++outcome.skippedReservations;
            continue;
        }

        if (findActiveLoanForStudentBook(studentId, transaction->bookId) != -1)
        {
            ++outcome.skippedReservations;
            continue;
        }

        int newId = 0;
        string issueError;

        if (issueBookInternal(studentId, transaction->bookId, returnDate, true, newId, issueError))
        {
            outcome.nextStudentId = studentId;
            outcome.newTransactionId = newId;
            break;
        }

        ++outcome.skippedReservations;
    }

    return true;
}

bool reserveBook(int studentId, int bookId, string& error)
{
    if (studentId <= 0 || bookId <= 0)
    {
        error = "Student ID and book ID must be positive.";
        return false;
    }

    if (findStudentById(studentId) == nullptr)
    {
        error = "Student not found.";
        return false;
    }

    const Book* book = findBookById(bookId);

    if (book == nullptr)
    {
        error = "Book not found.";
        return false;
    }

    if (book->availableCopies > 0)
    {
        error = "Copies are available; issue the book instead.";
        return false;
    }

    if (isReservationDuplicate(bookId, studentId))
    {
        error = "This student already has a reservation for this book.";
        return false;
    }

    if (findActiveLoanForStudentBook(studentId, bookId) != -1)
    {
        error = "This student already has an active loan for this book.";
        return false;
    }

    enqueueReservation(bookId, studentId);

    HistoryInfo info;
    info.action = "RESERVE";
    info.transactionId = 0;
    info.studentId = studentId;
    info.bookId = bookId;
    info.date = todayString();
    info.fine = 0.0;
    pushHistory(info);

    return true;
}

bool cancelReservation(int studentId, int bookId, string& error)
{
    if (!removeReservation(bookId, studentId))
    {
        error = "Reservation not found.";
        return false;
    }

    HistoryInfo info;
    info.action = "CANCEL";
    info.transactionId = 0;
    info.studentId = studentId;
    info.bookId = bookId;
    info.date = todayString();
    info.fine = 0.0;
    pushHistory(info);

    return true;
}

void displayActiveLoans()
{
    bool found = false;

    cout << left
         << setw(6) << "Txn"
         << setw(8) << "Stud ID"
         << setw(18) << "Student"
         << setw(8) << "Book ID"
         << setw(24) << "Book"
         << setw(12) << "Issued"
         << setw(12) << "Due" << endl;

    cout << string(88, '-') << endl;

    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (current->active)
        {
            found = true;
            const Student* student = findStudentById(current->studentId);
            const Book* book = findBookById(current->bookId);
            string studentName = student != nullptr ? student->name : "(missing)";
            string bookTitle = book != nullptr ? book->title : "(missing)";

            cout << left
                 << setw(6) << current->transactionId
                 << setw(8) << current->studentId
                 << setw(18) << fitText(studentName, 17)
                 << setw(8) << current->bookId
                 << setw(24) << fitText(bookTitle, 23)
                 << setw(12) << current->issueDate
                 << setw(12) << current->dueDate << endl;
        }

        current = current->next;
    }

    if (!found)
    {
        cout << "No active loans." << endl;
    }
}

void displayReservationQueues()
{
    if (queueHead == nullptr)
    {
        cout << "No pending reservations." << endl;
        return;
    }

    BookQueue* queue = queueHead;

    while (queue != nullptr)
    {
        const Book* book = findBookById(queue->bookId);
        string title = book != nullptr ? book->title : "(missing book)";

        cout << "\nBook " << queue->bookId << " - " << title << endl;
        cout << "Waiting list (" << queue->count << "):" << endl;

        ReservationNode* node = queue->front;
        int position = 1;

        while (node != nullptr)
        {
            const Student* student = findStudentById(node->studentId);
            string name = student != nullptr ? student->name : "(missing student)";
            cout << "  " << position << ". " << node->studentId << " - " << name << endl;
            ++position;
            node = node->next;
        }

        queue = queue->next;
    }
}

void displayRecentTransactions(int count)
{
    if (historyTop == nullptr)
    {
        cout << "No recent activity." << endl;
        return;
    }

    if (count <= 0)
    {
        count = Config::RECENT_HISTORY_COUNT;
    }

    cout << left
         << setw(14) << "Action"
         << setw(12) << "Date"
         << setw(8) << "Txn"
         << setw(8) << "Stud ID"
         << setw(8) << "Book ID"
         << setw(10) << "Fine" << endl;

    cout << string(60, '-') << endl;

    HistoryNode* current = historyTop;
    int shown = 0;

    while (current != nullptr && shown < count)
    {
        cout << left
             << setw(14) << current->info.action
             << setw(12) << current->info.date
             << setw(8) << current->info.transactionId
             << setw(8) << current->info.studentId
             << setw(8) << current->info.bookId
             << setw(10) << fixed << setprecision(2) << current->info.fine << endl;

        current = current->next;
        ++shown;
    }
}

void displayAllTransactions()
{
    if (transactionHead == nullptr)
    {
        cout << "No transactions recorded." << endl;
        return;
    }

    cout << left
         << setw(6) << "Txn"
         << setw(8) << "Stud ID"
         << setw(8) << "Book ID"
         << setw(12) << "Issued"
         << setw(12) << "Due"
         << setw(12) << "Returned"
         << setw(10) << "Fine"
         << setw(10) << "Status" << endl;

    cout << string(88, '-') << endl;

    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        cout << left
             << setw(6) << current->transactionId
             << setw(8) << current->studentId
             << setw(8) << current->bookId
             << setw(12) << current->issueDate
             << setw(12) << current->dueDate
             << setw(12) << (current->returnDate.empty() ? "-" : current->returnDate)
             << setw(10) << fixed << setprecision(2) << current->fine
             << setw(10) << (current->active ? "ACTIVE" : "RETURNED") << endl;

        current = current->next;
    }
}

int getActiveLoanCount()
{
    int count = 0;
    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (current->active)
        {
            ++count;
        }

        current = current->next;
    }

    return count;
}

int getPendingReservationCount()
{
    return reservationCount;
}

int getActiveLoanCountForStudent(int studentId)
{
    int count = 0;
    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (current->active && current->studentId == studentId)
        {
            ++count;
        }

        current = current->next;
    }

    return count;
}

bool hasActiveLoanForStudent(int studentId)
{
    return getActiveLoanCountForStudent(studentId) > 0;
}

bool hasActiveLoanForBook(int bookId)
{
    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (current->active && current->bookId == bookId)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}

bool hasAnyTransactionForBook(int bookId)
{
    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (current->bookId == bookId)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}

bool hasAnyTransactionForStudent(int studentId)
{
    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (current->studentId == studentId)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}

bool hasPendingReservationForStudent(int studentId)
{
    BookQueue* queue = queueHead;

    while (queue != nullptr)
    {
        ReservationNode* node = queue->front;

        while (node != nullptr)
        {
            if (node->studentId == studentId)
            {
                return true;
            }

            node = node->next;
        }

        queue = queue->next;
    }

    return false;
}

bool hasPendingReservationForBook(int bookId)
{
    BookQueue* queue = findBookQueue(bookId);
    return queue != nullptr && queue->count > 0;
}

int findActiveLoanForStudentBook(int studentId, int bookId)
{
    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (current->active && current->studentId == studentId && current->bookId == bookId)
        {
            return current->transactionId;
        }

        current = current->next;
    }

    return -1;
}

int getTransactionCount()
{
    return transactionCount;
}

int collectTransactions(TransactionInfo* output, int capacity)
{
    int count = 0;
    Transaction* current = transactionHead;

    while (current != nullptr && count < capacity)
    {
        output[count].transactionId = current->transactionId;
        output[count].studentId = current->studentId;
        output[count].bookId = current->bookId;
        output[count].issueDate = current->issueDate;
        output[count].dueDate = current->dueDate;
        output[count].returnDate = current->returnDate;
        output[count].fine = current->fine;
        output[count].active = current->active;
        ++count;
        current = current->next;
    }

    return count;
}

int getReservationCount()
{
    return reservationCount;
}

int collectReservations(ReservationInfo* output, int capacity)
{
    int count = 0;
    BookQueue* queue = queueHead;

    while (queue != nullptr)
    {
        ReservationNode* node = queue->front;

        while (node != nullptr && count < capacity)
        {
            output[count].bookId = queue->bookId;
            output[count].studentId = node->studentId;
            ++count;
            node = node->next;
        }

        queue = queue->next;
    }

    return count;
}

bool restoreTransaction(const TransactionInfo& info, string& error)
{
    if (info.transactionId <= 0 || info.studentId <= 0 || info.bookId <= 0)
    {
        error = "Transaction IDs must be positive.";
        return false;
    }

    if (findTransactionNode(info.transactionId) != nullptr)
    {
        error = "Duplicate transaction ID.";
        return false;
    }

    int year = 0;
    int month = 0;
    int day = 0;

    if (!parseDate(info.issueDate, year, month, day))
    {
        error = "Invalid issue date.";
        return false;
    }

    if (!parseDate(info.dueDate, year, month, day))
    {
        error = "Invalid due date.";
        return false;
    }

    if (daysBetween(info.issueDate, info.dueDate) < 0)
    {
        error = "Due date cannot be before the issue date.";
        return false;
    }

    if (info.active)
    {
        if (!info.returnDate.empty())
        {
            error = "Active transaction cannot have a return date.";
            return false;
        }
    }
    else
    {
        if (info.returnDate.empty() || !parseDate(info.returnDate, year, month, day))
        {
            error = "Returned transaction must have a valid return date.";
            return false;
        }

        if (daysBetween(info.issueDate, info.returnDate) < 0)
        {
            error = "Return date cannot be before the issue date.";
            return false;
        }
    }

    if (info.fine < 0.0)
    {
        error = "Fine cannot be negative.";
        return false;
    }

    if (info.active && findStudentById(info.studentId) == nullptr)
    {
        error = "Active transaction references a missing student.";
        return false;
    }

    if (info.active && findBookById(info.bookId) == nullptr)
    {
        error = "Active transaction references a missing book.";
        return false;
    }

    Transaction* transaction = new Transaction();
    transaction->transactionId = info.transactionId;
    transaction->studentId = info.studentId;
    transaction->bookId = info.bookId;
    transaction->issueDate = info.issueDate;
    transaction->dueDate = info.dueDate;
    transaction->returnDate = info.returnDate;
    transaction->fine = info.fine;
    transaction->active = info.active;

    if (transactionTail == nullptr)
    {
        transactionHead = transaction;
    }
    else
    {
        transactionTail->next = transaction;
    }

    transactionTail = transaction;
    ++transactionCount;

    if (info.transactionId >= nextTransactionId)
    {
        nextTransactionId = info.transactionId + 1;
    }

    return true;
}

bool restoreReservation(int bookId, int studentId, string& error)
{
    if (bookId <= 0 || studentId <= 0)
    {
        error = "Reservation IDs must be positive.";
        return false;
    }

    if (findBookById(bookId) == nullptr)
    {
        error = "Reservation references a missing book.";
        return false;
    }

    if (findStudentById(studentId) == nullptr)
    {
        error = "Reservation references a missing student.";
        return false;
    }

    if (isReservationDuplicate(bookId, studentId))
    {
        error = "Duplicate reservation.";
        return false;
    }

    enqueueReservation(bookId, studentId);
    return true;
}

void rebuildHistory()
{
    clearHistory();

    if (transactionCount == 0)
    {
        return;
    }

    int returnedCount = 0;
    Transaction* current = transactionHead;

    while (current != nullptr)
    {
        if (!current->active)
        {
            ++returnedCount;
        }

        current = current->next;
    }

    int eventCount = transactionCount + returnedCount;
    HistoryInfo* events = new HistoryInfo[eventCount > 0 ? eventCount : 1];
    int index = 0;

    current = transactionHead;

    while (current != nullptr)
    {
        HistoryInfo issued;
        issued.action = "ISSUE";
        issued.transactionId = current->transactionId;
        issued.studentId = current->studentId;
        issued.bookId = current->bookId;
        issued.date = current->issueDate;
        issued.fine = 0.0;
        events[index++] = issued;

        if (!current->active)
        {
            HistoryInfo returned;
            returned.action = "RETURN";
            returned.transactionId = current->transactionId;
            returned.studentId = current->studentId;
            returned.bookId = current->bookId;
            returned.date = current->returnDate;
            returned.fine = current->fine;
            events[index++] = returned;
        }

        current = current->next;
    }

    HistoryInfo* temp = new HistoryInfo[index > 0 ? index : 1];
    mergeSortHistory(events, 0, index - 1, temp);

    for (int i = 0; i < index; ++i)
    {
        pushHistory(events[i]);
    }

    delete[] temp;
    delete[] events;
}

int getHistoryCount()
{
    return historyCount;
}

bool getHistoryEntry(int indexFromTop, HistoryInfo& output)
{
    HistoryNode* current = historyTop;
    int index = 0;

    while (current != nullptr)
    {
        if (index == indexFromTop)
        {
            output = current->info;
            return true;
        }

        ++index;
        current = current->next;
    }

    return false;
}

void clearTransactions()
{
    while (transactionHead != nullptr)
    {
        Transaction* next = transactionHead->next;
        delete transactionHead;
        transactionHead = next;
    }

    transactionTail = nullptr;
    transactionCount = 0;
    nextTransactionId = 1;

    while (queueHead != nullptr)
    {
        BookQueue* nextQueue = queueHead->next;
        ReservationNode* node = queueHead->front;

        while (node != nullptr)
        {
            ReservationNode* nextNode = node->next;
            delete node;
            node = nextNode;
        }

        delete queueHead;
        queueHead = nextQueue;
    }

    reservationCount = 0;
    clearHistory();
}
