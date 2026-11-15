# DSA Used — Smart Library Management System

This document lists every data structure and algorithm used in the project, where it is implemented, and its complexity.

## 1. Quick Summary

| DSA | Purpose | Module |
|---|---|---|
| Singly linked list | Store book records | `book/book.cpp` |
| Singly linked list | Store transaction/loan records | `transaction/transaction.cpp` |
| Binary Search Tree (BST) | Store students by ID | `student/student.cpp` |
| Linked queue (FIFO) | Reservation waiting list per book | `transaction/transaction.cpp` |
| Linked stack (LIFO) | Recent activity history | `transaction/transaction.cpp` |
| Linear search | Search books by ID, title, author | `book/`, `search/` |
| Binary search | Search books by ID on a sorted snapshot | `search/search.cpp` |
| Merge sort | Sort books by title, author, year, ID | `search/search.cpp` |
| Merge sort | Rebuild history events chronologically | `transaction/transaction.cpp` |
| BST inorder traversal | Display students in ascending ID order | `student/student.cpp` |
| BST deletion | Delete leaf, one-child, two-child nodes | `student/student.cpp` |
| Serial-day date arithmetic | Due dates and overdue fines | `common/date_utils.h` |

## 2. Book Storage — Singly Linked List

Each book is a node:

```cpp
struct Book
{
    int id;
    string title;
    string author;
    string publisher;
    int publicationYear;
    int totalCopies;
    int availableCopies;
    Book* next;
};
```

Operations:

- Insert at head: O(1)
- Find by ID: O(n)
- Update: O(n)
- Delete: O(n)
- Display/traverse: O(n)

Why a linked list: the number of books is dynamic, and insertion does not require shifting elements or reallocating an array.

## 3. Student Storage — Binary Search Tree

The BST is keyed by student ID:

```cpp
struct Student
{
    int id;
    string name;
    string course;
    string phone;
    Student* left;
    Student* right;
};
```

Operations:

- Insert: O(h)
- Search: O(h)
- Delete: O(h)
- Inorder traversal: O(n)

Here `h` is the tree height. For an ordinary BST:

- Average case: O(log n)
- Worst case: O(n), if the tree becomes a chain

The project does **not** claim guaranteed O(log n) because the tree is not self-balancing.

### BST deletion cases

1. **Leaf node** — remove it directly.
2. **One child** — replace the node with its child.
3. **Two children** — copy the inorder successor (minimum of the right subtree), then delete the successor.

## 4. Reservations — FIFO Linked Queue per Book

Each book with reservations has its own queue:

```cpp
struct ReservationNode
{
    int studentId;
    ReservationNode* next;
};

struct BookQueue
{
    int bookId;
    ReservationNode* front;
    ReservationNode* rear;
    int count;
    BookQueue* next;
};
```

Operations:

- Enqueue: O(1) after locating the book queue
- Dequeue: O(1)
- Duplicate check: O(q), where `q` is the queue length
- Locate a book queue: O(k), where `k` is the number of books with reservations

FIFO is required so the student who reserved first gets the returned copy first.

## 5. Transaction History — LIFO Linked Stack

Recent activity is stored in a linked stack:

```cpp
struct HistoryNode
{
    HistoryInfo info;
    HistoryNode* next;
};
```

Operations:

- Push: O(1)
- Display newest `N` entries: O(N)

The newest action is at the top of the stack.

History is rebuilt at startup by creating `ISSUE` and `RETURN` events, merge-sorting them by date and transaction ID, then pushing them oldest-to-newest.

## 6. Transactions — Chronological Linked List

All loans are stored in a singly linked list with a tail pointer:

```cpp
struct Transaction
{
    int transactionId;
    int studentId;
    int bookId;
    string issueDate;
    string dueDate;
    string returnDate;
    double fine;
    bool active;
    Transaction* next;
};
```

- Append a new transaction: O(1) using the tail pointer
- Find by transaction ID: O(n)
- Count active loans: O(n)
- List active loans: O(n)

Transactions store book and student **IDs**, not pointers, so deleting a record cannot leave a dangling pointer.

## 7. Linear Search

Used for:

- Finding a book by ID in the linked list
- Searching books by title or author on a snapshot array
- Searching for active loans by student or book

Complexity:

- O(n) for the traversal
- O(n·L) in the worst case for case-insensitive substring matching, where `L` is the field length

Title and author search is case-insensitive and returns all matches.

## 8. Binary Search

Binary search is used only for book ID lookup on a **sorted array snapshot**.

Steps:

1. Copy book records into a temporary `BookInfo` array.
2. Sort the array by ID using merge sort.
3. Run binary search.

