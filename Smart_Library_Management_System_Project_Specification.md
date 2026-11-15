# Smart Library Management System — Project Specification

## 1. Project purpose

Build a fully working, console-based **Smart Library Management System** as a Data Structures and Algorithms (DSA) academic project.

The application will help a college library manage:
- Books and available copies
- Student/member records
- Book issuing and returning
- Reservations and waiting lists
- Searching and sorting
- Transaction history and library reports
- Persistent storage so records remain available after the program closes

The project must be a real, integrated application. It must not be a collection of disconnected demonstrations of data structures.

## 2. Default technical decisions

Use these defaults unless the course instructor requires otherwise:

| Item | Decision |
|---|---|
| Programming language | C++ |
| Interface | Console-based menu |
| Difficulty | Intermediate+ |
| Compiler | GCC / MinGW |
| IDE | VS Code or any C++ IDE |
| Data persistence | Text files |
| Build standard | C++17 |
| External libraries | None required |
| Core DSA | Implement manually; do not replace with STL containers |

Use `std::string` for text and standard input/output facilities, but manually implement the core linked list, binary search tree, stack, queue, searching and sorting algorithms. Avoid using `std::list`, `std::map`, `std::set`, `std::stack`, `std::queue` or `std::sort` for the assessed DSA operations.

## 3. Four-member team and ownership

Each member owns a separate module and must be able to explain and demonstrate it in the viva. All members should understand the overall workflow and integration.

### Member 1 — Book Management

**Responsibilities**
- Add a book.
- Update book details.
- Delete a book when safe to do so.
- Display all books.
- Track total copies and available copies.
- Validate unique book IDs.

**Primary DSA**
- Singly linked list for dynamic book records.
- Manual insertion, deletion, traversal and lookup.

**Suggested data model**
```cpp
struct Book {
    int id;
    std::string title;
    std::string author;
    std::string publisher;
    int publicationYear;
    int totalCopies;
    int availableCopies;
    Book* next;
};
```

**Viva topics:** pointers, dynamic memory allocation, linked-list traversal, insertion and deletion, time complexity.

**Owned files**
- `book/book.h`
- `book/book.cpp`

### Member 2 — Student Management

**Responsibilities**
- Register a student.
- Update student details.
- Remove a student only when they have no outstanding loans or reservations.
- Find a student by ID.
- Display students in ID order.
- Validate unique student IDs.

**Primary DSA**
- Binary Search Tree (BST), keyed by student ID.
- Manual insert, search, delete and inorder traversal.

**Suggested data model**
```cpp
struct Student {
    int id;
    std::string name;
    std::string course;
    std::string phone;
    Student* left;
    Student* right;
};
```

**Viva topics:** BST properties, recursion, inorder/preorder/postorder traversal, search/insert/delete complexity, worst-case BST behavior.

**Owned files**
- `student/student.h`
- `student/student.cpp`

### Member 3 — Issue, Return and Reservations

**Responsibilities**
- Issue a book to an eligible student.
- Return an issued book.
- Record issue date and due date.
- Calculate overdue fines.
- List active loans.
- Add a student to a reservation queue when a book has no available copies.
- When a copy is returned, offer it to the next eligible student in the queue.
- Maintain recent transaction history.

**Primary DSA**
- Queue for reservations (FIFO).
- Stack for recent transaction history (LIFO), if transaction history is implemented as a stack.
- Transaction records stored as structured data.

**Suggested data model**
```cpp
struct Transaction {
    int transactionId;
    int studentId;
    int bookId;
    std::string issueDate;  // YYYY-MM-DD
    std::string dueDate;    // YYYY-MM-DD
    std::string returnDate; // empty until returned
    double fine;
    bool active;
};
```

Implement the queue and stack manually using linked nodes or arrays with clear capacity/empty handling. A reservation should store at least the student ID and book ID. Do not allow duplicate active reservations for the same student and book.

**Viva topics:** FIFO, LIFO, enqueue, dequeue, push, pop, underflow/empty checks, transaction workflow.

**Owned files**
- `transaction/transaction.h`
- `transaction/transaction.cpp`

### Member 4 — Search, Sorting and Reports

**Responsibilities**
- Search books by ID, title or author.
- Search students by ID through the student module's BST interface.
- Sort book listings by title, author or publication year.
- Display available books.
- Generate basic library statistics:
  - Total book titles
  - Total physical copies
  - Available copies
  - Issued copies
  - Registered students
  - Active loans
  - Pending reservations
