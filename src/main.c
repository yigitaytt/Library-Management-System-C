#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Constants
#define MAX_NAME_LEN 50
#define STUDENT_ID_LEN 9
#define ISBN_LEN 14
#define LABEL_LEN 30
#define DATE_LEN 11
#define OP_TYPE_BORROW 0
#define OP_TYPE_RETURN 1
#define DATE_STR_LEN 11
#define MAX_STATUS_LEN 20

// File Names 
#define FILE_AUTHORS "authors.csv"
#define FILE_STUDENTS "students.csv"
#define FILE_BOOKS "books.csv"
#define FILE_BOOK_AUTHORS "book_authors.csv"
#define FILE_LOANS "loans.csv"
#define FILE_COPIES "copies.csv" // Was "ornekler.csv"

// --- STRUCTS ---

typedef struct Author {
    int id;
    char name[MAX_NAME_LEN];
    char surname[MAX_NAME_LEN];
    struct Author *next;
} Author;

typedef struct Student {
    char studentId[STUDENT_ID_LEN];
    char name[MAX_NAME_LEN];
    char surname[MAX_NAME_LEN];
    int score;
    struct Student *prev;
    struct Student *next;
} Student;

typedef struct BookCopy {
    char labelNo[ISBN_LEN + 10]; // E.g., ISBN_1
    char borrowerStudentId[STUDENT_ID_LEN];
    char status[MAX_STATUS_LEN];
    struct BookCopy* next;
} BookCopy;

typedef struct Book {
    char title[MAX_NAME_LEN];
    char isbn[ISBN_LEN];
    int quantity;
    BookCopy* copies;
    struct Book* next;
} Book;

// Many-to-Many Relationship Map
typedef struct {
    char bookISBN[ISBN_LEN];
    int authorID;
} BookAuthorMap;

typedef struct LoanTransaction {
    char studentId[STUDENT_ID_LEN];
    char bookLabelNo[ISBN_LEN + 5];
    int operationType;
    char date[DATE_STR_LEN];
    struct LoanTransaction* next;
} LoanTransaction;

// --- PROTOTYPES ---
int isStudentExists(Student* head, const char * studentId);
int isBookOnShelf(Book* head, const char* labelNo);
int isStudentScorePositive(Student* head, const char* studentId);
void saveBooksToFile(Book* head, const char* filename);
void borrowBookCopy(Book** head, const char* label, const char * studentId);
int isBookCopyBorrowed(Book* head, const char* label, const char* studentId);
void returnBookCopy(Book** head, const char* label);
void updateStudentScore(Student** head, const char* studentId, int points);
void saveStudentsToFile(Student* head, const char* filename);
void freeBookCopies(BookCopy* head);
void listNonReturnedBooks(Student* sHead, Book* bHead);
void listAuthors(Author* head);
void saveBookAuthorMapToFile(BookAuthorMap* array, int count);
void saveBookCopiesToFile(Book* head, const char* filename);

// --- HELPER FUNCTIONS ---

