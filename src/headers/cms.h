#ifndef CMS_H
#define CMS_H

// =========================
// System Configuration Limits
// =========================
#define MAX_LINE 256            // Maximum length for input line parsing
#define MAX_NAME 100            // Maximum character size for student name field
#define MAX_PROGRAMME 100       // Maximum character size for programme field
#define TABLE_SIZE 2003         // Hash table size (prime number improves distribution)

// =========================
// Display Formatting
// =========================
#define NAME_WIDTH 35           // Fixed width when printing names in table format
#define PROG_WIDTH 35           // Fixed width when printing programme values

// =========================
// Data Structures
// =========================

// Structure representing student data in the system
typedef struct StudentRecord {
    int id;
    char name[MAX_NAME];
    char programme[MAX_PROGRAMME];
    float mark;
} StudentRecord;

// Linked list + hash table node used for storage, lookup, and traversal
typedef struct Node {
    StudentRecord data;
    struct Node *next;          // Linked list pointer
    struct Node *hashNext;      // Hash table chaining pointer
} Node;

// Enumeration to classify operation types for undo/redo tracking
typedef enum {
    INSERT_OP,
    UPDATE_OP,
    DELETE_OP,
    RESTORE_OP 
} ActionType;

// Action structure stored in the undo/redo stacks to revert/reenact changes
typedef struct Action {
    ActionType type;
    StudentRecord oldData;
    StudentRecord newData;
    struct Action *next;
} Action;

// =========================
// Global Variables
// =========================
extern FILE *fptr;                     // File pointer used for reading/writing database files
extern Node *head;                     // Head of student linked list
extern Node *tail;                     // Tail pointer for fast insertions
extern Node *hashTable[TABLE_SIZE];    // Hash table for fast student lookup
extern int dbModified;                 // Flag indicating whether unsaved changes exist
extern int dbLoaded;                   // Flag to ensure certain actions only happen after loading a DB

extern Action *undoStack;              // Stack storing actions for undo functionality
extern Action *redoStack;              // Stack storing reversed actions for redo functionality

// =========================
// System Function Prototypes
// =========================

// Prints banner / program introduction
void printDeclaration();

// Undo/Redo management
void pushUndo(Action *action);
void undo();
void redo();

// Database file handling
void openDB();
void loadDB(const char *filename);
void showDB();

// Display functions
void showDBSorted(int sortByID, int ascending);
void showSummary(const char *programmeFilter);

// Core CRUD operations
void insertDB(int newID, char *newName, char *newProgramme, float newMark, int isUndoRedo);
void queryDB(int id);
void updateDB(int id, char *name, char *programme, float mark, int isUndoRedo);
int deleteDB(int id, int confirm, int isUndoRedo);

// Save/Restore data operations
void saveDB();
void restoreDB(int isUndoRedo);
void freeDB();

#endif