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
        case STATUS_CRC_MISMATCH:
            printf("\nCRC Mismatch - Frame is corrupted.\n");
            break;
        default:
            printf("\nUnknown Error.\n");
            break;
    }
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

static void promptErrorInjection(Frame *frames, int frameCount)
{
    int choice;

    printf("\n==============================================");
    printf("\n   INTRODUCE A TRANSMISSION ERROR?");
    printf("\n==============================================\n");
    printf("1. No, send frames unmodified\n");
    printf("2. Yes, flip one bit in a chosen frame\n");
    choice = readIntChoice("Enter Choice : ");

    if(choice != 2)
    {
        return;
    }

    {
        int frameNo;
        int bitPos;
        int low, high;

        do
        {
            frameNo = readIntChoice("Which frame number should be corrupted? : ");

            if(frameNo < 1 || frameNo > frameCount)
            {
                printf("Enter a frame number between 1 and %d.\n", frameCount);
            }
        }
        while(frameNo < 1 || frameNo > frameCount);

        low  = FLAG_LEN;
        high = frames[frameNo - 1].finalLen - FLAG_LEN - 1;

        printf("Frame %d is %d bits long on the wire (positions %d..%d are flippable,\n",
               frameNo, frames[frameNo - 1].finalLen, low, high);
        printf("i.e. anything after the opening flag and before the closing flag).\n");
        printf("Enter a bit position in that range, or -1 for a random position : ");
        bitPos = readIntChoice("");

        {
            char before[MAX_FRAME];
            int flippedIndex;

            memcpy(before, frames[frameNo - 1].finalFrame, frames[frameNo - 1].finalLen);

            injectSingleBitError(frames[frameNo - 1].finalFrame, frames[frameNo - 1].finalLen, bitPos);

            flippedIndex = -1;
            {
                int k;

                for(k = 0; k < frames[frameNo - 1].finalLen; k++)
                {
                    if(before[k] != frames[frameNo - 1].finalFrame[k])
                    {
                        flippedIndex = k;
                        break;
                    }
                }
            }

            printf("\n>> Bit %d of frame %d flipped: '%c' -> '%c'\n",
                   flippedIndex, frameNo,
                   before[flippedIndex], frames[frameNo - 1].finalFrame[flippedIndex]);
            printf(">> This simulates line noise corrupting one bit in transit.\n");
            printf(">> The receiver's CRC check should now detect this frame as corrupted.\n");
        }
    }
}