int isStudentExists(Student* head, const char* studentId) {
    Student* current = head;
    while (current != NULL) {
        if (strcmp(current->studentId, studentId) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

const char* findBookLabelByISBN(Book* head, const char* isbn) {
    Book* kCurrent = head;
    while (kCurrent) {
        if (strcmp(kCurrent->isbn, isbn) == 0) {
            BookCopy* oCurrent = kCurrent->copies;
            while (oCurrent) {
                if (strcmp(oCurrent->borrowerStudentId, "SHELF") == 0) { // Was "RAFTA"
                    return oCurrent->labelNo;
                }
                oCurrent = oCurrent->next;
            }
        }
        kCurrent = kCurrent->next;
    }
    return NULL;
}

int isBookOnShelf(Book* head, const char* labelNo) {
    Book* kCurrent = head;
    while (kCurrent != NULL) {
        BookCopy* oCurrent = kCurrent->copies;
        while (oCurrent != NULL) {
            if (strcmp(oCurrent->labelNo, labelNo) == 0) {
                if (strcmp(oCurrent->borrowerStudentId, "SHELF") == 0) {
                    return 1; // On Shelf
                } else {
                    return 0; // Borrowed
                }
            }
            oCurrent = oCurrent->next;
        }
        kCurrent = kCurrent->next;
    }
    return 0;
}

int isStudentScorePositive(Student* head, const char* studentId) {
    Student* current = head;
    while (current != NULL) {
        if (strcmp(current->studentId, studentId) == 0) {
            return (current->score > 0) ? 1 : 0;
        }
        current = current->next;
    }
    return 0;
}

// --- AUTHOR FUNCTIONS ---

Author* createAuthor(int id, const char* name, const char* surname) {
    Author* newAuthor = (Author*) malloc(sizeof(Author));
    if (!newAuthor) return NULL;
    newAuthor->id = id;
    strncpy(newAuthor->name, name, sizeof(newAuthor->name));
    strncpy(newAuthor->surname, surname, sizeof(newAuthor->surname));
    newAuthor->next = NULL;
    return newAuthor;
}

void addAuthor(Author** head, const char* name, const char* surname) {
    int newID = 1;
    Author* temp = *head;
    Author* prev = NULL;

    while (temp && temp->next) {
        temp = temp->next;
    }
    if (temp) {
        newID = temp->id + 1;
    }

    Author* newAuthor = createAuthor(newID, name, surname);
    if (!newAuthor) {
        printf("Memory allocation error!\n");
        return;
    }

    if (*head == NULL) {
        *head = newAuthor;
        return;
    }

    // Insert sorted logic
    temp = *head;
    prev = NULL;
    while (temp && temp->id < newID) {
        prev = temp;
        temp = temp->next;
    }
    if (prev == NULL) {
        newAuthor->next = *head;
        *head = newAuthor;
    } else {
        prev->next = newAuthor;
        newAuthor->next = temp;
    }
}

void listAuthors(Author* head) {
    printf("ID\tName\tSurname\n");
    while (head) {
        printf("%d\t%s\t%s\n", head->id, head->name, head->surname);
        head = head->next;
    }
}

void saveAuthorsToFile(Author* head, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("Could not open file: %s\n", filename);
        return;
    }
    fprintf(fp, "AuthorID,Name,Surname\n");
    while (head) {
        fprintf(fp, "%d,%s,%s\n", head->id, head->name, head->surname);
        head = head->next;
    }
    fclose(fp);
}

Author* loadAuthorsFromFile(int* lastID) {
    FILE* fp = fopen(FILE_AUTHORS, "r");
    if (!fp) {
        printf("File not found: %s\n", FILE_AUTHORS);
        *lastID = 0;
        return NULL;
    }
    char line[256];
    Author* head = NULL;
    *lastID = 0;
    fgets(line, sizeof(line), fp); // Skip header

    while (fgets(line, sizeof(line), fp)) {
        int id;
        char name[MAX_NAME_LEN], surname[MAX_NAME_LEN];
        if (sscanf(line, "%d,%49[^,],%49[^\n]", &id, name, surname) == 3) {
            Author* newAuthor = (Author*)malloc(sizeof(Author));
            if (!newAuthor) return NULL;
            newAuthor->id = id;
            strncpy(newAuthor->name, name, MAX_NAME_LEN);
            strncpy(newAuthor->surname, surname, MAX_NAME_LEN);
            newAuthor->next = head;
            head = newAuthor;
            if (id > *lastID) *lastID = id;
        }
    }
    fclose(fp);

    // Sort list 
    if (!head) return NULL;
    Author* sorted = NULL;
    while (head) {
        Author* minPrev = NULL;
        Author* minNode = head;
        Author* prev = head;
        Author* cur = head->next;
        while (cur) {
            if (cur->id < minNode->id) {
                minPrev = prev;
                minNode = cur;
            }
            prev = cur;
            cur = cur->next;
        }
        if (minPrev) minPrev->next = minNode->next;
        else head = minNode->next;

        minNode->next = sorted;
        sorted = minNode;
    }
    return sorted; 
}

void removeAuthorFromBooks(BookAuthorMap** mapArray, int* count, int authorID) {
    for (int i = 0; i < *count; i++) {
        if ((*mapArray)[i].authorID == authorID) {
            (*mapArray)[i].authorID = -1;
        }
    }
    saveBookAuthorMapToFile(*mapArray, *count);
}

Author* deleteAuthor(Author* head, int id, BookAuthorMap** mapArray, int* mapCount) {
    if (!head) return NULL;
    Author* temp = head;
    Author* prev = NULL;

    if (head->id == id) {
        head = head->next;
        removeAuthorFromBooks(mapArray, mapCount, id);
        free(temp);
        return head;
    }

    while (temp && temp->id != id) {
        prev = temp;
        temp = temp->next;
    }

    if (!temp) {
        printf("Author not found: %d\n", id);
        return head;
    }

    prev->next = temp->next;
    removeAuthorFromBooks(mapArray, mapCount, id);
    free(temp);
    return head;
}

int updateAuthor(Author* head, int id, const char* newName, const char* newSurname) {
    Author* iter = head;
    while (iter) {
        if (iter->id == id) {
            strncpy(iter->name, newName, MAX_NAME_LEN);
            strncpy(iter->surname, newSurname, MAX_NAME_LEN);
            return 1;
        }
        iter = iter->next;
    }
    return 0;
}

// --- STUDENT FUNCTIONS ---

Student* addStudent(Student* head, const char* id, const char* name, const char* surname) {
    Student* newNode = (Student*)malloc(sizeof(Student));
    if (!newNode) return head;
    strncpy(newNode->studentId, id, STUDENT_ID_LEN);
    strncpy(newNode->name, name, MAX_NAME_LEN);
    strncpy(newNode->surname, surname, MAX_NAME_LEN);
    newNode->score = 100;
    newNode->prev = NULL;
    newNode->next = NULL;

    if (!head) return newNode;

    Student* iter = head;
    while (iter && strcmp(iter->studentId, id) < 0)
        iter = iter->next;

    if (!iter) {
        Student* tail = head;
        while (tail->next) tail = tail->next;
        tail->next = newNode;
        newNode->prev = tail;
        return head;
    }

    if (iter == head) {
        newNode->next = head;
        head->prev = newNode;
        return newNode;
    }

    newNode->next = iter;
    newNode->prev = iter->prev;
    iter->prev->next = newNode;
    iter->prev = newNode;
    return head;
}

void deleteStudent(Student** head, const char* id) {
    Student* temp = *head;
    Student* prev = NULL;
    while (temp && strcmp(temp->studentId, id) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if (!temp) return;

    if (!prev) *head = temp->next;
    else prev->next = temp->next;
    free(temp);
}

int updateStudent(Student* head, const char* id, const char* newName, const char* newSurname, int newScore) {
    Student* iter = head;
    while (iter) {
        if (strcmp(iter->studentId, id) == 0) {
            strncpy(iter->name, newName, MAX_NAME_LEN);
            strncpy(iter->surname, newSurname, MAX_NAME_LEN);
            iter->score = newScore;
            return 1;
        }
        iter = iter->next;
    }
    return 0;
}

Student* loadStudentsFromFile() {
    FILE* fp = fopen(FILE_STUDENTS, "r");
    if (!fp) {
        printf("File not found: %s\n", FILE_STUDENTS);
        return NULL;
    }
    char line[256];
    Student* head = NULL;
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        char id[STUDENT_ID_LEN], name[MAX_NAME_LEN], surname[MAX_NAME_LEN];
        int score;
        if (sscanf(line, "%8[^,],%49[^,],%49[^,],%d", id, name, surname, &score) == 4) {
            head = addStudent(head, id, name, surname);
            updateStudent(head, id, name, surname, score);
        }
    }
    fclose(fp);
    return head;
}

void saveStudentsToFile(Student* head, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return;
    fprintf(fp, "StudentID,Name,Surname,Score\n");
    while (head) {
        fprintf(fp, "%s,%s,%s,%d\n", head->studentId, head->name, head->surname, head->score);
        head = head->next;
    }
    fclose(fp);
}

// --- BOOK FUNCTIONS ---

Book* addBook(Book* head, const char* title, const char* isbn, int qty, Book** newBookRef) {
    Book* newBook = (Book*)malloc(sizeof(Book));
    if (!newBook) return head;
    strncpy(newBook->title, title, MAX_NAME_LEN);
    strncpy(newBook->isbn, isbn, ISBN_LEN);
    newBook->quantity = qty;
    newBook->next = NULL;
    newBook->copies = NULL;

    for (int i = 1; i <= qty; i++) {
        BookCopy* copy = (BookCopy*)malloc(sizeof(BookCopy));
        if (!copy) return NULL;
        snprintf(copy->labelNo, sizeof(copy->labelNo), "%s_%d", isbn, i);
        strcpy(copy->borrowerStudentId, "SHELF");
        copy->next = newBook->copies;
        newBook->copies = copy;
    }

    *newBookRef = newBook;

    if (!head) return newBook;

    Book* iter = head;
    Book* prev = NULL;

    while (iter && strcmp(iter->title, title) < 0) {
        prev = iter;
        iter = iter->next;
    }
    while (iter && strcmp(iter->title, title) == 0 && strcmp(iter->isbn, isbn) < 0) {
        prev = iter;
        iter = iter->next;
    }

    if (!prev) {
        newBook->next = head;
        return newBook;
    }

    prev->next = newBook;
    newBook->next = iter;
    return head;
}

void deleteBook(Book** head, const char* isbn) {
    Book* temp = *head;
    Book* prev = NULL;

    while(temp && strcmp(temp->isbn, isbn) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if (!temp) return;

    if (!prev) *head = temp->next;
    else prev->next = temp->next;

    freeBookCopies(temp->copies);
    free(temp);
}

int updateBook(Book* head, const char* isbn, const char* newTitle, int newQty) {
    Book* iter = head;
    int found = 0;
    while (iter && !found) {
        if (strcmp(iter->isbn, isbn) == 0) {
            found = 1;
            strncpy(iter->title, newTitle, MAX_NAME_LEN);

            if (newQty > iter->quantity) {
                int i = iter->quantity + 1;
                while (i <= newQty) {
                    BookCopy* copy = (BookCopy*)malloc(sizeof(BookCopy));
                    if (copy) {
                        snprintf(copy->labelNo, sizeof(copy->labelNo), "%s_%d", isbn, i);
                        strcpy(copy->borrowerStudentId, "SHELF");
                        copy->next = iter->copies;
                        iter->copies = copy;
                    }
                    i++;
                }
            }
            else if (newQty < iter->quantity) {
                int toDelete = iter->quantity - newQty;
                int deleted = 0;
                while (deleted < toDelete && iter->copies) {
                    BookCopy* temp = iter->copies;
                    iter->copies = temp->next;
                    free(temp);
                    deleted++;
                }
            }
            iter->quantity = newQty;
        }
        iter = iter->next;
    }
    return found;
}

Book* loadBooksFromFile(const char* bookFile, const char* copiesFile) {
    Book* head = NULL;
    FILE* fp = fopen(bookFile, "r");
    if (!fp) {
        printf("Book file not found: %s\n", bookFile);
        return NULL;
    }
    char line[256];
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        char title[MAX_NAME_LEN], isbn[ISBN_LEN];
        int qty;
        if (sscanf(line, "%49[^,],%13[^,],%d", title, isbn, &qty) == 3) {
            Book* newBook = NULL;
            head = addBook(head, title, isbn, qty, &newBook);
        }
    }
    fclose(fp);
    
    // Note: Copies loading logic (below)
    return head;
}

void saveBooksToFile(Book* head, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return;
    fprintf(fp, "Title,ISBN,Quantity\n");
    while (head) {
        fprintf(fp, "%s,%s,%d\n", head->title, head->isbn, head->quantity);
        head = head->next;
    }
    fclose(fp);
}

void saveBookCopiesToFile(Book* head, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return;
    fprintf(fp, "LabelNo,ISBN,BorrowerID\n");
    while (head) {
        BookCopy* copy = head->copies;
        while (copy) {
            fprintf(fp, "%s,%s,%s\n", copy->labelNo, head->isbn, copy->borrowerStudentId);
            copy = copy->next;
        }
        head = head->next;
    }
    fclose(fp);
}

void loadBookCopiesFromFile(Book* head, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return;
    char line[256];
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        char* label = strtok(line, ",\n");
        char* isbn = strtok(NULL, ",\n");
        char* borrower = strtok(NULL, ",\n");

        if (label && isbn && borrower) {
            int found = 0;
            Book* bIter = head;
            while (bIter && !found) {
                if (strcmp(bIter->isbn, isbn) == 0) {
                    BookCopy* cIter = bIter->copies;
                    while (cIter) {
                        if (strcmp(cIter->labelNo, label) == 0) {
                            strncpy(cIter->borrowerStudentId, borrower, STUDENT_ID_LEN);
                            found = 1;
                        }
                        cIter = cIter->next;
                    }
                }
                bIter = bIter->next;
            }
        }
    }
    fclose(fp);
}

// --- BOOK-AUTHOR MAP FUNCTIONS ---

BookAuthorMap* loadBookAuthorMap(int* count) {
    FILE* fp = fopen(FILE_BOOK_AUTHORS, "r");
    if (!fp) {
        *count = 0;
        return NULL;
    }
    char line[256];
    int c = 0;
    while (fgets(line, sizeof(line), fp)) c++;
    rewind(fp);

    if (c == 0) {
        *count = 0;
        fclose(fp);
        return NULL;
    }
    BookAuthorMap* arr = (BookAuthorMap*)malloc(sizeof(BookAuthorMap) * c);
    if (!arr) { fclose(fp); return NULL; }

    int idx = 0;
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%13[^,],%d", arr[idx].bookISBN, &arr[idx].authorID);
        idx++;
    }
    *count = c;
    fclose(fp);
    return arr;
}

