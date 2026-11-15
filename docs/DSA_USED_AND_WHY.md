# DSA Used and Why

## Smart Library Management System

This document explains every data structure and algorithm implemented in the project and, for each one, **why it was chosen**. All core structures are written manually in C++17 without STL containers or `std::sort`.

---

## 1. Summary

| DSA | Used for | Main reason |
|---|---|---|
| Singly linked list | Book records | Dynamic size, O(1) insert at head |
| Singly linked list | Loan/transaction records | O(1) append with tail pointer, stable chronological order |
| Binary Search Tree | Student records | Average O(log n) search/insert/delete, sorted inorder listing |
| Linked queue (FIFO) | Reservation waiting list | Fair first-come-first-served allocation |
| Linked stack (LIFO) | Recent activity history | Newest event displayed first, O(1) push |
| Linear search | ID, title, and author lookup | Works on unsorted data, supports substring matching |
| Binary search | Book lookup by ID | O(log n) search on a sorted array snapshot |
| Merge sort | Book and history sorting | Guaranteed O(n log n), stable, multi-key sorting |
| BST inorder traversal | Student display | Produces ascending ID order directly |
| BST deletion | Student removal | Correct handling of leaf, one-child, two-child cases |
| Serial-day arithmetic | Due dates and fines | O(1), reliable across months and leap years |

---

## 2. Singly Linked List — Books

**What it does:** Stores the live book catalogue. Each `Book` node holds the book fields and a `next` pointer. New books are inserted at the head; lookup, update, delete, and display walk the list.

**Why we used it:**

1. **Dynamic size.** The library can gain or lose titles at any time. A linked list grows and shrinks one node at a time with no fixed capacity and no resizing.
2. **O(1) insertion.** Adding a book at the head only rewires two pointers. An array would shift elements or reallocate.
3. **No wasted slots.** Memory is allocated only for books that actually exist.
4. **Simple deletion.** Removing a book unlinks one node; no elements need to be shifted.
5. **Matches the syllabus.** The specification requires a manual linked list, so it demonstrates pointer manipulation, traversal, and memory management.

**Trade-off:** Lookup by ID is O(n) because the list has no index. At the expected catalogue size this is acceptable, and the same traversal is needed for display anyway.

| Operation | Complexity |
|---|---|
| Insert at head | O(1) |
| Find / update / delete | O(n) |
| Display all | O(n) |

---

## 3. Singly Linked List — Transactions

**What it does:** Stores every loan in chronological order in a linked list with `head` and `tail` pointers. Each record keeps the transaction ID, student ID, book ID, issue/due/return dates, fine, and active flag.

**Why we used it:**

1. **O(1) append.** The tail pointer appends a new loan without scanning the list.
2. **Natural chronology.** Loans are created in time order and read in the same order for reports and history.
3. **Stable records.** Transactions store **IDs**, not book/student pointers, so deleting a book or student cannot leave a dangling reference.
4. **No shifting.** Unlike an array, inserting a loan never moves existing records.
5. **Full audit trail.** Returned loans stay in the list with `active = false`, preserving history for reports.

| Operation | Complexity |
|---|---|
| Append transaction | O(1) with tail pointer |
| Find by transaction ID | O(n) |
| Count/list active loans | O(n) |

---

## 4. Binary Search Tree — Students

**What it does:** Stores students keyed by student ID. Insert, search, and delete follow the BST property: smaller IDs go left, larger IDs go right.

**Why we used it:**

1. **Faster keyed lookup.** Average O(log n) search versus O(n) for a linked list.
2. **Ordered data for free.** Inorder traversal visits IDs in ascending order, so the student list is sorted without a separate sort step.
3. **Efficient dynamic updates.** Insert and delete stay O(log n) on average without rebuilding the structure.
4. **Demonstrates recursion.** Insert, search, delete, and traversal are naturally recursive tree algorithms.
5. **Covers all deletion cases.** The implementation handles leaf nodes, one-child nodes, and two-child nodes using the inorder successor.

