#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include "headers/input_validation.h"
#include "headers/cms.h"



/* -------------------------------------------------------------------------
   readLine()
   Reads an entire line from stdin safely, without fixed-size limits.
   - Automatically expands buffer size as input grows.
   - Stops reading on newline or EOF.
   - Caller is responsible for freeing the returned string.
   Returns: dynamically allocated null-terminated string OR NULL on failure.
---------------------------------------------------------------------------*/
char* readLine() {
    int buffer_size = 128;
    char *buffer = (char*)malloc(buffer_size * sizeof(char));
    if (!buffer) {
        perror("Memory allocation failed");
        return NULL;
    }
    
    int position = 0;
    int c;
    
    while (1) {
        c = fgetc(stdin);

        if (c == EOF || c == '\n') {
            break;
        }

        // Resize buffer if needed
        if (position >= buffer_size - 1) {
            buffer_size *= 2; 
            char *new_buffer = (char*)realloc(buffer, buffer_size * sizeof(char));
            if (!new_buffer) {
                free(buffer);
                perror("Memory reallocation failed");
                return NULL;
            }
            buffer = new_buffer;
        }
        
        buffer[position++] = (char)c;
    }

    buffer[position] = '\0';

    return buffer;
}


/* -------------------------------------------------------------------------
   validateCommand()
   Checks if an input string begins with the specified command (case-insensitive)
   and ensures that it is followed by either a space or string termination.
   Returns: pointer to remainder of the string (after command) OR NULL if invalid.
---------------------------------------------------------------------------*/
char* validateCommand(const char *input, const char *cmd) {
    if (!input || !cmd) return NULL;
    
    const char *ptr = input;

    while (*ptr && isspace((unsigned char)*ptr)) {
        ptr++;
    }

    int len = strlen(cmd);

    // ensure command is followed by space or string end
    if (strncasecmp(ptr, cmd, len) != 0) {
        return NULL;
    }

    // ensure command is followed by space or string end
    if (!isspace((unsigned char)ptr[len]) && ptr[len] != '\0') {
        return NULL;
    }

    ptr += len;

    while (*ptr && isspace((unsigned char)*ptr)) {
        ptr++; // skip spaces after command
    }

    return (char*)ptr;
}


/* -------------------------------------------------------------------------
   validateID()
   Validates student ID format: must be exactly 7 digits and start with '2'.
   Returns: 1 if valid, 0 otherwise.
---------------------------------------------------------------------------*/
int validateID(const char *id_str) {
    if (!id_str || strlen(id_str) != 7) {
        return 0; 
    }
    if (id_str[0] != '2') {
        return 0; 
    }
    for (int i = 0; i < 7; i++) {
        if (!isdigit((unsigned char)id_str[i])) {
            return 0; 
        }
    }
    return 1;
}


/* -------------------------------------------------------------------------
   toTitleCase()
   Converts entire string into Title Case formatting:
   - First letter of each word is uppercase
   - Remaining characters converted to lowercase
---------------------------------------------------------------------------*/
void toTitleCase(char *str) {
    int capitalizeNext = 1;
    for (int i = 0; str[i]; i++) {
        if (isspace((unsigned char)str[i])) capitalizeNext = 1;
        else if (capitalizeNext) {
            str[i] = toupper((unsigned char)str[i]);
            capitalizeNext = 0;
        } else str[i] = tolower((unsigned char)str[i]);
    }
}



/* -------------------------------------------------------------------------
   validateMark()
   Validates numerical mark field:
   - Accepts numbers with optional sign and one decimal point.
   - Accepts empty values when optional.
   Returns: 1 if valid, 0 otherwise.
---------------------------------------------------------------------------*/
int validateMark(const char *str) {
    if (!str || !*str) return 1;
    int dotCount = 0, digitFound = 0, i = 0;
    if (str[i] == '+' || str[i] == '-') i++;
    for (; str[i]; i++) {
        if (isdigit((unsigned char)str[i])) digitFound = 1;
        else if (str[i] == '.') {
            dotCount++;
            if (dotCount > 1) return 0;
        } else return 0;
    }
    return digitFound;
}


