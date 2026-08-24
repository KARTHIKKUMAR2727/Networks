#include "head.h"

static void readLine(char buffer[], int maxLen)
{
    if (fgets(buffer, maxLen, stdin) != NULL)
    {
        int len = (int)strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
    }
    else
    {
        buffer[0] = '\0';
    }
}

static int promptYesNo(const char prompt[])
{
    char buffer[16];
    printf("%s", prompt);
    readLine(buffer, sizeof(buffer));
    return (buffer[0] == 'y' || buffer[0] == 'Y');
}

int main(void)
{
    char data[MAX_DATA];
    char code[MAX_CODE];
    char received[MAX_CODE];
    char recoveredData[MAX_DATA];
    int m, r, n;
    int codeLen;
    int fileLen;
    int recoveredLen;
    int errorPos;

    printf("Enter Binary Data : ");
    readLine(data, MAX_DATA);
    m = (int)strlen(data);

    if (!isValidBitString(data, m))
    {
        printf("\nInvalid input. Only 0/1 allowed.\n");
        return 1;
    }

    r = calculateRedundantBits(m);
    n = m + r;

    printf("\nNumber of Data Bits (m)       : %d\n", m);
    printf("Number of Redundant Bits (r)  : %d\n", r);
    printf("Total Code Length (m + r)     : %d\n", n);

    {
        int i;
        printf("\nRedundant Bit (r) Values (positions) :\n");
        for (i = 0; i < r; i++)
        {
            printf("  r%d -> position %d\n", i + 1, (1 << i));
        }
    }

    encodeHamming(data, m, r, code, &codeLen);
    printf("\nEncoded Hamming Code : %.*s\n", codeLen, code);

    if (promptYesNo("\nIntroduce an error into the code for testing? (y/n) : "))
    {
        int pos;
        do
        {
            printf("Enter Bit Position to flip (1 to %d) : ", codeLen);
            scanf("%d", &pos);
            {
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
            }
            if (pos < 1 || pos > codeLen)
                printf("Invalid position.\n");
        }
        while (pos < 1 || pos > codeLen);

        code[pos - 1] = (code[pos - 1] == '0') ? '1' : '0';
        printf("Error introduced at bit position %d.\n", pos);
        printf("Corrupted Code : %.*s\n", codeLen, code);
    }

    writeCodeToFile(FILE_NAME, code, codeLen);
    printf("\nCode stored in file: %s\n", FILE_NAME);

    fileLen = readCodeFromFile(FILE_NAME, received, MAX_CODE);

    if (fileLen != n)
    {
        printf("\nError: retrieved code length (%d) does not match expected length (%d).\n", fileLen, n);
        return 1;
    }

    printf("\nRetrieved Code from file : %.*s\n", fileLen, received);

    errorPos = detectAndCorrect(received, n, r);

    printf("\n------------------------------------------\n");
    if (errorPos == 0)
    {
        printf("No Error Detected.\n");
    }
    else
    {
        printf("Error Detected at bit position : %d\n", errorPos);
        printf("Corrected Code                 : %.*s\n", n, received);
    }
    printf("------------------------------------------\n");

    extractOriginalData(received, n, recoveredData, &recoveredLen);
    printf("\nRecovered Original Data : %.*s\n", recoveredLen, recoveredData);

    return 0;
}
