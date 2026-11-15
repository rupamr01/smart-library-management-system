#include "storage.h"
#include "../book/book.h"
#include "../student/student.h"
#include "../transaction/transaction.h"
#include "../common/validation.h"
#include <fstream>
#include <cstdio>
#include <cerrno>
#include <iomanip>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
using namespace std;

static void appendWarning(LoadReport& report, const string& message)
{
    report.hadWarnings = true;

    if (!report.warnings.empty())
    {
        report.warnings += "\n";
    }

    report.warnings += message;
}

static string joinPath(const string& directory, const string& fileName)
{
    if (directory.empty())
    {
        return fileName;
    }

    char last = directory[directory.size() - 1];

    if (last == '/' || last == '\\')
    {
        return directory + fileName;
    }

    return directory + "/" + fileName;
}

static bool ensureDirectory(const string& path)
{
    if (path.empty())
    {
        return false;
    }

#ifdef _WIN32
    if (_mkdir(path.c_str()) == 0)
    {
        return true;
    }
#else
    if (mkdir(path.c_str(), 0755) == 0)
    {
        return true;
    }
#endif

    return errno == EEXIST;
}

static bool fileExists(const string& path)
{
    ifstream input(path.c_str());
    return input.good();
}

static bool openForRead(const string& path, ifstream& input, bool& missing)
{
    missing = false;
    input.open(path.c_str());

    if (input.is_open())
    {
        return true;
    }

    FILE* file = fopen(path.c_str(), "r");

    if (file == nullptr)
    {
        if (errno == ENOENT)
        {
            missing = true;
            return true;
        }

        return false;
    }

    fclose(file);
    return false;
}

static string escapeField(const string& value)
{
    string result;

    for (size_t i = 0; i < value.size(); ++i)
    {
        char character = value[i];

        if (character == '\\' || character == '|')
        {
            result += '\\';
        }

        result += character;
    }

    return result;
}

static bool splitEscaped(const string& line, string fields[], int maxFields, int& fieldCount)
{
    fieldCount = 0;
    string current;
    bool escaped = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char character = line[i];

        if (escaped)
        {
            current += character;
            escaped = false;
        }
        else if (character == '\\')
        {
            escaped = true;
        }
        else if (character == '|')
        {
            if (fieldCount >= maxFields)
            {
                return false;
            }

            fields[fieldCount++] = current;
            current.clear();
        }
        else
        {
            current += character;
        }
    }

    if (escaped || fieldCount >= maxFields)
    {
        return false;
    }

    fields[fieldCount++] = current;
    return true;
}

static bool loadBooks(const string& path, LoadReport& report)
{
    ifstream input;
    bool missing = false;

    if (!openForRead(path, input, missing))
    {
        report.fatal = true;
        appendWarning(report, "Cannot open " + path);
        return false;
    }

    if (missing)
    {
        return true;
    }

    string line;
    int lineNumber = 0;

    while (getline(input, line))
    {
        ++lineNumber;
        line = trim(line);

        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        string fields[7];
        int fieldCount = 0;

        if (!splitEscaped(line, fields, 7, fieldCount) || fieldCount != 7)
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": invalid field count");
            continue;
        }

        int id = 0;
        int year = 0;
        int total = 0;
        int available = 0;

        if (!parseInt(fields[0], id) || !parseInt(fields[4], year) ||
            !parseInt(fields[5], total) || !parseInt(fields[6], available))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": invalid number");
            continue;
        }

        BookInfo info;
        info.id = id;
        info.title = fields[1];
        info.author = fields[2];
        info.publisher = fields[3];
        info.publicationYear = year;
        info.totalCopies = total;
        info.availableCopies = available;

        string error;

        if (!restoreBook(info, error))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": " + error);
            continue;
        }

        ++report.books;
    }

    return true;
}