/* -------------------------------------------------------------------------
   parseCommand()
   Parses CMS command key-value pairs formatted as:
        ID=xxxx NAME=xxxx PROGRAMME=xxxx MARK=xx
   Features:
   - Enforces required vs optional fields depending on command type.
   - Performs validation for ID, programme length, format, and mark boundaries.
   - Trims whitespace and rejects malformed formatting (duplicate fields, 
     missing '=', trailing spaces, invalid keys).
   Returns: 1 if successfully parsed, 0 if invalid.
---------------------------------------------------------------------------*/
int parseCommand(const char *input, int *id, char *name, char *programme, float *mark, int optionalMode) {
    // Create a mutable copy of the input string for parsing
    char *buf = strdup(input);
    if (!buf) {
        perror("Memory allocation failed for command parsing");
        return 0;
    }
    
    *id = -1;
    if (name) name[0] = '\0';
    if (programme) programme[0] = '\0';
    if (mark) *mark = 0.0f;

    const char *keys[] = {"ID", "NAME", "PROGRAMME", "MARK"};
    const int keyCount = 4;

    int found[keyCount];
    memset(found, 0, sizeof(found)); // Track which fields have already appeared

    char *ptr = buf;

    int optionalProvided = 0; // Flag to track if at least one optional field is present

    while (*ptr) {
        int fieldIndex = -1;
        // Identify which field starts at the current pointer
        for (int i = 0; i < keyCount; i++) {
            int len = strlen(keys[i]);
            if (strncasecmp(ptr, keys[i], len) == 0) {
                char after = ptr[len];
                if (after == '=' || isspace(after)) {
                    fieldIndex = i;
                    break;
                }
            }
        }

        if (fieldIndex == -1) {
            printf("CMS: Invalid command. Unknown field or missing '='.\n");
            free(buf);
            return 0;
        }

        if (found[fieldIndex]) {
            printf("CMS: Invalid command. Duplicate field.\n");
            free(buf);
            return 0;
        }
        found[fieldIndex] = 1;

        // Ensure '=' immediately follows the field name
        int keyLen = strlen(keys[fieldIndex]);
        char *afterKey = ptr + keyLen;
        int spaces = 0;
        while (*afterKey && isspace((unsigned char)*afterKey)) {
            spaces++;
            afterKey++;
        }

        if (*afterKey != '=') {
            printf("CMS: Invalid command. Missing '='.\n");
            free(buf);
            return 0;
        }

        if (spaces > 0) {
            printf("CMS: Invalid command. No space allowed before '='.\n");
            free(buf);
            return 0;
        }

        ptr = afterKey + 1;
        while (*ptr && isspace((unsigned char)*ptr)) ptr++; // Skip spaces after '='

        // Extract value for current field up to next key or end of string
        char *valueStart = ptr;
        while (*ptr) {
            int isNextKey = 0;
            for (int i = 0; i < keyCount; i++) {
                int len = strlen(keys[i]);
                if (strncasecmp(ptr, keys[i], len) == 0) {
                    char after = ptr[len];
                    if (after == '=' || isspace(after)) {
                        isNextKey = 1;
                        break;
                    }
                }
            }
            if (isNextKey) break;
            ptr++;
        }

        char *valueEnd = ptr;
        while (valueEnd > valueStart && isspace((unsigned char)*(valueEnd - 1)))
            valueEnd--;

        int len = valueEnd - valueStart; // TRUE length of the value is calculated here.
        
        // Use a temporary buffer for validation and cap copy length to prevent overflow
        char value[MAX_LINE] = {0}; 
        
        // Calculate safe copy length, preventing overflow in the temporary 'value' buffer
        size_t copyLen = (size_t)len;
        if (copyLen >= MAX_LINE) {
            copyLen = MAX_LINE - 1; // Cap to buffer size - 1
        }
        
        // Use safe copy length
        strncpy(value, valueStart, copyLen);
        value[copyLen] = '\0'; // Explicitly null-terminate the temporary buffer

        if (optionalMode == OPTIONAL_NONE && fieldIndex != 0) {
            printf("CMS: Invalid command. Only ID allowed.\n");
            free(buf);
            return 0;
        }

        // Validate and store the extracted field
        switch (fieldIndex) {
            case 0: // ID
                if (len == 0) {
                    printf("CMS: Missing required ID.\n");
                    free(buf);
                    return 0;
                }
                if (!validateID(value)) {
                    printf("CMS: Invalid command. ID must be 7 digits starting with '2'.\n");
                    free(buf);
                    return 0;
                }
                *id = atoi(value);
                break;

            case 1: // NAME
                if (!name) { free(buf); return 0; }
                if (len > 0) {
                    if (len >= MAX_NAME) {
                        printf("CMS: Invalid command. Name is too long (Max %d characters).\n", MAX_NAME - 1);
                        free(buf);
                        return 0;
                    }

                    strncpy(name, value, MAX_NAME - 1); 
                    name[MAX_NAME - 1] = '\0';
                    toTitleCase(name);
                    optionalProvided = 1;
                }
                break;

            case 2: // PROGRAMME      
                if (!programme) { free(buf); return 0; }
                if (len > 0) {
                    if (len >= MAX_PROGRAMME) {
                        printf("CMS: Invalid command. Programme is too long (Max %d characters).\n", MAX_PROGRAMME - 1);
                        free(buf);
                        return 0;
                    }

                    strncpy(programme, value, MAX_PROGRAMME - 1); 
                    programme[MAX_PROGRAMME - 1] = '\0';
                    toTitleCase(programme);
                    optionalProvided = 1;
                }
                break;

            case 3: // MARK            
                if (!mark) { free(buf); return 0; }
                if (len > 0) {
                    if (!validateMark(value)) {
                        printf("CMS: Invalid command. Mark must be numeric.\n");
                        free(buf); 
                        return 0;
                    }
                    
                    float tempMark = atof(value);
                    if (tempMark < 0.0f || tempMark > 100.0f) {
                        printf("CMS: Invalid command. Mark must be between 0 - 100.\n");
                        free(buf); 
                        return 0;
                    }

                    *mark = tempMark;
                    optionalProvided = 1;
                }
                break;
        }

        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
    }

    if (*id == -1) { // ID is always required
        printf("CMS: Missing required ID.\n");
        free(buf);
        return 0;
    }

    if (optionalMode == OPTIONAL_REQUIRED && !optionalProvided) {
        printf("CMS: At least one of NAME, PROGRAMME, or MARK must be provided for UPDATE.\n");
        free(buf);
        return 0;
    }

    free(buf);
    return 1;
}


