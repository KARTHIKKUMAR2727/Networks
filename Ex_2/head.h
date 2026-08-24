#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define K 8
struct Frame
{
    char *data;
    int dataLength;
};
void initializeFrame(struct Frame *frame);
void readFile(char fileName[], struct Frame *frame);
void writeFile(char fileName[], char *data);
void twoDParitySender(char *data);
void twoDParityReceiver(void);
void checksumSender(char *data);
void checksumReceiver(void);