static bool loadStudents(const string& path, LoadReport& report)
{
    ifstream input;
    bool missing = false;

    if (!openForRead(path, input, missing))
    {
        report.fatal = true;
        appendWarning(report, "Cannot open " + path);
        return false;
    }

    if (missing)
    {
        return true;
    }

    string line;
    int lineNumber = 0;

    while (getline(input, line))
    {
        ++lineNumber;
        line = trim(line);

        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        string fields[4];
        int fieldCount = 0;

        if (!splitEscaped(line, fields, 4, fieldCount) || fieldCount != 4)
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": invalid field count");
            continue;
        }

        int id = 0;

        if (!parseInt(fields[0], id))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": invalid student ID");
            continue;
        }

        string error;

        if (!addStudent(id, fields[1], fields[2], fields[3], error))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": " + error);
            continue;
        }

        ++report.students;
    }

    return true;
}

static bool loadTransactions(const string& path, LoadReport& report)
{
    ifstream input;
    bool missing = false;

    if (!openForRead(path, input, missing))
    {
        report.fatal = true;
        appendWarning(report, "Cannot open " + path);
        return false;
    }

    if (missing)
    {
        return true;
    }

    string line;
    int lineNumber = 0;

    while (getline(input, line))
    {
        ++lineNumber;
        line = trim(line);

        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        string fields[8];
        int fieldCount = 0;

        if (!splitEscaped(line, fields, 8, fieldCount) || fieldCount != 8)
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": invalid field count");
            continue;
        }

        int transactionId = 0;
        int studentId = 0;
        int bookId = 0;
        double fine = 0.0;
        int activeValue = 0;

        if (!parseInt(fields[0], transactionId) || !parseInt(fields[1], studentId) ||
            !parseInt(fields[2], bookId) || !parseDouble(fields[6], fine) ||
            !parseInt(fields[7], activeValue) || (activeValue != 0 && activeValue != 1))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": invalid transaction field");
            continue;
        }

        TransactionInfo info;
        info.transactionId = transactionId;
        info.studentId = studentId;
        info.bookId = bookId;
        info.issueDate = fields[3];
        info.dueDate = fields[4];
        info.returnDate = fields[5];
        info.fine = fine;
        info.active = (activeValue == 1);

        if (!info.active &&
            (findStudentById(info.studentId) == nullptr || findBookById(info.bookId) == nullptr))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) +
                          ": historical transaction references a deleted record");
        }

        string error;

        if (!restoreTransaction(info, error))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": " + error);
            continue;
        }

        ++report.transactions;
    }

    return true;
}

static bool loadReservations(const string& path, LoadReport& report)
{
    ifstream input;
    bool missing = false;

    if (!openForRead(path, input, missing))
    {
        report.fatal = true;
        appendWarning(report, "Cannot open " + path);
        return false;
    }

    if (missing)
    {
        return true;
    }

    string line;
    int lineNumber = 0;

    while (getline(input, line))
    {
        ++lineNumber;
        line = trim(line);

        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        string fields[2];
        int fieldCount = 0;

        if (!splitEscaped(line, fields, 2, fieldCount) || fieldCount != 2)
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": invalid field count");
            continue;
        }

        int bookId = 0;
        int studentId = 0;

        if (!parseInt(fields[0], bookId) || !parseInt(fields[1], studentId))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": invalid reservation ID");
            continue;
        }

        string error;

        if (!restoreReservation(bookId, studentId, error))
        {
            appendWarning(report, path + ":" + to_string(lineNumber) + ": " + error);
            continue;
        }

        ++report.reservations;
    }

    return true;
}

