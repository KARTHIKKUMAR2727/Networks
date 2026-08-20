#include "head.h"

#define FRAME_FILE "frame.txt"

static void readLine(char buffer[], int maxLen)
{
    if(fgets(buffer, maxLen, stdin) != NULL)
    {
        int len = (int)strlen(buffer);

        if(len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }
    }
    else
    {
        buffer[0] = '\0';
    }
}

static void flushStdinLine(void)
{
    int c;

    while((c = getchar()) != '\n' && c != EOF)
    {
    }
}

static int readIntChoice(const char prompt[])
{
    int value;

    printf("%s", prompt);

    while(scanf("%d", &value) != 1)
    {
        printf("\nInvalid input. Please enter a number: ");
        flushStdinLine();
    }

    flushStdinLine();

    return value;
}

static void printStatusMessage(Status status)
{
    switch(status)
    {
        case STATUS_OK:
            break;
        case STATUS_ALLOC_FAIL:
            printf("\nMemory Allocation Failed.\n");
            break;
        case STATUS_FILE_OPEN_FAIL:
            printf("\nError Opening File.\n");
            break;
        case STATUS_INVALID_START_FLAG:
            printf("\nInvalid Start Flag.\n");
            break;
        case STATUS_INVALID_END_FLAG:
            printf("\nInvalid End Flag.\n");
            break;
        case STATUS_FRAME_TOO_SHORT:
            printf("\nFrame Too Short.\n");
            break;
        case STATUS_INVALID_FCS_LEN:
            printf("\nInvalid FCS Length.\n");
            break;
        case STATUS_INVALID_INPUT:
            printf("\nInvalid Input.\n");
            break;
        case STATUS_NO_MORE_FRAMES:
            printf("\nNo more frames in stream.\n");
            break;
        case STATUS_CORRUPTED_DATA:
            printf("\nData Corrupted (bad stuffing pattern detected).\n");
            break;
        case STATUS_NO_END_FLAG_FOUND:
            printf("\nEnd Flag Not Found.\n");
            break;
        default:
            printf("\nUnknown Error.\n");
            break;
    }
}

static StuffMode promptStuffMode(void)
{
    int choice;

    do
    {
        printf("\n1. Bit Stuffing\n2. Byte Stuffing\n");
        choice = readIntChoice("Enter Choice : ");

        if(choice != 1 && choice != 2)
        {
            printf("Invalid choice. Enter 1 or 2.\n");
        }
    }
    while(choice != 1 && choice != 2);

    return (choice == 1) ? STUFF_BIT : STUFF_BYTE;
}

static InputMode promptInputMode(void)
{
    int choice;

    do
    {
        printf("\n1. String Data\n2. Binary Data\n");
        choice = readIntChoice("Enter Choice : ");

        if(choice != 1 && choice != 2)
        {
            printf("Invalid choice. Enter 1 or 2.\n");
        }
    }
    while(choice != 1 && choice != 2);

    return (choice == 1) ? INPUT_STRING : INPUT_BINARY;
}

/* Returns the length of the entered data via *outLen. buffer is still
   read as a plain C string from stdin (fgets needs that), but as soon as
   we know its length we track it explicitly from here on. */
static void promptInputLine(char buffer[], InputMode inputMode, int *outLen)
{
    int valid;
    int len;

    do
    {
        valid = 1;

        if(inputMode == INPUT_STRING)
        {
            printf("\nEnter Data : ");
        }
        else
        {
            printf("\nEnter Binary Data (multiple of 8 bits) : ");
        }

        readLine(buffer, MAX_INPUT);
        len = (int)strlen(buffer);

        if(len == 0)
        {
            printf("Input cannot be empty.\n");
            valid = 0;
        }
        else if(inputMode == INPUT_BINARY)
        {
            if(!isValidBitString(buffer, len) || (len % 8) != 0)
            {
                printf("Invalid binary data. Must be only 0/1 and a multiple of 8 bits.\n");
                valid = 0;
            }
        }
    }
    while(!valid);

    *outLen = len;
}

static int promptFrameSize(int maxBits)
{
    int frameSize;

    do
    {
        frameSize = readIntChoice("\nEnter Frame Size (in bytes) : ");

        if(frameSize <= 0)
        {
            printf("Frame size must be a positive integer.\n");
        }
    }
    while(frameSize <= 0);

    if(frameSize * 8 > maxBits)
    {
        frameSize = maxBits / 8;

        if(frameSize <= 0)
        {
            frameSize = 1;
        }
    }

    return frameSize;
}