static void runSender(Frame *frames, int frameCount)
{
    int i;

    remove(FRAME_FILE);

    printf("\n==============================================");
    printf("\n   SENDER  (bit stuffing + CRC-16 polynomial division)");
    printf("\n==============================================\n");

    for(i = 0; i < frameCount; i++)
    {
        Status status;
        char dataPoly[POLY_STR_LEN];
        char fcsPoly[POLY_STR_LEN];

        printf("\n------------------------------------------------------------\n");
        printf("FRAME %d\n", i + 1);
        printf("------------------------------------------------------------\n");

        printf("Address field            : %.*s\n", ADDR_LEN, frames[i].address);
        printf("Control field             : %.*s\n", CTRL_LEN, frames[i].control);
        printf("Raw data (payload)        : %.*s  (%d bits)\n",
               frames[i].binaryLen, frames[i].binaryData, frames[i].binaryLen);

        polynomialToString(frames[i].binaryData, frames[i].binaryLen, dataPoly);
        printf("Data as polynomial        : %s\n", dataPoly);

        printf("\n--- CRC-16 computation (polynomial long division) ---\n");
        printf("%s", frames[i].remainderTrace);

        polynomialToString(frames[i].fcs, frames[i].fcsLen, fcsPoly);
        printf("FCS (CRC remainder)       : %.*s\n", frames[i].fcsLen, frames[i].fcs);
        printf("FCS as polynomial         : %s\n", fcsPoly);

        printf("\nData after bit stuffing   : %.*s  (%d bits)\n",
               frames[i].stuffedLen, frames[i].stuffedData, frames[i].stuffedLen);

        printf("\nFinal frame on the wire   :\n");
        printf("  FLAG(%d) | ADDR(%d) | CTRL(%d) | STUFFED-DATA(%d) | FCS(%d) | FLAG(%d)\n",
               FLAG_LEN, ADDR_LEN, CTRL_LEN, frames[i].stuffedLen, frames[i].fcsLen, FLAG_LEN);
        printf("  %.*s\n", frames[i].finalLen, frames[i].finalFrame);
        printf("  Total frame length        : %d bits\n", frames[i].finalLen);

        status = appendStreamToFile(FRAME_FILE, frames[i].finalFrame, frames[i].finalLen);

        if(status != STATUS_OK)
        {
            printStatusMessage(status);
        }
    }

    printf("\n------------------------------------------------------------\n");
    printf("Contents of %s (full stream of bits, all frames concatenated)\n", FRAME_FILE);
    printf("------------------------------------------------------------\n");

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

static void runReceiver(InputMode inputMode)
{
    static char stream[MAX_FILE_STREAM];
    static char reassembledBinary[MAX_BIN_INPUT];
    Status readStatus;
    int streamLen = 0;
    int pos = 0;
    int frameNumber = 0;
    int reassembledLen = 0;
    int anyFrameFailed = 0;

    readStatus = readEntireStreamFromFile(FRAME_FILE, stream, MAX_FILE_STREAM, &streamLen);

    if(readStatus != STATUS_OK)
    {
        printStatusMessage(readStatus);
        return;
    }

    printf("\n==============================================");
    printf("\n   RECEIVER  (destuffing + CRC-16 verification)");
    printf("\n==============================================\n");

    while(1)
    {
        Decoded decoded;
        int endIndex;
        Status status;
        char recvTrace[MAX_FRAME];

        status = extractNextFrame(stream, streamLen, pos, &endIndex, &decoded);

        if(status == STATUS_NO_MORE_FRAMES)
        {
            break;
        }

        frameNumber++;

        printf("\n------------------------------------------------------------\n");
        printf("FRAME %d (received)\n", frameNumber);
        printf("------------------------------------------------------------\n");

        if(status != STATUS_OK && status != STATUS_CRC_MISMATCH)
        {
            printStatusMessage(status);
            anyFrameFailed = 1;
            break;
        }

        printf("Address field                     : %.*s\n", ADDR_LEN, decoded.address);
        printf("Control field                      : %.*s\n", CTRL_LEN, decoded.control);
        printf("Stuffed data (as received)         : %.*s  (%d bits)\n",
               decoded.stuffedLen, decoded.stuffedData, decoded.stuffedLen);
        printf("Data after destuffing               : %.*s  (%d bits)\n",
               decoded.binaryLen, decoded.binaryData, decoded.binaryLen);
        printf("FCS received                        : %.*s\n", decoded.fcsLen, decoded.fcs);

        printf("\n--- Receiver recomputes CRC-16 to verify (polynomial division) ---\n");
        verifyCRC16(decoded.binaryData, decoded.binaryLen, decoded.fcs, recvTrace);
        printf("%s", recvTrace);

        if(status == STATUS_CRC_MISMATCH)
        {
            printf("\nCRC Check Result                    : FAILED  (received FCS does not\n");
            printf("                                       match recomputed remainder)\n");
            printf("==> Frame %d is CORRUPTED and would be discarded / retransmission requested.\n",
                   frameNumber);
            anyFrameFailed = 1;
            break;
        }

        printf("\nCRC Check Result                    : OK  (remainder is zero / matches)\n");

        memcpy(reassembledBinary + reassembledLen, decoded.binaryData, decoded.binaryLen);
        reassembledLen += decoded.binaryLen;

        pos = endIndex;
    }

    if(frameNumber == 0)
    {
        printf("\nNo frames found in %s\n", FRAME_FILE);
        return;
    }

    if(anyFrameFailed)
    {
        printf("\n------------------------------------------------------------\n");
        printf("Transmission stopped due to a detected error - remaining frames,\n");
        printf("if any, were not processed.\n");
        printf("------------------------------------------------------------\n");
        return;
    }

    printf("\n------------------------------------------------------------\n");
    printf("Reassembled Binary Data (all frames combined)\n");
    printf("------------------------------------------------------------\n");
    printf("%.*s\n", reassembledLen, reassembledBinary);

    printf("\n------------------------------------------------------------\n");
    printf("Final Recovered Data\n");
    printf("------------------------------------------------------------\n");

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
    InputMode inputMode;

    srand((unsigned int)time(NULL));

    printf("\n==============================================");
    printf("\n   HDLC BIT-STUFFED FRAMING WITH CRC-16");
    printf("\n   (polynomial long division, frame-wise trace)");
    printf("\n==============================================\n");

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
            buildFrame(&frames[i]);
        }
    }

    printf("\nInput split into %d frame(s) of up to %d byte(s) each.\n", frameCount, frameSize);

    promptErrorInjection(frames, frameCount);

    runSender(frames, frameCount);

    runReceiver(inputMode);

    printf("\nProgram Terminated Successfully.\n");

    return 0;
}
