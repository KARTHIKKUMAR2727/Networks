#include "head.h"

int calculateRedundantBits(int m)
{
    int r = 0;
    while ((1 << r) < (m + r + 1))
    {
        r++;
    }
    return r;
}

int isPowerOfTwo(int n)
{
    return (n > 0) && ((n & (n - 1)) == 0);
}

void encodeHamming(const char data[], int m, int r, char code[], int *codeLen)
{
    int n = m + r;
    int i, j;
    int dataIndex = 0;

    for (i = 1; i <= n; i++)
    {
        if (isPowerOfTwo(i))
            code[i - 1] = '0';
        else
            code[i - 1] = data[dataIndex++];
    }

    for (i = 0; i < r; i++)
    {
        int parityPos = (1 << i);
        int parity = 0;

        for (j = 1; j <= n; j++)
        {
            if ((j & parityPos) && j != parityPos)
                parity ^= (code[j - 1] - '0');
        }

        code[parityPos - 1] = (char)(parity + '0');
    }

    *codeLen = n;
}

int detectAndCorrect(char code[], int n, int r)
{
    int i, j;
    int errorPos = 0;

    for (i = 0; i < r; i++)
    {
        int parityPos = (1 << i);
        int parity = 0;

        for (j = 1; j <= n; j++)
        {
            if (j & parityPos)
                parity ^= (code[j - 1] - '0');
        }

        if (parity != 0)
            errorPos += parityPos;
    }

    if (errorPos != 0 && errorPos <= n)
    {
        code[errorPos - 1] = (code[errorPos - 1] == '0') ? '1' : '0';
    }

    return errorPos;
}

void extractOriginalData(const char code[], int n, char data[], int *dataLen)
{
    int i;
    int idx = 0;

    for (i = 1; i <= n; i++)
    {
        if (!isPowerOfTwo(i))
            data[idx++] = code[i - 1];
    }

    data[idx] = '\0';
    *dataLen = idx;
}

int isValidBitString(const char str[], int len)
{
    int i;

    if (len <= 0)
        return 0;

    for (i = 0; i < len; i++)
    {
        if (str[i] != '0' && str[i] != '1')
            return 0;
    }

    return 1;
}

void writeCodeToFile(const char filename[], const char code[], int n)
{
    FILE *fp = fopen(filename, "w");

    if (fp == NULL)
    {
        printf("\nError opening file for writing.\n");
        return;
    }

    fwrite(code, 1, n, fp);
    fclose(fp);
}

int readCodeFromFile(const char filename[], char code[], int maxLen)
{
    FILE *fp = fopen(filename, "r");
    int c;
    int idx = 0;

    if (fp == NULL)
    {
        printf("\nError opening file for reading.\n");
        return -1;
    }

    while ((c = fgetc(fp)) != EOF && idx < maxLen)
    {
        if (c == '0' || c == '1')
            code[idx++] = (char)c;
    }

    fclose(fp);
    return idx;
}
