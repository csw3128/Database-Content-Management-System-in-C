/**
 * Student Course Management System (CMS)
 * --------------------------------------
 * This program maintains a student record database using:
 * - A linked list for sequential storage
 * - A hash table for fast ID-based lookup
 * - Undo/redo stack to support reversible operations
 *
 * Features include inserting, updating, deleting, searching, sorting,
 * file loading/saving, summary statistics, and restore functions.
 *
 * Authors: Team P4-1
 * Date: 25/11/2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "headers/cms.h"


// ===============================
// Global Variables
// ===============================

// File pointer for database file access
FILE *fptr = NULL;

// Linked list head and tail storing student records
Node *head = NULL;
Node *tail = NULL;

// Hash table structure for constant-time search by student ID
Node *hashTable[TABLE_SIZE] = {0};

// Flags to track database state
int dbModified = 0;  // Tracks unsaved changes
int dbLoaded = 0;    // Tracks whether a DB file has been loaded

// Undo/redo stack pointers
Action *undoStack = NULL;
Action *redoStack = NULL;

// Temporary variables for operations
int id; 
char name[MAX_NAME], programme[MAX_PROGRAMME]; 
float mark;


// ===============================
// Hash Table Utility Functions
// ===============================


/**
 * hash()
 * -----------------------------------------
 * Computes a hash index based on student ID.
 * 
 * @param id - student ID number
 * @return integer index within hash table bounds
 */
int hash(int id) {
    return id % TABLE_SIZE;
}


/**
 * hashInsert()
 * -----------------------------------------
 * Inserts a node into the hash table for fast access by ID.
 *
 * @param node - pointer to the record being inserted
 */
void hashInsert(Node *node) {
    int index = hash(node->data.id);
    // Insert at the beginning of the chain (chaining collision resolution)
    node->hashNext = hashTable[index];
    hashTable[index] = node;
}


/**
 * hashRemove()
 * -----------------------------------------
 * Removes a record from the hash table by ID.
 *
 * @param id - ID of the record to remove
 */