- Show recent activity using the transaction module's public interface.

**Primary DSA**
- Linear search for unsorted title/author searches.
- Binary search for book ID only when operating on a sorted array/list snapshot.
- Merge sort for sorting a temporary array of book records or pointers.
- Explain algorithmic complexity.

**Important:** A linked list is not suitable for efficient indexed binary search. To use binary search correctly, first create a sorted array snapshot of book references, then run binary search on that array. Do not claim binary search is being performed directly on a linked list.

**Viva topics:** linear search, binary search preconditions, merge sort, recursion, Big-O analysis.

**Owned files**
- `search/search.h`
- `search/search.cpp`

## 4. Shared integration contract

Before implementing modules, all members must agree on:
- Exact structure definitions
- Function names and parameter types
- Ownership of memory
- Error/return conventions
- File formats
- Date format
- Which module is allowed to modify each record

Use header files to expose module interfaces. Do not directly access another module's private linked-list or tree nodes. Prefer public functions.

Suggested public interfaces (adapt if needed, but keep one agreed version across the team):

```cpp
// book/book.h
bool addBook(/* book fields */);
bool updateBook(int bookId, /* updated fields */);
bool deleteBook(int bookId);
Book* findBookById(int bookId);
void displayAllBooks();
int getTotalBookTitles();
int getTotalCopies();
int getAvailableCopies();
bool changeAvailableCopies(int bookId, int delta);

// student/student.h
bool addStudent(/* student fields */);
bool updateStudent(int studentId, /* updated fields */);
bool deleteStudent(int studentId);
Student* findStudentById(int studentId);
void displayStudentsInOrder();
int getStudentCount();

// transaction/transaction.h
bool issueBook(int studentId, int bookId, const std::string& today);
bool returnBook(int transactionId, const std::string& today);
bool reserveBook(int studentId, int bookId);
void displayActiveLoans();
void displayRecentTransactions();
int getActiveLoanCount();
int getPendingReservationCount();

// search/search.h
void searchBooksMenu();
void sortBooksMenu();
void displayLibraryReports();
```

These are interface suggestions, not a requirement to expose raw node pointers publicly. A safer final design can use read-only record copies or accessor functions.

## 5. Core application workflow

### Main menu

```text
========================================
       SMART LIBRARY MANAGEMENT
========================================
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
========================================
Enter your choice:
```

### Issue workflow

1. Ask for student ID and book ID.
2. Confirm that the student exists.
3. Confirm that the book exists.
4. Check borrowing eligibility and any borrowing limit.
5. If a copy is available, decrease available copies and create an active transaction.
6. If no copy is available, offer the reservation queue.
7. Display a clear success or failure message.

### Return workflow

1. Ask for transaction ID (or another unambiguous active-loan identifier).
2. Confirm that the loan exists and is active.
3. Record return date.
4. Calculate fine if the return date is later than the due date.
5. Mark the transaction returned.
6. If a reservation is waiting, offer the returned copy to the next valid student; otherwise increase available copies.
7. Display the result.

### Reservation workflow

- Reservations are FIFO per book.
- A student must exist to reserve a book.
- Do not allow duplicate reservations by the same student for the same book.
- On return, skip invalid or no-longer-eligible queue entries safely.
- Define and document whether a reserved copy is temporarily held for the next student and how long it remains held. For a simple academic version, a returned copy can be allocated to the next student immediately.

## 6. Business rules and validation

Implement and test these rules:

1. Book IDs and student IDs must be unique.
2. IDs must be positive integers.
3. Total copies cannot be negative.
4. Available copies must always satisfy `0 <= availableCopies <= totalCopies`.
5. A book cannot be issued if no copies are available.
6. A student cannot exceed the configured borrowing limit (suggested default: 3 active loans).
7. A student with active loans cannot be deleted.
8. A book with active loans or pending reservations cannot be deleted.
9. Returning a loan twice must be rejected.
10. Invalid menu choices and malformed input must not crash the program.
11. When a book is deleted, ensure no transaction or reservation references it.
12. When a student is deleted, ensure no active transaction or reservation references them.
13. All data must be loaded at startup and saved on explicit save and normal exit.
14. If a data file is missing on first run, start with an empty collection; report other file errors clearly.
15. Never silently overwrite or discard data after a failed load.

## 7. Dates and fine calculation

Use one consistent date format: `YYYY-MM-DD`.