void saveBookAuthorMapToFile(BookAuthorMap* arr, int count) {
    FILE* fp = fopen(FILE_BOOK_AUTHORS, "w");
    if (!fp) return;
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%d\n", arr[i].bookISBN, arr[i].authorID);
    }
    fclose(fp);
}

int addBookAuthorRelation(BookAuthorMap** arr, int* count, const char* isbn, int authorID) {
    for (int i = 0; i < *count; i++) {
        if (strcmp((*arr)[i].bookISBN, isbn) == 0 && (*arr)[i].authorID == authorID)
            return 0; // Already exists
    }
    BookAuthorMap* newArr = (BookAuthorMap*)realloc(*arr, sizeof(BookAuthorMap) * (*count + 1));
    if (!newArr) return 0;
    *arr = newArr;
    strncpy((*arr)[*count].bookISBN, isbn, ISBN_LEN);
    (*arr)[*count].authorID = authorID;
    (*count)++;
    return 1;
}

int removeBookAuthorRelation(BookAuthorMap** arr, int* count, const char* isbn, int authorID) {
    int found = 0;
    int i = 0;
    while (i < *count && !found) {
        if (strcmp((*arr)[i].bookISBN, isbn) == 0 && (*arr)[i].authorID == authorID) {
            found = 1;
            (*arr)[i] = (*arr)[*count - 1];
            (*count)--;
            *arr = (BookAuthorMap*)realloc(*arr, sizeof(BookAuthorMap) * (*count));
        }
        i++;
    }
    return found;
}

