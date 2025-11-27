#ifndef INPUT_VALIDATION_H
#define INPUT_VALIDATION_H

#define OPTIONAL_NONE 0
#define OPTIONAL_REQUIRED 1
#define OPTIONAL_ALLOWED_EMPTY 2

char* readLine();
char* validateCommand(const char *input, const char *cmd);
void toTitleCase(char *str);
int validateMark(const char *str);

int parseCommand(const char *input, int *id, char *name, char *programme, float *mark, int optionalMode);
int handleShow(const char *input);

#endif