static void reconcileAvailability(LoadReport& report)
{
    int bookCount = getTotalBookTitles();

    if (bookCount == 0)
    {
        return;
    }

    BookInfo* books = new BookInfo[bookCount];
    collectBooks(books, bookCount);

    int transactionCount = getTransactionCount();
    TransactionInfo* transactions = new TransactionInfo[transactionCount > 0 ? transactionCount : 1];
    collectTransactions(transactions, transactionCount);

    for (int i = 0; i < bookCount; ++i)
    {
        int activeLoans = 0;

        for (int j = 0; j < transactionCount; ++j)
        {
            if (transactions[j].active && transactions[j].bookId == books[i].id)
            {
                ++activeLoans;
            }
        }

        int desiredAvailable = books[i].totalCopies - activeLoans;

        if (desiredAvailable < 0)
        {
            desiredAvailable = 0;
        }

        if (desiredAvailable > books[i].totalCopies)
        {
            desiredAvailable = books[i].totalCopies;
        }

        if (desiredAvailable != books[i].availableCopies)
        {
            string error;

            if (changeAvailableCopies(books[i].id, desiredAvailable - books[i].availableCopies, error))
            {
                appendWarning(report, "Book " + to_string(books[i].id) +
                              ": available copies adjusted from " +
                              to_string(books[i].availableCopies) + " to " +
                              to_string(desiredAvailable) + ".");
            }
            else
            {
                appendWarning(report, "Book " + to_string(books[i].id) +
                              ": availability could not be reconciled: " + error);
            }
        }

        if (hasPendingReservationForBook(books[i].id) && desiredAvailable > 0)
        {
            appendWarning(report, "Book " + to_string(books[i].id) +
                          ": pending reservations exist while copies are available.");
        }
    }

    delete[] transactions;
    delete[] books;
}

bool loadAllData(const string& dataDir, LoadReport& report)
{
    report = LoadReport();
    clearBooks();
    clearStudents();
    clearTransactions();

    string booksPath = joinPath(dataDir, "books.txt");
    string studentsPath = joinPath(dataDir, "students.txt");
    string transactionsPath = joinPath(dataDir, "transactions.txt");
    string reservationsPath = joinPath(dataDir, "reservations.txt");

    if (!loadBooks(booksPath, report))
    {
        return false;
    }

    if (!loadStudents(studentsPath, report))
    {
        return false;
    }

    if (!loadTransactions(transactionsPath, report))
    {
        return false;
    }

    if (!loadReservations(reservationsPath, report))
    {
        return false;
    }

    rebuildHistory();
    reconcileAvailability(report);

    report.books = getTotalBookTitles();
    report.students = getStudentCount();
    report.transactions = getTransactionCount();
    report.reservations = getReservationCount();

    return true;
}

static bool writeBooksFile(const string& path, const BookInfo* books, int count, string& error)
{
    ofstream output(path.c_str());

    if (!output.is_open())
    {
        error = "Cannot create " + path;
        return false;
    }

    output << "# books v1\n";

    for (int i = 0; i < count; ++i)
    {
        output << books[i].id << '|'
               << escapeField(books[i].title) << '|'
               << escapeField(books[i].author) << '|'
               << escapeField(books[i].publisher) << '|'
               << books[i].publicationYear << '|'
               << books[i].totalCopies << '|'
               << books[i].availableCopies << '\n';
    }

    output.close();

    if (output.fail())
    {
        error = "Failed writing " + path;
        return false;
    }

    return true;
}

static bool writeStudentsFile(const string& path, const StudentInfo* students, int count, string& error)
{
    ofstream output(path.c_str());

    if (!output.is_open())
    {
        error = "Cannot create " + path;
        return false;
    }

    output << "# students v1\n";

    for (int i = 0; i < count; ++i)
    {
        output << students[i].id << '|'
               << escapeField(students[i].name) << '|'
               << escapeField(students[i].course) << '|'
               << escapeField(students[i].phone) << '\n';
    }

    output.close();

    if (output.fail())
    {
        error = "Failed writing " + path;
        return false;
    }

    return true;
}

static bool writeTransactionsFile(const string& path, const TransactionInfo* transactions,
                                  int count, string& error)
{
    ofstream output(path.c_str());

    if (!output.is_open())
    {
        error = "Cannot create " + path;
        return false;
    }

    output << "# transactions v1\n";

    for (int i = 0; i < count; ++i)
    {
        output << transactions[i].transactionId << '|'
               << transactions[i].studentId << '|'
               << transactions[i].bookId << '|'
               << transactions[i].issueDate << '|'
               << transactions[i].dueDate << '|'
               << transactions[i].returnDate << '|'
               << fixed << setprecision(2) << transactions[i].fine << '|'
               << (transactions[i].active ? 1 : 0) << '\n';
    }

    output.close();

    if (output.fail())
    {
        error = "Failed writing " + path;
        return false;
    }

    return true;
}