// --- LOAN FUNCTIONS ---

void saveLoansToFile(LoanTransaction* head) {
    FILE* fp = fopen(FILE_LOANS, "w");
    if (!fp) return;
    LoanTransaction* iter = head;
    while (iter) {
        fprintf(fp, "%s,%s,%d,%s\n", iter->studentId, iter->bookLabelNo, iter->operationType, iter->date);
        iter = iter->next;
    }
    fclose(fp);
}

void addLoanTransaction(LoanTransaction** head, const char* sId, const char* label, int type, const char* date) {
    LoanTransaction* newNode = (LoanTransaction*)malloc(sizeof(LoanTransaction));
    if (!newNode) return;
    strncpy(newNode->studentId, sId, STUDENT_ID_LEN - 1);
    newNode->studentId[STUDENT_ID_LEN - 1] = '\0';

    strncpy(newNode->bookLabelNo, label, sizeof(newNode->bookLabelNo) - 1);
    newNode->bookLabelNo[sizeof(newNode->bookLabelNo) - 1] = '\0';

    newNode->operationType = type;
    strncpy(newNode->date, date, sizeof(newNode->date) - 1);
    newNode->date[sizeof(newNode->date) - 1] = '\0';

    newNode->next = *head;
    *head = newNode;
}

void borrowBookCopy(Book** head, const char* label, const char* sId) {
    Book* bIter = *head;
    while (bIter) {
        BookCopy* cIter = bIter->copies;
        while (cIter) {
            if (strcmp(cIter->labelNo, label) == 0) {
                strncpy(cIter->borrowerStudentId, sId, STUDENT_ID_LEN);
                saveBookCopiesToFile(*head, FILE_COPIES);
                return;
            }
            cIter = cIter->next;
        }
        bIter = bIter->next;
    }
    printf("Copy not found: %s\n", label);
}

