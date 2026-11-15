# Test Results — Smart Library Management System

## Environment

- Compiler: `g++.exe (MinGW.org GCC-6.3.0-1) 6.3.0`
- Standard: `-std=c++17`
- Flags: `-Wall -Wextra -pedantic`
- Platform: Windows (MinGW)

## Build Result

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp book/book.cpp student/student.cpp transaction/transaction.cpp search/search.cpp storage/storage.cpp -o library.exe
```

Result: **compiled successfully with no warnings.**

## Automated Test Result

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_core.cpp book/book.cpp student/student.cpp transaction/transaction.cpp search/search.cpp storage/storage.cpp -o test_core.exe
.\test_core.exe
```

Result:

```text
Tests passed: 132, failed: 0
```

### Coverage

| Area | Cases covered |
|---|---|
| Date utilities | Valid/invalid dates, leap year, month/year rollover, date difference |
| Book linked list | Add first/multiple, duplicate ID, find, update, copy-count bounds, delete head/middle/tail |
| Student BST | Insert root/left/right, duplicate ID, search hit/miss, delete leaf/one-child/two-child, count |
| Transactions | Issue, no copies, borrowing limit, duplicate active loan, return, double return, overdue fine, zero fine |
| Reservations | FIFO queue, duplicate rejection, cancellation, auto-allocation on return |
| History stack | Push order, top entry after return |
| Search/sort | Linear title/author, binary search hit/miss, merge sort title/year ascending/descending, one-item sort |
| Storage | Save/load round-trip, active loan restore, availability restore, malformed-line warning, missing-file start, historical transaction preservation |

## Manual End-to-End Test

Scenario executed through the console:

1. Added book `1` — *Data Structures*, Mark Allen Weiss, Pearson, 2020, 1 copy.
2. Registered students `1` (Alice) and `2` (Bob).
3. Issued book `1` to student `1`; transaction `1` was created with due date `2026-11-22`.
4. Attempted to issue book `1` to student `2`; rejected with “No copies are available.”
5. Added student `2` to the reservation queue.
6. Returned transaction `1` on `2026-11-20`; fine was Rs. 0.00.
7. The returned copy was automatically issued to student `2` as transaction `2`, due `2026-11-27`.
8. Recent activity showed `AUTO-ISSUE`, `RETURN`, `RESERVE`, and `ISSUE`.
9. Report showed 1 title, 1 physical copy, 0 available, 1 issued, 2 students, 1 active loan, 0 pending reservations.
10. Saved and exited.
11. Restarted the application; startup loaded 1 book, 2 students, 2 transactions, and 0 reservations.

Result: **passed.**

## Additional Manual Checks

- Search by ID, title, and availability worked.
- Sort by title ascending worked.
- Reservation queue view reported “No pending reservations” when empty.
- Save created `.bak` copies of all four data files.
- Restart preserved books, students, transactions, and active-loan state.
- Attempting to delete a book after a returned loan was blocked with “transaction history references this book.”
- Loading a file with a malformed line showed a warning and skipped only that line.
- Declining to save after a load warning did not overwrite the original file.

## Not Tested

- No AddressSanitizer/Valgrind run was possible with the installed MinGW GCC 6.3 toolchain. Memory cleanup was reviewed manually and all module `clear*` functions are called on exit.
- No multi-user or concurrent access testing; the application is single-process and console-based.

## Test Data

The repository data files were reset to empty after verification. Use the menu or edit the files to add demo records.