```text
low = 0
high = n - 1

while low <= high:
    middle = low + (high - low) / 2
    if array[middle].id == target: return middle
    if target < array[middle].id: high = middle - 1
    else: low = middle + 1

return -1
```

Complexity:

- Sort: O(n log n)
- Search: O(log n)

Binary search is **not** performed directly on the linked list because a linked list does not support O(1) index access.

## 9. Merge Sort

Used for:

- Sorting book snapshots by title, author, publication year, or ID
- Sorting history events by date during startup

```text
mergeSort(array, left, right):
    if left >= right: return
    middle = left + (right - left) / 2
    mergeSort(array, left, middle)
    mergeSort(array, middle + 1, right)
    merge two sorted halves into a temporary array
    copy back
```

Complexity:

- Time: O(n log n)
- Extra space: O(n)

The live book linked list is never sorted. Only a temporary array of value copies is sorted, so the book module keeps ownership and ordering.

## 10. Date Arithmetic — Serial Day Numbers

Dates are stored as `YYYY-MM-DD` and converted to serial day numbers for arithmetic:

```text
serial = number of days since a fixed epoch
```

This allows:

```text
dueDate = issueDate + 7 days
overdueDays = max(0, returnDate - dueDate)
fine = overdueDays * 5.0
```

Complexity: O(1) per operation.

The implementation validates leap years and month lengths, so invalid dates such as `2026-02-30` are rejected.

## 11. Availability Reconciliation

At startup, the system checks each book:

```text
expectedAvailable = totalCopies - activeLoanCount
```

If the stored value differs, it is corrected and a warning is shown. This keeps:

```text
0 <= availableCopies <= totalCopies
```

and prevents copy counts from becoming negative or exceeding the total.

## 12. File Parsing and Saving Algorithms

- **Escaping:** `\` becomes `\\`, and `|` becomes `\|` before saving.
- **Splitting:** the loader splits on unescaped `|` characters.
- **Validation:** IDs, dates, counts, references, and numeric fields are checked.
- **Atomic-style save:** write `.tmp` files first, then back up originals as `.bak`, then replace.
- **History rebuild:** merge-sort `ISSUE`/`RETURN` events by date.

Load/save complexity: O(n) per file, plus O(n log n) for history rebuilding.

## 13. Complexity Table

| Operation | Structure / Algorithm | Complexity |
|---|---|---|
| Book lookup by ID | Singly linked list | O(n) |
| Book insert at head | Singly linked list | O(1) |
| Book update/delete | Singly linked list | O(n) |
| Book display | Linked-list traversal | O(n) |
| Student search/insert/delete | Ordinary BST | O(log n) average, O(n) worst |
| Student inorder display | BST traversal | O(n) |
| Reservation enqueue/dequeue | Linked queue with front/rear | O(1) |
| Reservation duplicate check | Queue traversal | O(q) |
| History push | Linked stack | O(1) |
| Recent history display | Stack traversal | O(N) |
| Linear search | Snapshot traversal | O(n) |
| Binary search | Sorted snapshot | O(log n) |
| Merge sort | Snapshot array | O(n log n) |
| Date arithmetic | Serial day numbers | O(1) |
| Load/save | File I/O and parsing | O(n) per file |

## 14. What Is Not Used

The assessed DSA operations do **not** use:

- `std::list`
- `std::map`
- `std::set`
- `std::stack`
- `std::queue`
- `std::sort`

The linked list, BST, queue, stack, merge sort, linear search, and binary search are all implemented manually.

## 15. Viva Notes

Common points to remember:

1. Book lookup is O(n) because the live store is a linked list.
2. Student search is O(log n) average but O(n) worst because the BST is unbalanced.
3. Binary search requires a sorted array, so a snapshot is created first.
4. Reservations are FIFO per book.
5. History is LIFO.
6. Merge sort is O(n log n) and uses O(n) extra space.
7. Transactions and reservations store IDs, not pointers.
8. Availability is reconciled from active loans at startup.
9. Deletion is blocked for books with transaction history, active loans, or reservations.
10. A student with active loans or reservations cannot be deleted.

## 16. File References

| File | Contents |
|---|---|
| `book/book.cpp` | Book linked list |
| `student/student.cpp` | Student BST and traversal |
| `transaction/transaction.cpp` | Loans, reservation queue, history stack |
| `search/search.cpp` | Linear search, binary search, merge sort |
| `common/date_utils.h` | Serial-day date arithmetic |
| `storage/storage.cpp` | File parsing, validation, save/load |
| `tests/test_core.cpp` | Automated tests for the DSA operations |