/* -------------------------------------------------------------------------
   handleShow()
   Processes the SHOW command and its variations:
   - SHOW ALL
   - SHOW ALL SORT BY (ID|MARK) [ASC|DESC]
   - SHOW SUMMARY [PROGRAMME=value] 
   Performs syntax validation and delegates execution to display functions.
   Returns: 1 after processing (not used as boolean result).
---------------------------------------------------------------------------*/
int handleShow(const char *input) {
    char *buf = strdup(input);
    if (!buf) {
        perror("Memory allocation failed for handleShow");
        return 1;
    }
    
    // Extract first token to determine SHOW type
    char *token = strtok(buf, " ");

    if (!token) {
        printf("CMS: Enter a valid SHOW command.\n");
        free(buf);
        return 1;
    }

    // SHOW ALL with no extra options: display all records
    if (strcasecmp(token, "ALL") == 0) {
        token = strtok(NULL, " ");

        if (!token) {
            showDB();
            free(buf);
            return 1;
        }

        // SHOW ALL SORT BY <field> [ASC|DESC]
        if (strcasecmp(token, "SORT") == 0) {
            token = strtok(NULL, " ");
            if (!token || strcasecmp(token, "BY") != 0) {
                printf("CMS: Expected 'SORT BY'.\n");
                free(buf);
                return 1;
            }

            // Determine sort field
            token = strtok(NULL, " ");
            if (!token) {
                printf("CMS: Missing sort field (ID or MARK).\n");
                free(buf);
                return 1;
            }

            int sortByID;
            if (strcasecmp(token, "ID") == 0) {
                sortByID = 1;
            }
                
            else if (strcasecmp(token, "MARK") == 0) {
                sortByID = 0;
            }
                
            else {
                printf("CMS: Invalid sort field. Use ID or MARK.\n");
                free(buf); 
                return 1;
            }

            // Determine sort order (optional)
            token = strtok(NULL, " ");
            int ascending = 1; // default ascending

            if (token) {
                if (strcasecmp(token, "DESC") == 0) ascending = 0;
                else if (strcasecmp(token, "ASC") == 0) ascending = 1;
                else {
                    printf("CMS: Invalid sort order. Use ASC or DESC.\n");
                    free(buf);
                    return 1;
                }

                // Check for trailing invalid input
                if ((token = strtok(NULL, " ")) != NULL) {
                    printf("CMS: Invalid trailing input.\n");
                    free(buf);
                    return 1;
                }
            }
            showDBSorted(sortByID, ascending);
            free(buf);
            return 1;
        }

        printf("CMS: Invalid SHOW ALL format.\n");
        free(buf);
        return 1;
    }

    // SHOW SUMMARY
    if (strcasecmp(token, "SUMMARY") == 0) {
        char programme[MAX_PROGRAMME] = ""; // Buffer to store optional programme filter
        int hasFilter = 0;                  // Flag to indicate if a filter was provided

        char *rest = strtok(NULL, ""); // Get the rest of the input after "SUMMARY" (entire remaining string)
        if (rest) {
            char *ptr = rest;

            // Loop through each segment in the remaining string
            while (*ptr) {
                // Skip leading whitespace before processing next key=value pair
                while (*ptr && isspace((unsigned char)*ptr)) ptr++;
                if (!*ptr) break;

                char *eq = strchr(ptr, '=');
                if (!eq) { // '=' not found → invalid format
                    printf("CMS: Invalid filter format. Use key=value.\n");
                    free(buf);
                    return 1;
                }

                // Check for spaces immediately before '=' (not allowed)
                if (*(eq - 1) == ' ' || *(eq - 1) == '\t') {
                    printf("CMS: Invalid command. No space allowed before '='.\n");
                    free(buf);
                    return 1;
                }

                char *key = ptr;        // Key starts at current pointer
                *eq = '\0';             // Terminate key string at '='
                char *value = eq + 1;   // Value starts immediately after '='

                // Skip leading spaces in value
                while (*value && isspace((unsigned char)*value)) value++;

                // Determine the end of the value by trimming trailing spaces
                char *valEnd = value + strlen(value);
                while (valEnd > value && isspace((unsigned char)*(valEnd - 1))) valEnd--;
                *valEnd = '\0'; // Null-terminate value string

                // Process recognized filter key: PROGRAMME
                if (strcasecmp(key, "PROGRAMME") == 0) {
                    if (strlen(value) >= MAX_PROGRAMME) {
                        printf("CMS: Programme too long.\n");
                        free(buf);
                        return 1;
                    }
                    // Copy value into programme buffer and ensure null-termination
                    strncpy(programme, value, MAX_PROGRAMME - 1);
                    programme[MAX_PROGRAMME - 1] = '\0';
                    toTitleCase(programme); // Convert to title case
                    hasFilter = 1;          // Mark that a filter has been provided
                } else {
                    printf("CMS: Unknown filter key '%s'.\n", key);
                    free(buf);
                    return 1;
                }
                // Move pointer to end of current value to continue parsing next key=value
                ptr = valEnd;
            }
        }
        showSummary(hasFilter ? programme : NULL);
        free(buf);
        return 1;
    }
    printf("CMS: Unknown SHOW command.\n");
    free(buf);
    return 1;
}