int processLoan(Student** sHead, Book** bHead, LoanTransaction** lHead, const char* sId, const char* isbn, const char* date) {
    if (!isStudentExists(*sHead, sId)) {
        printf("Error: Student not found!\n");
        return 0;
    }
    if (!isStudentScorePositive(*sHead, sId)) {
        printf("Error: Student score insufficient!\n");
        return 0;
    }
    const char* label = findBookLabelByISBN(*bHead, isbn);
    if (!label) {
        printf("Error: No copies available on shelf!\n");
        return 0;
    }
    borrowBookCopy(bHead, label, sId);
    addLoanTransaction(lHead, sId, label, OP_TYPE_BORROW, date);
    saveLoansToFile(*lHead);
    return 1;
}

int getDaysDifference(const char* start, const char* end) {
    int y1, m1, d1, y2, m2, d2;
    sscanf(start, "%d-%d-%d", &d1, &m1, &y1);
    sscanf(end, "%d-%d-%d", &d1, &m2, &y2);
    struct tm tm1 = {0}, tm2 = {0};
    tm1.tm_year = y1 - 1900; tm1.tm_mon = m1 - 1; tm1.tm_mday = d1;
    tm2.tm_year = y2 - 1900; tm2.tm_mon = m2 - 1; tm2.tm_mday = d2;
    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);
    return (int)difftime(t2, t1) / (60 * 60 * 24);
}