For a simple, reliable implementation, use C++ date/time utilities or a small validated date helper. Do not compare dates as arbitrary user-entered strings unless the format has been validated and is fixed-width ISO format.

Suggested default:
- Loan period: 7 days.
- Fine: ₹5 per overdue day.
- Due date: issue date + 7 calendar days.

Keep these values in one configuration section so they can be changed easily. Clearly label the fine as an academic-project rule, not an actual institutional policy.

## 8. Data persistence

Use separate text files:

```text
data/
    books.txt
    students.txt
    transactions.txt
    reservations.txt
```

Use a documented delimiter format or another simple robust text format. If using a delimiter, validate or escape delimiter characters in user-entered fields.

Recommended approach:
- Load all files during startup.
- Validate records as they load.
- Build the linked list and BST from loaded records.
- Rebuild reservation queues and transaction history.
- Save to temporary files first, then replace the old files after successful writes where practical.
- Check file-open, read and write errors.
- Save automatically on normal exit and offer a manual save option.

Do not save raw pointer addresses or binary memory dumps of linked-list/tree nodes. Save the record fields and reconstruct the data structures at startup.

## 9. Suggested folder structure

```text
Smart-Library-Management/
|
|-- main.cpp
|
|-- book/
|   |-- book.h
|   |-- book.cpp
|
|-- student/
|   |-- student.h
|   |-- student.cpp
|
|-- transaction/
|   |-- transaction.h
|   |-- transaction.cpp
|
|-- search/
|   |-- search.h
|   |-- search.cpp
|
|-- storage/
|   |-- storage.h
|   |-- storage.cpp
|
|-- common/
|   |-- models.h
|   |-- validation.h
|   |-- date_utils.h
|
|-- data/
|   |-- books.txt
|   |-- students.txt
|   |-- transactions.txt
|   |-- reservations.txt
|
|-- README.md
```

Keep shared model definitions in one agreed header to prevent conflicting duplicate definitions. The exact folder layout may be adjusted, but do not duplicate structure definitions across modules.

## 10. Development phases

### Phase 1 — Design
- Confirm all structure definitions and module interfaces.
- Draw the architecture and workflows.
- Agree on naming conventions, file formats and memory ownership.
- Create the repository and folder structure.

### Phase 2 — Independent modules
- Member 1 implements and tests the book linked list.
- Member 2 implements and tests the student BST.
- Member 3 implements and tests transaction, reservation queue and history stack.
- Member 4 implements and tests search, merge sort and reports using agreed interfaces.

### Phase 3 — Integration
- Connect modules through headers and public functions.
- Implement the main menu.
- Verify that book, student and transaction records stay consistent.
- Compile all source files together.

### Phase 4 — Persistence
- Implement load/save routines.
- Test first-run behavior and restart behavior.
- Verify that records survive closing and reopening the program.

### Phase 5 — Testing and debugging
- Test normal workflows and edge cases.
- Test empty linked list, empty BST, empty/full array queue if using one, empty stack and missing records.
- Test duplicate IDs, invalid input, unavailable books, overdue returns and duplicate reservations.
- Test corrupted or missing data files.
- Run memory checks where available.

### Phase 6 — Submission
- Prepare README and user instructions.
- Prepare project report, flowchart and architecture diagram.
- Document each DSA implementation and its time complexity.
- Prepare each member's individual contribution and viva explanation.
- Demonstrate an end-to-end issue/return/reservation workflow.

## 11. Testing checklist

### Book module
- Add first book to an empty list.
- Add multiple books.
- Reject duplicate ID.
- Find existing and missing IDs.
- Update existing and missing books.
- Delete head, middle and tail nodes.
- Display empty and non-empty lists.

### Student module
- Insert root, left child and right child.
- Reject duplicate ID.
- Search existing and missing students.
- Display inorder traversal in ascending ID order.
- Delete leaf, one-child and two-child nodes.
- Handle an empty tree.

### Transaction module
- Issue when copies are available.
- Reject issue for unknown student/book.
- Reject issue when no copies are available.
- Enforce borrowing limit.
- Return a valid active loan.
- Reject a second return of the same loan.
- Calculate zero and positive overdue fines.
- Enqueue and dequeue reservations in FIFO order.
- Handle empty queue and empty history stack.

### Search and sorting
- Linear search for title/author.
- Binary search on a sorted array snapshot by ID.
- Merge sort ascending and descending if supported.
- Test empty, one-item and duplicate-title cases.
- Verify sorted output is correct.

