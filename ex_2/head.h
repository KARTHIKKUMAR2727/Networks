#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT 4000
#define MAX_BIN_INPUT (MAX_INPUT * 8)
#define MAX_FRAMES 500
#define MAX_STUFFED (MAX_BIN_INPUT * 2)
#define MAX_FRAME (MAX_STUFFED + 200)
#define MAX_FILE_STREAM 400000

#define FLAG_BITS "01111110"
#define ADDRESS_BITS "11111111"
#define CONTROL_BITS "00000011"

#define FLAG_LEN 8
#define ADDR_LEN 8
#define CTRL_LEN 8
#define ADDR_CTRL_LEN 16

#define CRC_LEN 16
#define CRC16_POLY "10001000000100001"
#define CRC16_POLY_LEN 17

#define POLY_STR_LEN 256

typedef enum
{
    STATUS_OK = 0,
    STATUS_ALLOC_FAIL = 1,
    STATUS_FILE_OPEN_FAIL = 2,
    STATUS_INVALID_START_FLAG = 3,
    STATUS_INVALID_END_FLAG = 4,
    STATUS_FRAME_TOO_SHORT = 5,
    STATUS_INVALID_FCS_LEN = 6,
    STATUS_INVALID_INPUT = 7,
    STATUS_NO_MORE_FRAMES = 8,
    STATUS_CORRUPTED_DATA = 9,
    STATUS_NO_END_FLAG_FOUND = 10,
    STATUS_CRC_MISMATCH = 11
} Status;

typedef enum
{
    INPUT_STRING = 1,
    INPUT_BINARY = 2
} InputMode;

typedef struct
{
    char address[ADDR_LEN];
    char control[CTRL_LEN];
    char binaryData[MAX_BIN_INPUT];
    int binaryLen;
    char stuffedData[MAX_STUFFED];
    int stuffedLen;
    char fcs[CRC_LEN];
    int fcsLen;
    char finalFrame[MAX_FRAME];
    int finalLen;

    char dividendBits[MAX_BIN_INPUT + CRC_LEN];
    int dividendLen;
    char remainderTrace[MAX_FRAME];
} Frame;

typedef struct
{
    char address[ADDR_LEN];
    char control[CTRL_LEN];
    char fcs[CRC_LEN];
    int fcsLen;
    char stuffedData[MAX_STUFFED];
    int stuffedLen;
    char binaryData[MAX_BIN_INPUT];
    int binaryLen;

    char recomputedCrc[CRC_LEN];
} Decoded;

int isValidBitString(const char str[], int len);
int stringToBinary(const char input[], int inputLen, char binary[]);
int binaryToString(const char binary[], int binaryLen, char output[]);

int bitStuff(const char input[], int inputLen, char stuffed[]);
Status bitDestuff(const char stuffed[], int stuffedLen, char original[], int *originalLen);

int splitIntoFrames(const char fullBinary[], int totalLen, int frameSizeBits, Frame frames[], int maxFrames);

void computeCRC16(char data[], int dataLen, char crcOut[CRC_LEN], char trace[]);
int verifyCRC16(char data[], int dataLen, const char receivedCrc[CRC_LEN], char trace[]);

void polynomialToString(const char bits[], int len, char out[]);

Status buildFrame(Frame *frame);

Status appendStreamToFile(const char filename[], const char bits[], int bitsLen);
Status readEntireStreamFromFile(const char filename[], char bits[], int maxLen, int *outLen);

Status extractNextFrame(const char stream[], int streamLen, int startIndex, int *endIndex,
                        Decoded *decoded);

void injectSingleBitError(char frameBits[], int frameLen, int bitPosition);