char* findBorrowDate(LoanTransaction* head, const char* sId, const char* label) {
    while (head) {
        if (strcmp(head->studentId, sId) == 0 &&
            strcmp(head->bookLabelNo, label) == 0 &&
            head->operationType == OP_TYPE_BORROW) {
            return head->date;
        }
        head = head->next;
    }
    return NULL;
}

void returnBookCopy(Book** head, const char* label) {
    Book* bIter = *head;
    while (bIter) {
        BookCopy* cIter = bIter->copies;
        while (cIter) {
            if (strcmp(cIter->labelNo, label) == 0) {
                strcpy(cIter->borrowerStudentId, "SHELF");
                return;
            }
            cIter = cIter->next;
        }
        bIter = bIter->next;
    }
}

void updateStudentScore(Student** head, const char* sId, int points) {
    Student* iter = *head;
    while (iter) {
        if (strcmp(iter->studentId, sId) == 0) {
            iter->score += points;
            return;
        }
        iter = iter->next;
    }
}

int isBookCopyBorrowed(Book* head, const char* label, const char* sId) {
    Book* bIter = head;
    while (bIter) {
        BookCopy* cIter = bIter->copies;
        while (cIter) {
            if (strcmp(cIter->labelNo, label) == 0) {
                if (strcmp(cIter->borrowerStudentId, sId) == 0) return 1;
                else return 0;
            }
            cIter = cIter->next;
        }
        bIter = bIter->next;
    }
    return 0;
}

int processReturn(Student** sHead, Book** bHead, LoanTransaction** lHead, const char* sId, const char* label, const char* date) {
    if (!isStudentExists(*sHead, sId)) {
        printf("Student not found.\n"); return 0;
    }
    if (!isBookCopyBorrowed(*bHead, label, sId)) {
        printf("Error: This book is not borrowed by this student.\n"); return 0;
    }
    char* borrowDate = findBorrowDate(*lHead, sId, label);
    if (!borrowDate) {
        printf("Error: Loan record not found.\n"); return 0;
    }
    int diff = getDaysDifference(borrowDate, date);
    if (diff > 15) {
        updateStudentScore(sHead, sId, -10);
    }
    returnBookCopy(bHead, label);
    addLoanTransaction(lHead, sId, label, OP_TYPE_RETURN, date);
    saveLoansToFile(*lHead);
    saveBookCopiesToFile(*bHead, FILE_COPIES);
    saveStudentsToFile(*sHead, FILE_STUDENTS);
    printf("Book returned successfully.\n");
    return 1;
}

LoanTransaction* loadLoansFromFile() {
    FILE* fp = fopen(FILE_LOANS, "r");
    if (!fp) return NULL;
    LoanTransaction* head = NULL;
    LoanTransaction* tail = NULL;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        LoanTransaction* newNode = (LoanTransaction*)malloc(sizeof(LoanTransaction));
        if (!newNode) { fclose(fp); return NULL; }
        char* token = strtok(line, ",");
        if(token) strncpy(newNode->studentId, token, STUDENT_ID_LEN);
        token = strtok(NULL, ",");
        if(token) strncpy(newNode->bookLabelNo, token, sizeof(newNode->bookLabelNo));
        token = strtok(NULL, ",");
        if(token) newNode->operationType = atoi(token);
        token = strtok(NULL, ",\n");
        if(token) strncpy(newNode->date, token, DATE_STR_LEN);
        newNode->next = NULL;
        if (!head) head = tail = newNode;
        else { tail->next = newNode; tail = newNode; }
    }
    fclose(fp);
    return head;
}

// --- MENUS ---

void menuAddAuthor(Author** head) {
    char name[50], surname[50];
    printf("Name: "); fgets(name, 50, stdin); name[strcspn(name, "\n")] = 0;
    printf("Surname: "); fgets(surname, 50, stdin); surname[strcspn(surname, "\n")] = 0;
    addAuthor(head, name, surname);
    saveAuthorsToFile(*head, FILE_AUTHORS);
}