**Trade-off:** The tree is not self-balancing. If IDs arrive in sorted order it can become a chain, making operations O(n) in the worst case. An AVL or red-black tree was not required and would add complexity beyond the specification.

| Operation | Average | Worst |
|---|---|---|
| Search | O(log n) | O(n) |
| Insert | O(log n) | O(n) |
| Delete | O(log n) | O(n) |
| Inorder traversal | O(n) | O(n) |

---

## 5. Linked Queue (FIFO) — Reservations

**What it does:** Maintains one waiting list per book. Each queue has `front` and `rear` pointers; students join at the rear and are served from the front.

**Why we used it:**

1. **Fairness.** Reservations must be first-come-first-served. A queue guarantees that order; a stack or unsorted list would not.
2. **O(1) enqueue and dequeue.** Rear insertion and front removal use direct pointers, with no traversal.
3. **Per-book queues.** A student waiting for one book does not block reservations for another book.
4. **Automatic allocation.** When a copy is returned, the queue front is dequeued and, if eligible, the copy is issued immediately.
5. **Duplicate protection.** A short queue scan prevents the same student from reserving the same book twice.

**Trade-off:** Locating the queue for a given book is O(k) in the number of books that currently have reservations. With a small number of reservation queues this is negligible.

| Operation | Complexity |
|---|---|
| Enqueue / dequeue | O(1) after locating the queue |
| Duplicate check | O(q) where q is queue length |

---

## 6. Linked Stack (LIFO) — Activity History

**What it does:** Stores recent actions (`ISSUE`, `RETURN`, `RESERVE`, `CANCEL`, `AUTO-ISSUE`) as a linked stack. The newest event is at the top.

**Why we used it:**

1. **Newest-first is the requirement.** “Recent activity” should show the latest event first, which is exactly LIFO behavior.
2. **O(1) push.** Recording an event only adds a node at the top.
3. **No searching.** Displaying recent activity simply walks down from the top for N entries.
4. **Simple integration.** Every workflow pushes one event without touching the transaction list structure.

History is rebuilt at startup from the transaction records using merge sort, so the stack is restored after a restart.

| Operation | Complexity |
|---|---|
| Push | O(1) |
| Display newest N | O(N) |

---

## 7. Linear Search

**What it does:** Scans records one by one. Used for book lookup by ID in the linked list, title/author substring search, and transaction scans.

**Why we used it:**

1. **The data is unsorted.** Title and author searches have no sorted index, so a linear scan is the natural choice.
2. **Substring matching.** Users search for part of a title or author, which binary search cannot do.
3. **Works on linked structures.** The live book list has no random access, so it can only be traversed sequentially.
4. **Simple and correct.** For the expected data size, O(n) is fast enough and avoids maintaining secondary indexes.

| Operation | Complexity |
|---|---|
| Scan | O(n) |
| Case-insensitive substring match | O(n·L) worst case, L = field length |

---

## 8. Binary Search

**What it does:** Finds a book ID in a **sorted array snapshot**. The search module copies books into a temporary array, merge-sorts it by ID, then applies binary search.

**Why we used it:**

1. **O(log n) lookup.** Far fewer comparisons than a linear scan on large catalogues.
2. **Demonstrates the precondition.** Binary search is only valid on sorted, indexable data; the project shows the required sort step explicitly.
3. **Linked lists cannot support it directly.** There is no O(1) index access in a linked list, so a snapshot array is created first.
4. **Keeps the live list untouched.** The book module keeps ownership and insertion order; only a value copy is sorted.

| Step | Complexity |
|---|---|
| Snapshot + merge sort | O(n log n) |
| Binary search | O(log n) |

---

## 9. Merge Sort

**What it does:** Sorts book snapshots by title, author, publication year, or ID in ascending or descending order. Also sorts history events by date when rebuilding the activity stack.

**Why we used it:**

1. **Guaranteed O(n log n).** Better than the O(n²) of bubble, selection, or insertion sort on larger catalogues.
2. **Stable ordering.** Equal keys keep their relative order; ties are also broken by book ID for deterministic output.
3. **Divide and conquer.** It clearly demonstrates recursion, splitting, and merging for the DSA assessment.
4. **Works well on arrays.** The merge step needs sequential access and temporary storage, which suits the snapshot array.
5. **Multi-key sorting.** One comparison function switches between title, author, year, and ID, so one algorithm serves all sort menu options.

