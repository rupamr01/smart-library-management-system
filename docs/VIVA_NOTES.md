# Viva Notes — Smart Library Management System

## Member 1 — Book Management

**Owned files:** `book/book.h`, `book/book.cpp`

**What to demonstrate**
- Add a book to an empty list.
- Add multiple books.
- Reject a duplicate ID.
- Find an existing and a missing book.
- Update a book and change total copies.
- Delete the head, a middle node, and the tail.

**Likely questions**
1. **Why a linked list instead of an array?**
   The number of books is not fixed. A linked list grows and shrinks without reallocating or shifting elements.
2. **Why insert at the head?**
   It is O(1). Display and lookup already require O(n), so maintaining sorted order on insert would add cost without improving the assessed lookup.
3. **How is deletion handled?**
   Keep `previous` and `current`. If the target is the head, move `head`; otherwise set `previous->next = current->next`. Then `delete current`.
4. **What happens when total copies are reduced below issued copies?**
   The update is rejected. Issued copies are `oldTotal - oldAvailable`; total cannot go below that.
5. **When can a book be deleted?**
   Only when it has no active loans, no pending reservations, and no transaction history. `main.cpp` performs these checks before calling the book module.
6. **What is the complexity?**
   Insert at head O(1); find, update, delete, and display O(n).

## Member 2 — Student Management

**Owned files:** `student/student.h`, `student/student.cpp`

**What to demonstrate**
- Insert root, left child, and right child.
- Reject a duplicate ID.
- Search hit and miss.
- Inorder display in ascending ID order.
- Delete a leaf, a one-child node, and a two-child node.

**Likely questions**
1. **What is the BST property?**
   For every node, all IDs in the left subtree are smaller and all IDs in the right subtree are larger.
2. **Why is inorder traversal sorted?**
   Inorder visits left subtree, node, then right subtree; by the BST property this produces ascending IDs.
3. **How does two-child deletion work?**
   Find the inorder successor (minimum of the right subtree), copy its fields into the node, then delete the successor.
4. **What is the complexity?**
   Search, insert, and delete are O(h), average O(log n) and worst O(n) for an ordinary unbalanced BST. Inorder is O(n).
5. **Why not claim guaranteed O(log n)?**
   An ordinary BST can become a chain if IDs arrive in sorted order; balancing (AVL/red-black) would be required for a guarantee.

## Member 3 — Issue, Return, Reservations, History

**Owned files:** `transaction/transaction.h`, `transaction/transaction.cpp`

**What to demonstrate**
- Issue a book with an available copy.
- Reject an unknown student/book, no copies, borrowing limit, or duplicate active loan.
- Reserve a book when no copies are available.
- Return a book and calculate a fine.
- Show auto-allocation to the next reservation holder.
- Show recent activity from the stack.

**Likely questions**
1. **Why a queue for reservations?**
   Reservations are first-come, first-served; a queue gives FIFO order.
2. **How is the queue implemented?**
   One linked queue per book, with `front` and `rear` pointers. Enqueue and dequeue are O(1) after locating the book queue.
3. **Why a stack for history?**
   Recent activity is naturally LIFO: the newest event is on top.
4. **How is the fine calculated?**
   `overdueDays = max(0, returnDate - dueDate)` and `fine = overdueDays × Rs. 5`.
5. **What happens when a reserved copy is returned?**
   Availability is incremented, then the queue is dequeued. Ineligible students are skipped. The first eligible student is issued the copy immediately, which decrements availability again.
6. **How are duplicate returns prevented?**
   The transaction has an `active` flag; returning an inactive transaction is rejected.
7. **How is availability kept consistent?**
   Issue decrements available copies, return increments them, and auto-allocation decrements them again through the normal issue path.

## Member 4 — Search, Sorting, Reports

**Owned files:** `search/search.h`, `search/search.cpp`

**What to demonstrate**
- Linear search by title and author.
- Binary search by ID on a sorted snapshot.
- Merge sort by title, author, year, and ID, ascending and descending.
- Library report totals.

**Likely questions**
1. **Why not binary search directly on the linked list?**
   Linked lists do not support O(1) random access. Binary search needs index access, so the module copies books into an array and sorts that array first.
2. **What is the precondition for binary search?**
   The array must be sorted in ascending order by book ID.
3. **How does merge sort work?**
   Recursively sort the left and right halves, then merge the two sorted halves into a temporary array.
4. **What is the complexity?**
   Merge sort is O(n log n) time and O(n) extra space. Binary search is O(log n) after sorting. Linear search is O(n) traversal.
5. **How are duplicate titles handled?**
   Comparison is case-insensitive, with book ID as a deterministic tie-breaker.
6. **Why copy records instead of sorting the live list?**
   Sorting the live linked list would change the book module's ownership and ordering; a snapshot keeps module boundaries clean.

## Integration Questions

1. **How do modules communicate?**
   Through public functions in headers. Transactions store IDs, not book/student pointers.
2. **Who prevents deleting a book with active loans?**
   `main.cpp` checks `hasActiveLoanForBook` and `hasPendingReservationForBook` before calling `deleteBook`.
3. **What happens if a data file is missing?**
   The loader treats it as an empty collection. An existing but unreadable file stops startup so data is not overwritten.
4. **What happens if a line is malformed?**
   The line is skipped and a warning with the file and line number is shown. Saving after warnings requires confirmation.
5. **How is persistence reconstructed?**
   Books are loaded into the linked list, students into the BST, transactions into the linked list, reservations into per-book queues, history is rebuilt, and availability is reconciled from active loans.
6. **How is memory cleaned up?**
   Each module has a `clear*` function; `main` calls all of them before exit.