void menuDeleteAuthor(Author** head, BookAuthorMap** arr, int* count) {
    int id;
    printf("Author ID to delete: "); scanf("%d", &id); while(getchar()!='\n');
    *head = deleteAuthor(*head, id, arr, count);
    saveAuthorsToFile(*head, FILE_AUTHORS);
}

void menuAuthors(Author** head, BookAuthorMap** arr, int* count) {
    int choice;
    do {
        printf("\n--- Author Menu ---\n1. Add Author\n2. Delete Author\n3. List Authors\n0. Back\nChoice: ");
        scanf("%d", &choice); while(getchar()!='\n');
        switch(choice) {
            case 1: menuAddAuthor(head); break;
            case 2: menuDeleteAuthor(head, arr, count); break;
            case 3: listAuthors(*head); break;
        }
    } while(choice != 0);
}

void menuStudents(Student** sHead, Book** bHead, LoanTransaction** lHead) {
    int choice;
    do {
        printf("\n--- Student Menu ---\n1. Add Student\n2. Delete Student\n3. List Students\n4. Borrow/Return\n0. Back\nChoice: ");
        scanf("%d", &choice); 
        while(getchar()!='\n'); // Buffer temizliği

        switch(choice) {
            case 1: {
                // Buffer boyutlarını güvenli hale getirdik (9 -> 20)
                char id[20], name[50], sur[50]; 
                
                printf("ID: "); 
                fgets(id, sizeof(id), stdin); 
                id[strcspn(id, "\n")] = 0;

                printf("Name: "); 
                fgets(name, sizeof(name), stdin); 
                name[strcspn(name, "\n")] = 0;

                printf("Surname: "); 
                fgets(sur, sizeof(sur), stdin); 
                sur[strcspn(sur, "\n")] = 0;

                *sHead = addStudent(*sHead, id, name, sur);
                saveStudentsToFile(*sHead, FILE_STUDENTS);
                break;
            }
            case 2: {
                char id[20]; // Burayı da arttırdık
                printf("ID to delete: "); 
                fgets(id, sizeof(id), stdin); 
                id[strcspn(id, "\n")] = 0;
                
                deleteStudent(sHead, id);
                saveStudentsToFile(*sHead, FILE_STUDENTS);
                break;
            }
            case 3: {
                Student* t = *sHead;
                printf("ID\tName\tScore\n");
                while(t){ 
                    printf("%s\t%s %s\t%d\n", t->studentId, t->name, t->surname, t->score); 
                    t=t->next; 
                }
                break;
            }
            case 4: {
                int op;
                // KRİTİK DÜZELTME BURADA: sId[9] yerine sId[20] yaptık
                char sId[20], info[50], date[20]; 
                
                printf("1. Borrow\n2. Return\nSelect: "); 
                scanf("%d", &op); 
                while(getchar()!='\n'); // scanf sonrası temizlik

                printf("Student ID: "); 
                fgets(sId, sizeof(sId), stdin); 
                sId[strcspn(sId, "\n")] = 0;

                if(op==1) { 
                    printf("Book ISBN: "); 
                    fgets(info, sizeof(info), stdin); 
                    info[strcspn(info, "\n")] = 0; 
                }
                else { 
                    printf("Label (ISBN_1): "); 
                    fgets(info, sizeof(info), stdin); 
                    info[strcspn(info, "\n")] = 0; 
                }

                printf("Date (DD.MM.YYYY): "); 
                fgets(date, sizeof(date), stdin); 
                date[strcspn(date, "\n")] = 0;
                
                if(op==1) processLoan(sHead, bHead, lHead, sId, info, date);
                else processReturn(sHead, bHead, lHead, sId, info, date);
                break;
            }
        }
    } while(choice!=0);
}

// --- BOOK AUTHOR LINKING MENU ---

void menuLinkBookAuthor(Book* bHead, Author* aHead, BookAuthorMap** map, int* count) {
    char isbn[14];
    int authorId;

    printf("\n--- Available Books ---\n");
    Book* bTmp = bHead;
    while(bTmp) {
        printf("ISBN: %s | Title: %s\n", bTmp->isbn, bTmp->title);
        bTmp = bTmp->next;
    }

    printf("\nEnter Book ISBN to Link: ");
    fgets(isbn, sizeof(isbn), stdin);
    isbn[strcspn(isbn, "\n")] = 0;

    printf("\n--- Available Authors ---\n");
    listAuthors(aHead); 

    printf("\nEnter Author ID: ");
    scanf("%d", &authorId);
    while(getchar()!='\n'); 

    if (addBookAuthorRelation(map, count, isbn, authorId)) {
        printf("Success: Book linked to Author.\n");
        saveBookAuthorMapToFile(*map, *count); 
    } else {
        printf("Error: Relation already exists or memory error.\n");
    }
}

