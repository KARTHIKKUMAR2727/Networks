#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DATA   100
#define MAX_CODE   150
#define FILE_NAME  "hamming.txt"

int calculateRedundantBits(int m);
int isPowerOfTwo(int n);
void encodeHamming(const char data[], int m, int r, char code[], int *codeLen);
int detectAndCorrect(char code[], int n, int r);
void extractOriginalData(const char code[], int n, char data[], int *dataLen);
int isValidBitString(const char str[], int len);
void writeCodeToFile(const char filename[], const char code[], int n);
int readCodeFromFile(const char filename[], char code[], int maxLen);
