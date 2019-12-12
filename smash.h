
//----------------------------------------------------------------------------
//
// Sajia Zafreen
// smash.h -- Defines prototypes and symbols for smash.c and commands.c
//
//----------------------------------------------------------------------------

void commandEnd (char *arg);
int flushEmptyLine (char *buffer);
void executeCommand(char *str);
int executeExternalCommand(char *arg0, char *args[], int inRedirect, int outRedirect, char *indexIn,char *indexOut);
int ifCommand(char const *p, char const *q);
int cd (char *path);
//int hasPrefix(char const *p, char const *q);