void menuBooks(Book** head, Author* aHead, BookAuthorMap** map, int* count) {
    int choice;
    do {
        printf("\n--- Book Menu ---\n");
        printf("1. Add Book\n");
        printf("2. Delete Book\n");
        printf("3. List Books\n");
        printf("4. Assign Author to Book\n"); 
        printf("0. Back\n");
        printf("Choice: ");
        scanf("%d", &choice); 
        while(getchar()!='\n');

        switch(choice) {
            case 1: {
                char t[50], i[14]; int q;
                printf("Title: "); fgets(t,50,stdin); t[strcspn(t,"\n")]=0;
                printf("ISBN: "); fgets(i,14,stdin); i[strcspn(i,"\n")]=0;
                printf("Quantity: "); scanf("%d", &q);
                Book* n = NULL;
                *head = addBook(*head, t, i, q, &n);
                saveBooksToFile(*head, FILE_BOOKS);
                if(n) saveBookCopiesToFile(*head, FILE_COPIES);
                break;
            }
            case 2: {
                char i[14]; printf("ISBN: "); fgets(i,14,stdin); i[strcspn(i,"\n")]=0;
                deleteBook(head, i);
                saveBooksToFile(*head, FILE_BOOKS);
                saveBookCopiesToFile(*head, FILE_COPIES);
                break;
            }
            case 3: {
                Book* tmp = *head;
                while(tmp) {
                    printf("%s (ISBN: %s) Qty: %d\n", tmp->title, tmp->isbn, tmp->quantity);
                    tmp = tmp->next;
                }
                break;
            }
            case 4: {
                // YENİ EKLENEN EŞLEŞTİRME MENÜSÜ ÇAĞRISI
                menuLinkBookAuthor(*head, aHead, map, count);
                break;
            }
        }
    } while(choice!=0);
}

// --- MEMORY CLEANUP ---
void freeBookCopies(BookCopy* head) {
    BookCopy* tmp;
    while (head) { tmp = head; head = head->next; free(tmp); }
}
void freeBookList(Book* head) {
    Book* tmp;
    while (head) { freeBookCopies(head->copies); tmp = head; head = head->next; free(tmp); }
}
void freeAuthorList(Author* head) {
    Author* tmp;
    while(head){ tmp=head; head=head->next; free(tmp); }
}
void freeStudentList(Student* head) {
    Student* tmp;
    while(head){ tmp=head; head=head->next; free(tmp); }
}
void freeLoanList(LoanTransaction* head) {
    LoanTransaction* tmp;
    while(head){ tmp=head; head=head->next; free(tmp); }
}

int main() {
    int lastID = 0;
    Author* authors = loadAuthorsFromFile(&lastID);
    Student* students = loadStudentsFromFile();
    // Load books and then load copies into them
    Book* books = loadBooksFromFile(FILE_BOOKS, FILE_COPIES);
    loadBookCopiesFromFile(books, FILE_COPIES);
    
    LoanTransaction* loans = loadLoansFromFile();
    
    int mapCount = 0;
    BookAuthorMap* mapArr = loadBookAuthorMap(&mapCount);

    int choice;
    do {
        printf("\n=== Library Automation System ===\n");
        printf("1. Author Ops\n2. Student Ops\n3. Book Ops\n0. Exit\nSelect: ");
        scanf("%d", &choice); while(getchar()!='\n');
        
        switch(choice) {
            case 1: menuAuthors(&authors, &mapArr, &mapCount); break;
            case 2: menuStudents(&students, &books, &loans); break;
            case 3: menuBooks(&books, authors, &mapArr, &mapCount); break;
            case 0: printf("Exiting...\n"); break;
        }
    } while (choice != 0);

    // Save final state
    saveAuthorsToFile(authors, FILE_AUTHORS);
    saveStudentsToFile(students, FILE_STUDENTS);
    saveBooksToFile(books, FILE_BOOKS);
    saveBookCopiesToFile(books, FILE_COPIES);
    saveLoansToFile(loans);
    saveBookAuthorMapToFile(mapArr, mapCount);

    // Cleanup
    freeAuthorList(authors);
    freeStudentList(students);
    freeBookList(books);
    freeLoanList(loans);
    if(mapArr) free(mapArr);

    return 0;
}