void hashRemove(int id) {
    int index = hash(id);
    Node *cur = hashTable[index], *prev = NULL;

    while (cur) {
        if (cur->data.id == id) {
            // Fix linking depending on position
            if (prev) prev->next = cur->next;
            else hashTable[index] = cur->next;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}


/**
 * findNode()
 * -----------------------------------------
 * Searches for a student record by ID using the hash table.
 *
 * @param id - student ID to search for
 * @return pointer to the matching record or NULL if not found
 */
Node* findNode(int id) {
    int index = hash(id);
    Node *cur = hashTable[index];
    while (cur) {
        if (cur->data.id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}


void printDeclaration() {
    printf("\t\t\t\t\t\t\tDeclaration\t\t\t\t\t\t\n");
    printf("SIT's policy on copying does not allow the students to copy source code as well as assessment solutions\n");
    printf("from another person, AI, or other places. It is the students' responsibility to guarantee that their\n");
    printf("assessment solutions are their own work. Meanwhile, the students must also ensure that their work is\n");
    printf("not accessible by others. Where such plagiarism is detected, both of the assessments involved will\n");
    printf("receive ZERO mark.\n\n");

    printf("We hereby declare that:\n");
    printf("We fully understand and agree to the abovementioned plagiarism policy.\n");
    printf("We did not copy any code from others or from other places.\n");
    printf("We did not share our codes with others or upload to any other places for public access and will not do that in the future.\n");
    printf("We agree that our project will receive Zero mark if there is any plagiarism detected.\n");
    printf("We agree that we will not disclose any information or material of the group project to others or upload to any other places for public access.\n");
    printf("We agree that we did not copy any code directly from AI generated sources.\n\n");

    printf("Declared by: P4-1\n");
    printf("Team members:\n");
    printf("\t1. Chew Shu Wen\n");
    printf("\t2. Adora Goh Shao Qi \n");
    printf("\t3. Calson See Jia Jun\n");
    printf("\t4. Au Myat Yupar Aung\n");
    printf("\t5. Chung Kai Sheng Desmond\n");
    printf("Date: 25/11/2025\n");
}


// Push a new action to the undo stack and clear redo stack
void pushUndo(Action *action) {
    action->next = undoStack; // Attach action to top of undo stack
    undoStack = action;

    // User made a new change → redo stack becomes invalid → clear redo history
    while (redoStack) {
        Action *tmp = redoStack;
        redoStack = redoStack->next;
        free(tmp);
    }
}


// Undo the most recent action performed by the user
void undo() {
    if (!undoStack) { // Nothing to revert
        printf("CMS: Nothing to undo.\n");
        return;
    }

    Action *action = undoStack;     // Take latest recorded action
    undoStack = undoStack->next;    // Pop from undo stack

    // Reverse user action depending on type
    switch (action->type) {
        case INSERT_OP:   // Undo insert → delete record
            deleteDB(action->newData.id, 1, 1);
            printf("CMS: UNDO -> Undid INSERT (ID %d).\n", action->newData.id);
            break;
        case UPDATE_OP:   // Undo update → restore previous values
            updateDB(action->oldData.id,
                     action->oldData.name,
                     action->oldData.programme,
                     action->oldData.mark,
                     1);
            printf("CMS: UNDO -> Undid UPDATE on (ID %d).\n", action->newData.id);
            break;
        case DELETE_OP:   // Undo delete → re-insert deleted record
            insertDB(action->oldData.id,
                     action->oldData.name,
                     action->oldData.programme,
                     action->oldData.mark,
                     1);
            printf("CMS: UNDO -> Undid DELETE (ID %d).\n", action->oldData.id);
            break;
        case RESTORE_OP:  // Undo restore → reload previous version
            loadDB("./data/P4_1-CMS.txt");
            printf("CMS: UNDO -> Undid RESTORE operation.\n");
            break;            
    }
    // Move undone action to redo stack
    action->next = redoStack;
    redoStack = action;
}


// Redo the last undone action
void redo() {
    if (!redoStack) { // Nothing available to redo
        printf("CMS: Nothing to redo.\n");
        return;
    }

    Action *action = redoStack;    // Take most recent redo item
    redoStack = redoStack->next;

    switch (action->type) {
        case INSERT_OP:
            insertDB(action->newData.id,
                     action->newData.name,
                     action->newData.programme,
                     action->newData.mark,
                     1);
            printf("CMS: REDO -> Redid INSERT (ID %d).\n", action->newData.id);
            break;
        case UPDATE_OP:
            updateDB(action->newData.id,
                     action->newData.name,
                     action->newData.programme,
                     action->newData.mark,
                     1);
            printf("CMS: REDO -> Redid UPDATE on (ID %d).\n", action->newData.id);
            break;
        case DELETE_OP:
            deleteDB(action->oldData.id, 1, 1);
            printf("CMS: REDO -> Redid DELETE (ID %d).\n", action->oldData.id);
            break;
        case RESTORE_OP:
            restoreDB(1); 
            printf("CMS: REDO -> Redid RESTORE operation.\n");
            break;            
    }
    // Return action back to undo stack
    action->next = undoStack;
    undoStack = action;
}


// Open and load main dataset file only once
void openDB() {
    if (dbLoaded) {
        printf("CMS: The database file \"P4_1-CMS.txt\" has already been opened.\n");
        return;
    }

    loadDB("./data/P4_1-CMS.txt");
}


// Load database from file into memory and hash table
void loadDB(const char *filename) {
    freeDB();  // Clear old memory before loading new data

    // Reset hash table
    for (int i = 0; i < TABLE_SIZE; i++) {
        hashTable[i] = NULL;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("CMS: Could not open file \"%s\".\n", filename);
        dbLoaded = 0;
        return;
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;

        // Skip header lines
        if (strstr(line, "Database Name: P4_1-CMS") ||
            strstr(line, "Authors: P4-1") ||
            strstr(line, "Table Name: StudentRecords") ||
            strstr(line, "ID\tName\tProgramme\tMark") ||
            strlen(line) == 0) {
            continue;
        }

        // Split values on TAB delimiter
        char *ptr = line;
        char *fields[4] = {NULL, NULL, NULL, NULL};
        for (int i = 0; i < 4; i++) {
            fields[i] = ptr;
            char *tabPos = strchr(ptr, '\t');
            if (tabPos) {
                *tabPos = '\0';
                ptr = tabPos + 1;
            } else {
                ptr += strlen(ptr);
            }
        }      

        // Create new record node in memory
        Node *newNode = malloc(sizeof(Node));
        if (!newNode) {
            printf("CMS: Memory allocation failed during load.\n");
            fclose(file);
            freeDB();
            return;
        }

         // Assign parsed values
        newNode->data.id = fields[0] && strlen(fields[0]) > 0 ? atoi(fields[0]) : 0;
        strcpy(newNode->data.name, fields[1] ? fields[1] : "");
        strcpy(newNode->data.programme, fields[2] ? fields[2] : "");
        newNode->data.mark = fields[3] && strlen(fields[3]) > 0 ? atof(fields[3]) : 0.0f;
        newNode->next = NULL;
        newNode->hashNext = NULL;

        // Append into linked list
        if (!head) head = tail = newNode;
        else {
            tail->next = newNode;
            tail = newNode;
        }
        hashInsert(newNode);  // Store in hash table
    }
    fclose(file);
    dbLoaded = 1;

    printf("CMS: The database file \"P4_1-CMS.txt\" is successfully opened.\n");
}


// Print records in formatted table layout (supports multi-line wrapping)
void printNodeList(Node* list, const char *headerMsg) {
    if (!list) {
        printf("CMS: No records to display.\n");
        return;
    }

    // Discover longest field values for dynamic alignment
    int max_name_len = 4;
    int max_prog_len = 9;
    Node* temp = list;
    while (temp) {
        int nameLen = strlen(temp->data.name);
        int progLen = strlen(temp->data.programme);
        if (nameLen > max_name_len) max_name_len = nameLen;
        if (progLen > max_prog_len) max_prog_len = progLen;
        temp = temp->next;
    }

    // Set final display width limits
    int display_name_width = max_name_len + 2;
    int display_prog_width = max_prog_len + 2;

    if (display_name_width > NAME_WIDTH + 2) display_name_width = NAME_WIDTH + 2;
    if (display_prog_width > PROG_WIDTH + 2) display_prog_width = PROG_WIDTH + 2;
    if (display_name_width < 6) display_name_width = 6;
    if (display_prog_width < 11) display_prog_width = 11;

    // Print table header
    printf("%s\n", headerMsg);
    printf("%-8s %-*s %-*s %-5s\n",
           "ID",
           display_name_width, "Name",
           display_prog_width, "Programme",
           "Mark");

    // Output each record (multi-line wrapping supported)
    Node* curr = list;
    while (curr) {
        int nameLen = strlen(curr->data.name);
        int progLen = strlen(curr->data.programme);
        int lines_name = (nameLen > 0) ? (nameLen + NAME_WIDTH - 1) / NAME_WIDTH : 1;
        int lines_prog = (progLen > 0) ? (progLen + PROG_WIDTH - 1) / PROG_WIDTH : 1;
        int lines = (lines_name > lines_prog) ? lines_name : lines_prog;

        for (int i = 0; i < lines; i++) {
            if (i == 0) printf("%-8d ", curr->data.id);
            else printf("%-8s ", "");

            if (i * NAME_WIDTH < nameLen)
                printf("%-*.*s ", display_name_width, NAME_WIDTH, curr->data.name + i * NAME_WIDTH);
            else
                printf("%-*s ", display_name_width, "");

            if (i * PROG_WIDTH < progLen)
                printf("%-*.*s ", display_prog_width, PROG_WIDTH, curr->data.programme + i * PROG_WIDTH);
            else
                printf("%-*s ", display_prog_width, "");

            if (i == 0) printf("%.1f", curr->data.mark);
            printf("\n");
        }
        curr = curr->next;
    }
}


void showDB() {
    printNodeList(head, "CMS: Here are all the records found in the table \"StudentRecords\".");
}


// Create an independent duplicate of the linked list (used before sorting)
Node* cloneList(Node* original) {
    if (!original) return NULL;

    Node* newHead = NULL;
    Node* tail = NULL;

    while (original) {
        Node* newNode = malloc(sizeof(Node));
        newNode->data = original->data;
        newNode->next = NULL;

        if (!newHead)
            newHead = tail = newNode;
        else {
            tail->next = newNode;
            tail = newNode;
        }

        original = original->next;
    }

    return newHead;
}


// Merge two sorted lists based on ID or mark, ascending or descending
static Node* mergeSorted(Node* list1, Node* list2, int sortByID, int ascending) {
    if (!list1) return list2;
    if (!list2) return list1;

    Node* mergedHead = NULL;

    int compare = 0;
    if (sortByID) {
        compare = list1->data.id - list2->data.id;
    } 
    else {
        if (list1->data.mark > list2->data.mark) compare = 1;
        else if (list1->data.mark < list2->data.mark) compare = -1;
        else compare = 0;
    }

    if ((ascending && compare <= 0) || (!ascending && compare > 0)) {
        mergedHead = list1;
        mergedHead->next = mergeSorted(list1->next, list2, sortByID, ascending);
    } 
    else {
        mergedHead = list2;
        mergedHead->next = mergeSorted(list1, list2->next, sortByID, ascending);
    }
    return mergedHead;
}


// Merge sort implementation optimized for linked list structure
static Node* mergeSort(Node* head, int sortByID, int ascending) {
    if (!head || !head->next) return head;

    Node* slow = head;
    Node* fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    Node* middle = slow->next;
    slow->next = NULL;

    Node* leftSorted = mergeSort(head, sortByID, ascending);
    Node* rightSorted = mergeSort(middle, sortByID, ascending);

    return mergeSorted(leftSorted, rightSorted, sortByID, ascending);
}


// Show all records sorted by ID or mark
void showDBSorted(int sortByID, int ascending) {
    if (!head) {
        printf("CMS: No records to display.\n");
        return;
    }

    Node* tempList = cloneList(head);
    Node* sortedHead = mergeSort(tempList, sortByID, ascending);

    char headerMsg[150];
    if (sortByID) {
        snprintf(headerMsg, sizeof(headerMsg),
                 "CMS: Here are all the records sorted by ID %s from the table \"StudentRecords\".",
                 ascending ? "ASC" : "DESC");
    } else {
        snprintf(headerMsg, sizeof(headerMsg),
                 "CMS: Here are all the records sorted by mark %s from the table \"StudentRecords\".",
                 ascending ? "ASC" : "DESC");
    }

    printNodeList(sortedHead, headerMsg);

    while (sortedHead) {
        Node* temp = sortedHead;
        sortedHead = sortedHead->next;
        free(temp);
    }
}


// Compute and display statistics such as: total count,
// average score, highest and lowest marks (with names)
void showSummary(const char *programmeFilter) {
    if (!head) {
        printf("CMS: No records to display.\n");
        return;
    }

    int total = 0;
    float sum = 0.0f;
    float maxMark = -1.0f, minMark = 101.0f;
    struct Node *current = head;

    while (current) {
        if (!programmeFilter || strcasecmp(current->data.programme, programmeFilter) == 0) {
            float mark = current->data.mark;
            if (mark > maxMark) maxMark = mark;
            if (mark < minMark) minMark = mark;
            sum += mark;
            total++;
        }
        current = current->next;
    }

    if (total == 0) {
        if (programmeFilter)
            printf("CMS: No matching records found for programme '%s'.\n", programmeFilter);
        else
            printf("CMS: No records found.\n");
        return;
    }

    printf("CMS: Here are summary statistics from the table \"StudentRecords\"");
    if (programmeFilter) printf(" (Programme: %s)", programmeFilter);
    printf(".\n");

    printf("Total students: %d\n", total);

    printf("Average mark: %.2f\n", sum / total);

    // Print highest mark students
    printf("\nHighest mark: %.1f\n", maxMark);
    int count = 1;
    current = head;
    while (current) {
        if ((!programmeFilter || strcasecmp(current->data.programme, programmeFilter) == 0)
            && current->data.mark == maxMark) {
            printf("%d. %s (ID: %d)\n", count++, current->data.name, current->data.id);
        }
        current = current->next;
    }
    
    // Print lowest mark students
    printf("\nLowest mark: %.1f\n", minMark);
    count = 1;
    current = head;
    while (current) {
        if ((!programmeFilter || strcasecmp(current->data.programme, programmeFilter) == 0)
            && current->data.mark == minMark) {
            printf("%d. %s (ID: %d)\n", count++, current->data.name, current->data.id);
        }
        current = current->next;
    }
}


// Inserts a new student record into both the linked list and hash table.
// If the operation is user-initiated (not from undo/redo), it is recorded
// for reversal and user feedback is displayed.
void insertDB(int newID, char *newName, char *newProgramme, float newMark, int isUndoRedo) {
    // Prevent duplicate records
    if (findNode(newID)) {
        if (!isUndoRedo) printf("CMS: Record with ID=%d already exists.\n", newID);
        return;
    }

    // Allocate space for a new node
    Node *newNode = malloc(sizeof(Node)); // Allocate memory for new node
    if (!newNode) {
        printf("CMS: Memory allocation failed.\n");
        return;
    }

    // Populate record fields
    newNode->data.id = newID;
    strcpy(newNode->data.name, newName ? newName : "");
    strcpy(newNode->data.programme, newProgramme ? newProgramme : "");
    newNode->data.mark = newMark;
    newNode->next = NULL;
    newNode->hashNext = NULL;

    // Append to linked list (tail insertion)
    if (!head) head = tail = newNode;
    else {
        tail->next = newNode;
        tail = newNode;
    }

    // Add record to hash table
    hashInsert(newNode);

    // Record action for undo stack 
    if (!isUndoRedo) {
        Action *action = malloc(sizeof(Action));
        action->type = INSERT_OP;
        action->newData = newNode->data;
        pushUndo(action);
        printf("CMS: Record with ID=%d inserted.\n", newID);
    }

    dbModified = 1;
}


// Searches for a record by ID and prints it in formatted table output.
// A temporary single-node list is used for reuse of print formatting logic.
void queryDB(int id) {
    Node *node = findNode(id);
    if (!node) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return;
    }

    // Create a temporary wrapper so printNodeList() can format properly
    Node tempNode;
    tempNode.data = node->data;
    tempNode.next = NULL;

    printf("CMS: The record with ID=%d is found in the data table.\n", id);
    printNodeList(&tempNode, "");
}


// Modifies an existing record (name, programme, or mark). Changes are tracked
// so undo/redo can revert modifications when needed.
void updateDB(int id, char *name, char *programme, float mark, int isUndoRedo) {
    Node *record = findNode(id);
    if (!record) {
        if (!isUndoRedo) printf("CMS: The record with ID=%d does not exist.\n", id);
        return;
    }

    Action *action = NULL;
    // Prepare undo entry only if initiated by user    
    if (!isUndoRedo) {
        action = malloc(sizeof(Action));
        action->type = UPDATE_OP;
        action->oldData = record->data;
        action->newData = record->data;
    }

    // Apply updates selectively
    if (name && strlen(name) > 0) strcpy(record->data.name, name);
    if (programme && strlen(programme) > 0) strcpy(record->data.programme, programme);
    if (mark >= 0.0f) record->data.mark = mark;  

    // Finalize undo stack if applicable
    if (action) {
        action->newData = record->data;
        pushUndo(action);
        if (!isUndoRedo) printf("CMS: The record with ID=%d is successfully updated.\n", id);
    }

    dbModified = 1;
}


// Removes a record from both the linked list and hash table.
// If user-triggered, the action is stored so it can be undone.
int deleteDB(int id, int confirm, int isUndoRedo) {
    Node *current = head;
    Node *prev = NULL;

    // Find node to delete
    while (current && current->data.id != id) {
        prev = current;
        current = current->next;
    }

    if (!current) return 0; // Not found
    if (!confirm) return 1; // Preview mode (for DELETE confirmation)

    if (!isUndoRedo) {
        Action *action = malloc(sizeof(Action));
        action->type = DELETE_OP;
        action->oldData = current->data;
        pushUndo(action);
    }

    // Remove from linked list
    if (!prev) head = current->next;
    else prev->next = current->next;

    if (current == tail) tail = prev;

    // Remove from hash table & free memory
    hashRemove(id);
    free(current);

    if (!isUndoRedo) printf("CMS: The record with ID=%d is successfully deleted.\n", id);

    dbModified = 1;
    return 1;
}


// Saves the in-memory database to a file, but only if changes exist.
// Prevents unnecessary overwrites and preserves backup integrity.
void saveDB() {
    if (!dbLoaded) {
        printf("CMS: No database loaded. Nothing to save.\n");
        return;
    }

    // Read current file contents dynamically
    FILE *original = fopen("./data/P4_1-CMS.txt", "r");
    char *fileContent = NULL;
    long fileLen = 0;

    if (original) {
        int ch;
        while ((ch = fgetc(original)) != EOF) {
            char *newPtr = realloc(fileContent, fileLen + 2);
            if (!newPtr) {
                free(fileContent);
                fclose(original);
                printf("CMS: Memory allocation failed during file comparison.\n");
                return;
            }
            fileContent = newPtr;
            fileContent[fileLen++] = ch;
            fileContent[fileLen] = '\0';
        }
        fclose(original);
    }

    // Build current memory representation dynamically
    char *memContent = NULL;
    long memLen = 0;

    // Add headers exactly as written during save
    const char *header =
        "Database Name: P4_1-CMS\n"
        "Authors: P4-1\n\n"
        "Table Name: StudentRecords\n"
        "ID\tName\tProgramme\tMark\n";

    for (const char *p = header; *p; p++) {
        char *newPtr = realloc(memContent, memLen + 2);
        if (!newPtr) {
            free(fileContent);
            free(memContent);
            printf("CMS: Memory allocation failed while preparing save buffer.\n");
            return;
        }
        memContent = newPtr;
        memContent[memLen++] = *p;
        memContent[memLen] = '\0';
    }

    // Append all records from linked list
    Node *current = head;
    char line[256];

    while (current) {
        snprintf(line, sizeof(line), "%d\t%s\t%s\t%.1f\n",
                 current->data.id,
                 current->data.name,
                 current->data.programme,
                 current->data.mark);

        for (char *p = line; *p; p++) {
            char *newPtr = realloc(memContent, memLen + 2);
            if (!newPtr) {
                free(fileContent);
                free(memContent);
                printf("CMS: Memory allocation failed while writing memory comparison.\n");
                return;
            }
            memContent = newPtr;
            memContent[memLen++] = *p;
            memContent[memLen] = '\0';
        }
        current = current->next;
    }

    // Compare memory and file contents
    if (fileContent && strcmp(fileContent, memContent) == 0) {
        printf("CMS: No changes detected. Nothing to save.\n");
        free(fileContent);
        free(memContent);
        return;
    }

    // Proceed with actual saving
    FILE *backup = fopen("./data/P4_1-CMS.bak", "w");
    if (backup && fileContent) {
        fputs(fileContent, backup);
        fclose(backup);
    }

    FILE *file = fopen("./data/P4_1-CMS.txt", "w");
    if (!file) {
        printf("CMS: Error saving the database file.\n");
        free(fileContent);
        free(memContent);
        return;
    }

    fputs(memContent, file);
    fclose(file);

    free(fileContent);
    free(memContent);

    dbModified = 0;
    printf("CMS: The database file \"P4_1-CMS.txt\" has been successfully saved.\n");
}


// Loads the backup file into memory, replacing the current dataset.
// The action is recorded unless triggered by undo/redo logic.
void restoreDB(int isUndoRedo) {
    FILE *backup = fopen("./data/P4_1-CMS.bak", "r");
    if (!backup) {
        printf("CMS: Backup file \"P4_1-CMS.bak\" does not exist. Cannot restore.\n");
        return;
    }
    fclose(backup); 

    if (!isUndoRedo) {
        Action *action = malloc(sizeof(Action));
        if (!action) {
            printf("CMS: Memory allocation failed for RESTORE action.\n");
            return;
        }

        action->type = RESTORE_OP;
        pushUndo(action);
    }
    
    loadDB("./data/P4_1-CMS.bak"); // Load backup into memory
    
    if (!isUndoRedo) {
        printf("CMS: Database successfully restored from backup. Changes are not saved yet.\n");
    }

    dbModified = 1; 
}


// Releases all dynamically allocated nodes from memory.
// Called when loading a new DB or exiting the program.
void freeDB() {
    struct Node *current = head; 
    while (current != NULL) {
        struct Node *nextNode = current->next; 
        free(current); 
        current = nextNode; 
    }
    head = NULL; 
}