static void runSender(Frame *frames, int frameCount)
{
    int i;

    remove(FRAME_FILE);

    printf("\n========== SENDER ==========\n");

    for(i = 0; i < frameCount; i++)
    {
        Status status;

        status = appendStreamToFile(FRAME_FILE, frames[i].finalFrame, frames[i].finalLen);

        printf("\nFrame %d built.\n", i + 1);

        if(status != STATUS_OK)
        {
            printStatusMessage(status);
        }
    }

    printf("\n------------------------------------------");
    printf("\nContents of %s (stream of bits, all frames)", FRAME_FILE);
    printf("\n------------------------------------------\n");

    {
        FILE *fp = fopen(FRAME_FILE, "r");
        int c;

        if(fp != NULL)
        {
            while((c = fgetc(fp)) != EOF)
            {
                putchar(c);
            }

            fclose(fp);
        }

        printf("\n");
    }
}

static void runReceiver(StuffMode stuffMode, InputMode inputMode)
{
    static char stream[MAX_FILE_STREAM];
    static char reassembledBinary[MAX_BIN_INPUT];
    Status readStatus;
    int streamLen = 0;
    int pos = 0;
    int frameNumber = 0;
    int reassembledLen = 0;

    readStatus = readEntireStreamFromFile(FRAME_FILE, stream, MAX_FILE_STREAM, &streamLen);

    if(readStatus != STATUS_OK)
    {
        printStatusMessage(readStatus);
        return;
    }

    printf("\n========== RECEIVER ==========\n");

    while(1)
    {
        Decoded decoded;
        int endIndex;
        Status status;

        status = extractNextFrame(stream, streamLen, pos, &endIndex, stuffMode, &decoded);

        if(status == STATUS_NO_MORE_FRAMES)
        {
            break;
        }

        frameNumber++;

        printf("\n------------------------------------------");
        printf("\nFrame %d", frameNumber);
        printf("\n------------------------------------------\n");

        if(status != STATUS_OK)
        {
            printStatusMessage(status);
            break;
        }

        printf("Address : %.*s\n", ADDR_LEN, decoded.address);
        printf("Control : %.*s\n", CTRL_LEN, decoded.control);
        printf("FCS     : %.*s\n", FCS_LEN, decoded.fcs);
        printf("Stuffed Data (destuffing input)  : %.*s\n", decoded.stuffedLen, decoded.stuffedData);
        printf("Binary Data (after destuffing)   : %.*s\n", decoded.binaryLen, decoded.binaryData);

        memcpy(reassembledBinary + reassembledLen, decoded.binaryData, decoded.binaryLen);
        reassembledLen += decoded.binaryLen;

        pos = endIndex;
    }

    if(frameNumber == 0)
    {
        printf("\nNo frames found in %s\n", FRAME_FILE);
        return;
    }

    printf("\n------------------------------------------");
    printf("\nReassembled Binary Data (all frames combined)");
    printf("\n------------------------------------------\n");
    printf("%.*s\n", reassembledLen, reassembledBinary);

    printf("\n------------------------------------------");
    printf("\nFinal Recovered Data");
    printf("\n------------------------------------------\n");

    if(inputMode == INPUT_STRING)
    {
        static char recoveredString[MAX_INPUT];
        int recoveredLen;

        recoveredLen = binaryToString(reassembledBinary, reassembledLen, recoveredString);
        printf("%.*s\n", recoveredLen, recoveredString);
    }
    else
    {
        printf("%.*s\n", reassembledLen, reassembledBinary);
    }
}

int main()
{
    static char inputBuffer[MAX_INPUT];
    static char fullBinary[MAX_BIN_INPUT];
    static Frame frames[MAX_FRAMES];
    int inputLen;
    int fullBinaryLen;
    int frameSize;
    int frameCount;
    StuffMode stuffMode;
    InputMode inputMode;

    printf("\n==============================================");
    printf("\n   HDLC-STYLE FRAMING (STREAM OF BITS)");
    printf("\n==============================================\n");

    stuffMode = promptStuffMode();
    inputMode = promptInputMode();

    promptInputLine(inputBuffer, inputMode, &inputLen);

    if(inputMode == INPUT_STRING)
    {
        fullBinaryLen = stringToBinary(inputBuffer, inputLen, fullBinary);
    }
    else
    {
        memcpy(fullBinary, inputBuffer, inputLen);
        fullBinaryLen = inputLen;
    }

    frameSize = promptFrameSize(fullBinaryLen);

    frameCount = splitIntoFrames(fullBinary, fullBinaryLen, frameSize * 8, frames, MAX_FRAMES);

    if(frameCount <= 0)
    {
        printStatusMessage(STATUS_INVALID_INPUT);
        return 1;
    }

    {
        int i;

        for(i = 0; i < frameCount; i++)
        {
            buildFrame(&frames[i], stuffMode);
        }
    }

    printf("\nInput split into %d frame(s) of up to %d byte(s) each.\n", frameCount, frameSize);

    runSender(frames, frameCount);

    runReceiver(stuffMode, inputMode);

    printf("\nProgram Terminated Successfully.\n");

    return 0;
}