**Trade-off:** It needs O(n) extra memory for the temporary array. That is acceptable for a console application of this size.

| Metric | Complexity |
|---|---|
| Time | O(n log n) |
| Extra space | O(n) |

---

## 10. BST Traversal and Deletion

**Inorder traversal** visits left subtree, node, then right subtree. Because of the BST property, this prints students in ascending ID order in O(n).

**Deletion** handles the three structural cases:

1. **Leaf node** — remove it directly.
2. **One child** — replace the node with its only child.
3. **Two children** — copy the inorder successor (minimum of the right subtree) into the node, then delete the successor.

**Why:** Correct deletion is essential to keep the BST property intact. Handling the two-child case with the inorder successor preserves ordering without rebuilding the tree.

---

## 11. Date Arithmetic — Serial Day Numbers

**What it does:** Converts `YYYY-MM-DD` dates to serial day numbers, performs addition/subtraction, and converts back.

**Why we used it:**

1. **Correct across months and leap years.** Adding 7 days to 28 February must produce 29 February in a leap year and 1 March otherwise.
2. **O(1) fine calculation.** `overdueDays = max(0, returnDate − dueDate)` becomes simple integer subtraction.
3. **No manual month tables in business logic.** Validation and conversion are centralized in `common/date_utils.h`.
4. **Reliable due dates.** Issue date + 7 days is computed without special cases.

Complexity: O(1) for parsing, conversion, and date difference.

---

## 12. Why No STL Containers

The project deliberately avoids `std::list`, `std::map`, `std::set`, `std::stack`, `std::queue`, and `std::sort` for the assessed operations.

**Reasons:**

1. **Specification requirement.** The linked list, BST, queue, stack, searching, and sorting must be implemented manually.
2. **Academic assessment.** The goal is to demonstrate understanding of pointers, recursion, and algorithm design, not library usage.
3. **Viva readiness.** Every operation can be explained line by line because the code is visible in the project.
4. **No hidden behavior.** Manual code exposes complexity, memory ownership, and edge cases.

Only standard support facilities are used: `string`, `iostream`, `iomanip`, `fstream`, `sstream`, and `ctime`.

---

## 13. Complexity Summary

| Operation | Structure / Algorithm | Complexity |
|---|---|---|
| Book insert at head | Singly linked list | O(1) |
| Book lookup / update / delete | Singly linked list | O(n) |
| Transaction append | Linked list with tail | O(1) |
| Student search / insert / delete | Ordinary BST | O(log n) average, O(n) worst |
| Student inorder display | BST traversal | O(n) |
| Reservation enqueue / dequeue | Linked queue | O(1) |
| Reservation duplicate check | Queue scan | O(q) |
| History push | Linked stack | O(1) |
| Recent history display | Stack traversal | O(N) |
| Book search by title / author | Linear search | O(n) |
| Book search by ID | Binary search on sorted snapshot | O(log n) |
| Book sorting | Merge sort | O(n log n) |
| History rebuild | Merge sort | O(n log n) |
| Date arithmetic | Serial day numbers | O(1) |

---

## 14. Design Rationale

Each structure was matched to how the data is used:

| Data | Access pattern | Chosen structure |
|---|---|---|
| Books | Frequent insert/delete, sequential display, occasional lookup | Linked list |
| Students | Keyed lookup, ordered listing, insert/delete | BST |
| Reservations | First-come-first-served per book | FIFO queue |
| Recent activity | Newest event first | LIFO stack |
| Loans | Append and chronological reporting | Linked list with tail |
| Unsorted text search | Partial title/author match | Linear search |
| Sorted ID search | Fast lookup on ordered data | Binary search |
| Book listings | Multi-key, stable ordering | Merge sort |

The result is a system where each data structure solves the problem it is best suited for, while every core implementation remains manual, readable, and explainable in a viva.

---

*Smart Library Management System — C++17 console application.*