### Whole application
- Save, close, reopen and verify data.
- Check all menu options.
- Check malformed input.
- Verify copy counts never become negative or exceed total copies.
- Verify a returned copy is either allocated to a waiting reservation or made available.

## 12. DSA complexity guide

Document the actual complexity of the implemented version. Typical expected values:

| Operation | Data structure / algorithm | Typical complexity |
|---|---|---|
| Book lookup by ID | Linked list | O(n) |
| Book insertion at head | Linked list | O(1) |
| Book deletion by ID | Linked list | O(n) |
| Student search | Balanced BST | O(log n) average if balanced; O(n) worst case for ordinary BST |
| Student insertion | Ordinary BST | O(log n) average; O(n) worst case |
| Student inorder traversal | BST | O(n) |
| Reservation enqueue/dequeue | Linked queue with front/rear pointers | O(1) |
| History push/pop | Linked stack | O(1) |
| Linear search | Array/list traversal | O(n) |
| Binary search | Sorted array | O(log n) |
| Merge sort | Array snapshot | O(n log n) |

Do not describe an ordinary, unbalanced BST as guaranteed O(log n). State average and worst-case behavior accurately.

## 13. Build and run

Example GCC command from the project root:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic \
    main.cpp \
    book/book.cpp \
    student/student.cpp \
    transaction/transaction.cpp \
    search/search.cpp \
    storage/storage.cpp \
    -o library
```

On Windows with MinGW, run:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp book/book.cpp student/student.cpp transaction/transaction.cpp search/search.cpp storage/storage.cpp -o library.exe
.\library.exe
```

Adjust the source list if files are added or renamed. Compile frequently, not only at the end.

## 14. AI-assisted development rules

AI may be used to draft code, explain concepts, create tests and help debug. However:

1. Generate one module at a time.
2. Provide the shared project specification and agreed headers to the AI before asking it to code a module.
3. Ask for complete source and header files, not disconnected snippets.
4. Ask the AI to explain each function and the DSA used.
5. Compile and test each module before integration.
6. Do not allow the AI to silently change shared structure definitions or function signatures.
7. Ask for edge-case tests and complexity analysis.
8. Review generated code for memory leaks, dangling pointers, invalid input handling and data consistency.
9. Each member must understand their module well enough to explain it without reading the code line by line.
10. Keep a Git commit history showing meaningful contributions from all four members.

## 15. Expected final deliverables

- Fully working C++ console application.
- Four integrated modules with clearly documented ownership.
- Manual implementations of linked list, BST, queue, stack, merge sort, linear search and binary search.
- File-based persistent storage.
- Input validation and error handling.
- Test cases and test results.
- README with build/run instructions.
- Project report with objectives, scope, architecture, algorithms and complexity.
- Flowchart and DSA diagrams.
- Individual contribution summary for all four members.
- Viva preparation notes for each member.

## 16. Master instruction for an AI coding assistant

Use this specification as the source of truth for building the Smart Library Management System.

Work incrementally and produce a reliable, compilable project. Start by reviewing the specification, identifying any interface conflicts or missing decisions, and proposing the final shared data models and header-file contracts. Do not begin by generating the entire application in one giant response.

Then proceed in this order:
1. Create the folder structure and common model/interface headers.
2. Implement Member 1's book linked-list module and test it.
3. Implement Member 2's student BST module and test it.
4. Implement Member 3's transaction, reservation queue and history stack module and test it.
5. Implement Member 4's search, sorting and reporting module and test it.
6. Implement storage/load/save functionality.
7. Integrate everything through `main.cpp`.
8. Compile the complete project, fix all errors and warnings that matter, and run the testing checklist.
9. Produce README, documentation, complexity analysis and individual viva notes.

Important constraints:
- Use C++17.
- Keep the application console-based.
- Implement core DSA manually; do not substitute STL containers or sorting for assessed structures/algorithms.
- Keep interfaces consistent across all files.
- Do not invent successful test results. State exactly what has and has not been compiled or tested.
- Do not leave placeholder functions or pretend incomplete features are finished.
- Explain commands and file paths for a Windows/MinGW user when giving setup instructions.
- Preserve the four-member responsibility split.
- Prefer readable beginner/intermediate-level code with comments over unnecessarily clever code.
- If a requirement is ambiguous, choose the simplest academically sound behavior, document the choice and continue unless it would cause an interface-breaking decision.