static bool writeReservationsFile(const string& path, const ReservationInfo* reservations,
                                  int count, string& error)
{
    ofstream output(path.c_str());

    if (!output.is_open())
    {
        error = "Cannot create " + path;
        return false;
    }

    output << "# reservations v1\n";

    for (int i = 0; i < count; ++i)
    {
        output << reservations[i].bookId << '|'
               << reservations[i].studentId << '\n';
    }

    output.close();

    if (output.fail())
    {
        error = "Failed writing " + path;
        return false;
    }

    return true;
}

static bool replaceFile(const string& tempPath, const string& targetPath, string& error)
{
    string backupPath = targetPath + ".bak";

    if (fileExists(targetPath))
    {
        remove(backupPath.c_str());

        if (rename(targetPath.c_str(), backupPath.c_str()) != 0)
        {
            error = "Could not back up " + targetPath;
            return false;
        }
    }

    if (rename(tempPath.c_str(), targetPath.c_str()) != 0)
    {
        if (fileExists(backupPath))
        {
            rename(backupPath.c_str(), targetPath.c_str());
        }

        error = "Could not replace " + targetPath;
        return false;
    }

    return true;
}

bool saveAllData(const string& dataDir, string& error)
{
    if (!ensureDirectory(dataDir))
    {
        error = "Could not create data directory: " + dataDir;
        return false;
    }

    string booksPath = joinPath(dataDir, "books.txt");
    string studentsPath = joinPath(dataDir, "students.txt");
    string transactionsPath = joinPath(dataDir, "transactions.txt");
    string reservationsPath = joinPath(dataDir, "reservations.txt");

    string booksTemp = booksPath + ".tmp";
    string studentsTemp = studentsPath + ".tmp";
    string transactionsTemp = transactionsPath + ".tmp";
    string reservationsTemp = reservationsPath + ".tmp";

    int bookCount = getTotalBookTitles();
    int studentCount = getStudentCount();
    int transactionCount = getTransactionCount();
    int reservationCount = getReservationCount();

    BookInfo* books = new BookInfo[bookCount > 0 ? bookCount : 1];
    StudentInfo* students = new StudentInfo[studentCount > 0 ? studentCount : 1];
    TransactionInfo* transactions = new TransactionInfo[transactionCount > 0 ? transactionCount : 1];
    ReservationInfo* reservations = new ReservationInfo[reservationCount > 0 ? reservationCount : 1];

    collectBooks(books, bookCount);
    collectStudents(students, studentCount);
    collectTransactions(transactions, transactionCount);
    collectReservations(reservations, reservationCount);

    bool written = writeBooksFile(booksTemp, books, bookCount, error);

    if (written)
    {
        written = writeStudentsFile(studentsTemp, students, studentCount, error);
    }

    if (written)
    {
        written = writeTransactionsFile(transactionsTemp, transactions, transactionCount, error);
    }

    if (written)
    {
        written = writeReservationsFile(reservationsTemp, reservations, reservationCount, error);
    }

    delete[] reservations;
    delete[] transactions;
    delete[] students;
    delete[] books;

    if (!written)
    {
        remove(booksTemp.c_str());
        remove(studentsTemp.c_str());
        remove(transactionsTemp.c_str());
        remove(reservationsTemp.c_str());
        return false;
    }

    if (!replaceFile(booksTemp, booksPath, error))
    {
        remove(studentsTemp.c_str());
        remove(transactionsTemp.c_str());
        remove(reservationsTemp.c_str());
        return false;
    }

    if (!replaceFile(studentsTemp, studentsPath, error))
    {
        remove(transactionsTemp.c_str());
        remove(reservationsTemp.c_str());
        return false;
    }

    if (!replaceFile(transactionsTemp, transactionsPath, error))
    {
        remove(reservationsTemp.c_str());
        return false;
    }

    if (!replaceFile(reservationsTemp, reservationsPath, error))
    {
        return false;
    }

    return true;
}
