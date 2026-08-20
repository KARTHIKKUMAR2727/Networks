#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT        4000
#define MAX_BIN_INPUT    (MAX_INPUT * 8)
#define MAX_FRAMES       500
#define MAX_STUFFED      (MAX_BIN_INPUT * 2)
#define MAX_FRAME        (MAX_STUFFED + 200)
#define MAX_FILE_STREAM  400000

#define FLAG_BITS        "01111110"
#define ESCAPE_BITS      "01111101"
#define ADDRESS_BITS     "11111111"
#define CONTROL_BITS     "00000011"
#define FCS_BITS         "00000000000000000000000000000000"

#define FLAG_LEN     8
#define ADDR_LEN     8
#define CTRL_LEN     8
#define FCS_LEN      32

#define ADDR_CTRL_LEN (ADDR_LEN + CTRL_LEN)

typedef enum
{
    STATUS_OK                 = 0,
    STATUS_ALLOC_FAIL         = 1,
    STATUS_FILE_OPEN_FAIL     = 2,
    STATUS_INVALID_START_FLAG = 3,
    STATUS_INVALID_END_FLAG   = 4,
    STATUS_FRAME_TOO_SHORT    = 5,
    STATUS_INVALID_FCS_LEN    = 6,
    STATUS_INVALID_INPUT      = 7,
    STATUS_NO_MORE_FRAMES     = 8,
    STATUS_CORRUPTED_DATA     = 9,
    STATUS_NO_END_FLAG_FOUND  = 10
} Status;

typedef enum
{
    STUFF_BIT  = 1,
    STUFF_BYTE = 2
} StuffMode;

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
    int  binaryLen;
    char stuffedData[MAX_STUFFED];
    int  stuffedLen;
    char fcs[FCS_LEN];
    char finalFrame[MAX_FRAME];
    int  finalLen;
} Frame;

typedef struct
{
    char address[ADDR_LEN];
    char control[CTRL_LEN];
    char fcs[FCS_LEN];
    char stuffedData[MAX_STUFFED];
    int  stuffedLen;
    char binaryData[MAX_BIN_INPUT];
    int  binaryLen;
} Decoded;

Frame *allocateFrames(int n);
void freeFrames(Frame *frames);

int isValidBitString(const char str[], int len);

int stringToBinary(const char input[], int inputLen, char binary[]);
int binaryToString(const char binary[], int binaryLen, char output[]);

int bitStuff(const char input[], int inputLen, char stuffed[]);
Status bitDestuff(const char stuffed[], int stuffedLen, char original[], int *originalLen);

int byteStuff(const char input[], int inputLen, char stuffed[]);
Status byteDestuff(const char stuffed[], int stuffedLen, char original[], int *originalLen);

int splitIntoFrames(const char fullBinary[], int totalLen, int frameSizeBits, Frame frames[], int maxFrames);

Status buildFrame(Frame *frame, StuffMode stuffMode);

Status appendStreamToFile(const char filename[], const char bits[], int bitsLen);
Status readEntireStreamFromFile(const char filename[], char bits[], int maxLen, int *outLen);

Status extractNextFrame(const char stream[], int streamLen, int startIndex, int *endIndex,
                         StuffMode stuffMode, Decoded *decoded);
