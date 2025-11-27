#include <stdio.h>
#include <stdlib.h>
#include "headers/cms.h"
#include "headers/input_validation.h"


int main() {
    printDeclaration();

    char *input = NULL; // User input buffer
    char *valid;        // Pointer to validated command substring

    int newID, deleteID, queryID;
    char newName[MAX_NAME], newProgramme[MAX_PROGRAMME];
    float newMark;

    while (1) {
        printf("\n\nP4_1: ");
        
        input = readLine();
        
        if (input == NULL) {
            printf("CMS: Fatal error reading input.\n");
            break; 
        }
        
        // Process OPEN command
        if ((valid = validateCommand(input, "OPEN")) != NULL) {
            if (*valid != '\0') {
                printf("CMS: Enter a valid command.\n");
            } 
            else {
                openDB();
            }
        }
        
        // Process SHOW commands
        else if ((valid = validateCommand(input, "SHOW")) != NULL) {
            if (!dbLoaded) {
                printf("CMS: No records loaded. Open and load the database first.\n");
            } 
            else {
                handleShow(valid);
            }
        }
        
        // Process INSERT command
        else if ((valid = validateCommand(input, "INSERT")) != NULL) {
            if (!dbLoaded) {
                printf("CMS: No records loaded. Open and load the database first.\n");
            } else {
                
                if (parseCommand(valid, &newID, newName, newProgramme, &newMark, OPTIONAL_ALLOWED_EMPTY)) {
                    insertDB(newID, newName, newProgramme, newMark, 0);
                }
            }
        }

        // Process UPDATE command
        else if ((valid = validateCommand(input, "UPDATE")) != NULL) {
            if (!dbLoaded) {
                printf("CMS: No records loaded. Open and load the database first.\n");
                free(input);
                input = NULL;
                continue;
            }
            if (parseCommand(valid, &newID, newName, newProgramme, &newMark, OPTIONAL_REQUIRED)) {
                updateDB(newID, newName, newProgramme, newMark, 0);
            }
        }

        // Process DELETE command with confirmation
        else if ((valid = validateCommand(input, "DELETE")) != NULL) {
            if (!dbLoaded) {
                printf("CMS: No records loaded. Open and load the database first.\n");
            } 
            else if (parseCommand(valid, &deleteID, NULL, NULL, NULL, OPTIONAL_NONE)) {
                if (!deleteDB(deleteID, 0, 0)) {
                    printf("CMS: The record with ID=%d does not exist.\n", deleteID);
                } 
                else {
                    printf("CMS: Are you sure you want to delete record with ID=%d? Type \"Y\" to confirm or \"N\" to cancel.\n", deleteID);
                    printf("\nP4_1: ");
                    
                    char *confirm_input = readLine(); 

                    if (confirm_input == NULL) {
                        printf("CMS: Fatal error reading confirmation input. The deletion is cancelled.\n");
                    } 
                    else {
                        if ((valid = validateCommand(confirm_input, "Y")) != NULL && *valid == '\0') {
                            deleteDB(deleteID, 1, 0);  
                        } 
                        else if ((valid = validateCommand(confirm_input, "N")) != NULL && *valid == '\0') {
                            printf("CMS: The deletion is cancelled.\n");  
                        } 
                        else {
                            printf("CMS: Invalid input. The deletion is cancelled.\n");
                        }
                        
                        free(confirm_input);
                    }
                }
            }
        }
        
        // Process QUERY command
        else if ((valid = validateCommand(input, "QUERY")) != NULL) {
            if (!dbLoaded) {
                printf("CMS: No records loaded. Open and load the database first.\n");
            } 
            else if (parseCommand(valid, &queryID, NULL, NULL, NULL, OPTIONAL_NONE)) {
                queryDB(queryID);
            }
        }

        // Process UNDO command
        else if ((valid = validateCommand(input, "UNDO")) != NULL) {
            if (*valid != '\0') {
                printf("CMS: Enter a valid command.\n");
            } 
            else if (!dbLoaded) {
                printf("CMS: No records loaded. Open and load the database first.\n");
            } 
            else {
                undo();
            } 
        } 

        // Process REDO command
        else if ((valid = validateCommand(input, "REDO")) != NULL) {
            if (*valid != '\0') {
                printf("CMS: Enter a valid command.\n");
            } 
            else if (!dbLoaded) {
                printf("CMS: No records loaded. Open and load the database first.\n");
            } 
            else {
                redo();
            } 
        }

        // Process SAVE command
        else if ((valid = validateCommand(input, "SAVE")) != NULL) {
            if (*valid != '\0') {
                printf("CMS: Enter a valid command.\n");
            } 
            else if (!dbLoaded) {
                printf("CMS: No records loaded. Open and load the database first.\n");
            } 
            else {
                saveDB();
            }
        }

        // Process RESTORE command with confirmation
        else if ((valid = validateCommand(input, "RESTORE")) != NULL) {
            if (*valid != '\0') {
                printf("CMS: Enter a valid command.\n");
            } 
            if (!dbLoaded) {
                printf("CMS: No records loaded. Open the database first.\n");
            } 
            else {
                printf("CMS: WARNING: This will overwrite the current in-memory state with the backup file. Are you sure? Type \"Y\" to confirm or \"N\" to cancel.\n");
                printf("\nP4_1: ");

                char *confirm_input = readLine();

                if (confirm_input == NULL) {
                    printf("CMS: Fatal error reading confirmation input. Restore cancelled.\n");
                } 
                else {
                    if ((valid = validateCommand(confirm_input, "Y")) != NULL && *valid == '\0') {
                        restoreDB(0); 
                    } 
                    else if ((valid = validateCommand(confirm_input, "N")) != NULL && *valid == '\0') {
                        printf("CMS: Restore operation cancelled.\n");
                    } 
                    else {
                        printf("CMS: Invalid input. Restore operation cancelled.\n");
                    }
                    free(confirm_input);
                }
            }
        }        

        // Process QUIT command with confirmation if unsaved changes exist
        else if ((valid = validateCommand(input, "QUIT")) != NULL) {
            if (*valid != '\0') {
                printf("CMS: Enter a valid command (QUIT takes no arguments).\n");
            } 
            else {
                if (dbLoaded && dbModified) {
                    printf("CMS: WARNING: You have unsaved changes. Are you sure you want to quit? Type \"Y\" to confirm or \"N\" to cancel.\n");
                } 
                else {
                    printf("CMS: Are you sure you want to quit? There are no unsaved changes. Type \"Y\" to confirm or \"N\" to cancel.\n");
                }

                printf("\nP4_1: ");
                char *confirm_input = readLine();

                if (confirm_input == NULL) {
                    printf("CMS: Fatal error reading confirmation input. Quit cancelled.\n");
                }
                else {
                    if ((valid = validateCommand(confirm_input, "Y")) != NULL && *valid == '\0') {
                        free(confirm_input);
                        free(input);
                        input = NULL;
                        break; 
                    }
                    else if ((valid = validateCommand(confirm_input, "N")) != NULL && *valid == '\0') {
                        printf("CMS: Quit operation cancelled.\n");
                    }
                    else {
                        printf("CMS: Invalid input. Quit operation cancelled.\n");
                    }
                    free(confirm_input);
                }
            }
        }

        else {
            printf("CMS: Enter a valid command\n");
        }

        free(input); // Free buffer before next iteration
        input = NULL;
    }

    freeDB(); // Free all database memory
    printf("CMS: Exiting program.\n");
    return 0;
}