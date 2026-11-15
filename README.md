# Smart Library Management System

A console-based library management application written in C++17. The project integrates four modules — books, students, transactions, and search/reports — and implements the core data structures and algorithms manually.

## Features

- Book management: add, update, delete, list, and find books.
- Student management: register, update, delete, and display students in ID order.
- Issue and return workflow with a 7-day loan period.
- Overdue fine calculation at Rs. 5 per day.
- Reservation waiting lists with FIFO allocation when a copy is returned.
- Active-loan limits and duplicate-loan protection.
- Search by book ID, title, author, and availability.
- Merge-sort listings by title, author, publication year, or ID.
- Library reports and recent activity history.
- Text-file persistence with validation, `.bak` backups, and safe save handling.

## Project Structure

```text
smart library management/
|-- main.cpp
|-- book/           Book linked list
|-- student/        Student binary search tree
|-- transaction/    Loans, reservation queues, history stack
|-- search/         Search, merge sort, reports
|-- storage/        Load/save routines
|-- common/         Shared models, configuration, validation, date helpers
|-- data/           Persistent text files
|-- tests/          Automated test harness
|-- docs/           Design, test results, report, viva notes
|-- build.bat
```

## Build

From the project root in PowerShell:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp book/book.cpp student/student.cpp transaction/transaction.cpp search/search.cpp storage/storage.cpp -o library.exe
```

Or run:

```powershell
.\build.bat
```

The project is compatible with the installed MinGW GCC 6.3.0. It is written in simple C++11/14 style and compiled with the C++17 flag. It avoids `std::filesystem` and other newer library features so it also works on older compilers.

## Run

```powershell
.\library.exe
```

## Automated Tests

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_core.cpp book/book.cpp student/student.cpp transaction/transaction.cpp search/search.cpp storage/storage.cpp -o test_core.exe
.\test_core.exe
```

The harness covers dates, book operations, BST deletion cases, issue/return/reservation rules, fines, history order, search, merge sort, binary search, and storage round-trips.

## Data Files

```text
data/
    books.txt
    students.txt
    transactions.txt
    reservations.txt
```

Format: pipe-delimited text with `\` escaping for literal pipes and backslashes. Each file starts with a version comment. Records are validated while loading. Invalid lines are reported and skipped.

On save, the application writes temporary files first, then replaces the originals and keeps a `.bak` copy. If a save fails, the error is reported; existing files are not silently discarded.

## Business Rules

- Book IDs and student IDs must be positive and unique.
- Total copies cannot be negative; `0 <= availableCopies <= totalCopies`.
- A book cannot be issued when no copy is available.
- A student can hold at most 3 active loans.
- A student cannot hold two active loans of the same book.
- A student with active loans or reservations cannot be deleted. Returned historical transactions may still reference the student.
- A book with active loans, pending reservations, or transaction history cannot be deleted.
- Returning the same loan twice is rejected.
- Reservations are FIFO per book and cannot be duplicated.
- A returned copy is auto-issued to the next eligible reservation holder, or becomes available if nobody is eligible.
- Loan period: 7 days. Fine: Rs. 5 per overdue day.

## Menu Overview

```text
1. Book Management
2. Student Management
3. Issue Book
4. Return Book
5. Search Books
6. Sort Books
7. Reservation Management
8. Transaction History
9. Library Reports
10. Save Data
0. Save and Exit
```

## DSA Summary

| Operation | Structure / Algorithm | Complexity |
|---|---|---|
| Book lookup | Singly linked list | O(n) |
| Book insert at head | Singly linked list | O(1) |
| Book update/delete | Singly linked list | O(n) |
| Student search/insert/delete | Ordinary BST | O(log n) average, O(n) worst |
| Student inorder display | BST traversal | O(n) |
| Reservation enqueue/dequeue | Linked queue with front/rear | O(1) |
| History push | Linked stack | O(1) |
| Linear search | Snapshot traversal | O(n) |
| Binary search | Sorted snapshot | O(log n) |
| Merge sort | Snapshot array | O(n log n) |
| Date arithmetic | Serial day numbers | O(1) |

## Documentation

- `docs/DESIGN.md` — architecture, interfaces, algorithms, file formats.
- `docs/TEST_RESULTS.md` — actual test and manual workflow results.
- `docs/REPORT.md` — project report with diagrams.
- `docs/VIVA_NOTES.md` — member-wise viva preparation.

## Notes

- Fines are displayed as `Rs.` because the default Windows console may not render the rupee symbol correctly.
- Recent activity is rebuilt from transaction records at startup. Auto-issued loans are shown as `ISSUE` after a restart because the history action itself is not persisted separately.
- Returned transactions keep their book/student IDs as history. If a student is deleted after all loans are returned, the historical transaction is preserved and a load warning is shown.
- The project uses `using namespace std;` and unqualified `cout`/`cin`/`string` for a simple academic style.
