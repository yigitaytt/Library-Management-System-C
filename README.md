# 📚 Library Management System in C

A comprehensive Library Automation System developed in C programming language. This project demonstrates core computer science concepts including **Data Structures (Linked Lists)**, **File I/O Operations**, and **Dynamic Memory Management**.

## 🚀 Features
* **Book Management:** Add, delete, update, and search books.
* **Author & Student Management:** database management for library users and authors.
* **Lending System:** Borrow and return logic with date tracking.
* **Penalty System:** Automatic score deduction for late returns.
* **Data Persistence:** Uses CSV files (`books.csv`, `authors.csv`, etc.) to store data permanently.

## 🛠️ Technical Implementation
### 🧠 Memory Management & Pointers
* **Dynamic Memory Allocation:** Utilized `malloc` and `realloc` for flexible memory usage on the **Heap**, ensuring the program consumes only the necessary amount of RAM.
* **Leak Prevention:** Implemented dedicated "destructor-like" functions (e.g., `freeBookList`, `freeBookCopies`) that recursively traverse and free linked lists to prevent **memory leaks** upon program termination.
* **Pointer Arithmetic:** Extensive use of pointers for traversing lists and referencing data without unnecessary copying.

### 🏗️ Advanced Data Structures
* **Nested Linked Lists (One-to-Many Relationship):** Designed a complex struct architecture where each `Book` node points to a sub-linked list of `BookCopy` nodes. This simulates real-world "Parent-Child" database relationships in raw C.
* **Doubly Linked Lists:** Implemented for the Student database to enable efficient bi-directional traversal (`prev` and `next` pointers) and faster node deletion operations.

### 💾 Persistence & File I/O
* **Custom CSV Parsing:** Built a custom parser using `strtok` and file handling functions (`fopen`, `fprintf`, `fgets`) to simulate a relational database system.
* **State Preservation:** All runtime data (loans, users, books) is serialized into CSV files, ensuring data persistence across sessions.

### Prerequisites
* GCC Compiler (MinGW for Windows or standard gcc for Linux/Mac)

## 📂 File Structure
* `src/main.c`: The main source code containing all logic and structs.
* `*.csv`: Data files generated automatically upon first run.

---
**Author:** Yiğit Aytürk# Library-Management-System-C
