#include "head.h"

Frame *allocateFrames(int n)
{
    return (Frame *)malloc(n * sizeof(Frame));
}

void freeFrames(Frame *frames)
{
    free(frames);
}

/* str is treated as exactly 'len' bytes, no '\0' scanning */
int isValidBitString(const char str[], int len)
{
    int i;

    if(len <= 0)
    {
        return 0;
    }

    for(i = 0; i < len; i++)
    {
        if(str[i] != '0' && str[i] != '1')
        {
            return 0;
        }
    }

    return 1;
}

/* Returns number of bits written into binary[] */
int stringToBinary(const char input[], int inputLen, char binary[])
{
    int i, j;
    int ascii;
    int pos = 0;

    for(i = 0; i < inputLen; i++)
    {
        ascii = (unsigned char)input[i];

        for(j = 7; j >= 0; j--)
        {
            binary[pos++] = ((ascii >> j) & 1) ? '1' : '0';
        }
    }

    return pos;
}

/* Returns number of bytes written into output[] */
int binaryToString(const char binary[], int binaryLen, char output[])
{
    int i, j;
    int value;
    int index = 0;

    for(i = 0; i + 7 < binaryLen; i += 8)
    {
        value = 0;

        for(j = 0; j < 8; j++)
        {
            value = value * 2 + (binary[i + j] - '0');
        }

        output[index++] = (char)value;
    }

    return index;
}

/* Returns number of bits written into stuffed[] */
int bitStuff(const char input[], int inputLen, char stuffed[])
{
    int i;
    int j = 0;
    int count = 0;

    for(i = 0; i < inputLen; i++)
    {
        stuffed[j++] = input[i];

        if(input[i] == '1')
        {
            count++;

            if(count == 5)
            {
                stuffed[j++] = '0';
                count = 0;
            }
        }
        else
        {
            count = 0;
        }
    }

    return j;
}

/* Writes bit count of destuffed result into *originalLen */
Status bitDestuff(const char stuffed[], int stuffedLen, char original[], int *originalLen)
{
    int i;
    int j = 0;
    int count = 0;

    for(i = 0; i < stuffedLen; i++)
    {
        if(stuffed[i] == '0')
        {
            if(count == 5)
            {
                count = 0;
                continue;
            }

            original[j++] = stuffed[i];
            count = 0;
        }
        else
        {
            count++;

            if(count == 5 && i + 1 < stuffedLen && stuffed[i + 1] == '1')
            {
                return STATUS_CORRUPTED_DATA;
            }

            original[j++] = stuffed[i];
        }
    }

    *originalLen = j;

    return STATUS_OK;
}

/* Returns number of bits written into stuffed[] */
int byteStuff(const char input[], int inputLen, char stuffed[])
{
    int i, j = 0;

    for(i = 0; i + 7 < inputLen; i += 8)
    {
        if(memcmp(input + i, FLAG_BITS, FLAG_LEN) == 0)
        {
            memcpy(stuffed + j, ESCAPE_BITS, FLAG_LEN);
            j += FLAG_LEN;
            memcpy(stuffed + j, FLAG_BITS, FLAG_LEN);
            j += FLAG_LEN;
        }
        else if(memcmp(input + i, ESCAPE_BITS, FLAG_LEN) == 0)
        {
            memcpy(stuffed + j, ESCAPE_BITS, FLAG_LEN);
            j += FLAG_LEN;
            memcpy(stuffed + j, ESCAPE_BITS, FLAG_LEN);
            j += FLAG_LEN;
        }
        else
        {
            memcpy(stuffed + j, input + i, 8);
            j += 8;
        }
    }

    return j;
}

Status byteDestuff(const char stuffed[], int stuffedLen, char original[], int *originalLen)
{
    int i = 0;
    int j = 0;

    while(i + 7 < stuffedLen)
    {
        if(memcmp(stuffed + i, ESCAPE_BITS, FLAG_LEN) == 0)
        {
            i += FLAG_LEN;

            if(i + 7 >= stuffedLen)
            {
                return STATUS_CORRUPTED_DATA;
            }

            memcpy(original + j, stuffed + i, 8);
            j += 8;
            i += 8;
        }
        else
        {
            memcpy(original + j, stuffed + i, 8);
            j += 8;
            i += 8;
        }
    }

    *originalLen = j;

    return STATUS_OK;
}

int splitIntoFrames(const char fullBinary[], int totalLen, int frameSizeBits, Frame frames[], int maxFrames)
{
    int pos = 0;
    int count = 0;

    while(pos < totalLen && count < maxFrames)
    {
        int chunkLen = frameSizeBits;

        if(pos + chunkLen > totalLen)
        {
            chunkLen = totalLen - pos;
        }

        memcpy(frames[count].binaryData, fullBinary + pos, chunkLen);
        frames[count].binaryLen = chunkLen;

        memcpy(frames[count].address, ADDRESS_BITS, ADDR_LEN);
        memcpy(frames[count].control, CONTROL_BITS, CTRL_LEN);
        memcpy(frames[count].fcs, FCS_BITS, FCS_LEN);

        pos += chunkLen;
        count++;
    }

    return count;
}

Status buildFrame(Frame *frame, StuffMode stuffMode)
{
    int j = 0;

    if(stuffMode == STUFF_BIT)
    {
        frame->stuffedLen = bitStuff(frame->binaryData, frame->binaryLen, frame->stuffedData);
    }
    else
    {
        frame->stuffedLen = byteStuff(frame->binaryData, frame->binaryLen, frame->stuffedData);
    }

    /* Assemble finalFrame by fixed-length concatenation: flag, address,
       control, stuffed data, fcs, flag - no strcat, no '\0' anywhere */
    memcpy(frame->finalFrame + j, FLAG_BITS, FLAG_LEN);
    j += FLAG_LEN;

    memcpy(frame->finalFrame + j, frame->address, ADDR_LEN);
    j += ADDR_LEN;

    memcpy(frame->finalFrame + j, frame->control, CTRL_LEN);
    j += CTRL_LEN;

    memcpy(frame->finalFrame + j, frame->stuffedData, frame->stuffedLen);
    j += frame->stuffedLen;

    memcpy(frame->finalFrame + j, frame->fcs, FCS_LEN);
    j += FCS_LEN;

    memcpy(frame->finalFrame + j, FLAG_BITS, FLAG_LEN);
    j += FLAG_LEN;

    frame->finalLen = j;

    return STATUS_OK;
}

Status appendStreamToFile(const char filename[], const char bits[], int bitsLen)
{
    FILE *fp;

    fp = fopen(filename, "a");

    if(fp == NULL)
    {
        return STATUS_FILE_OPEN_FAIL;
    }

    fwrite(bits, 1, bitsLen, fp);

    fclose(fp);

    return STATUS_OK;
}

Status readEntireStreamFromFile(const char filename[], char bits[], int maxLen, int *outLen)
{
    FILE *fp;
    int c;
    int idx = 0;

    fp = fopen(filename, "r");

    if(fp == NULL)
    {
        return STATUS_FILE_OPEN_FAIL;
    }

    while((c = fgetc(fp)) != EOF && idx < maxLen)
    {
        if(c == '0' || c == '1')
        {
            bits[idx++] = (char)c;
        }
    }

    *outLen = idx;

    fclose(fp);

    return STATUS_OK;
}

/*
 * Frame layout per your spec:
 *   [start flag: 8][address: 8][control: 8][ data...(stuffed) ][fcs: 32][end flag: 8]
 *
 * Steps, exactly as requested:
 *   1. Check for the starting flag.
 *   2. Skip address + control together as one 16-bit block (by length,
 *      not by scanning for '\0').
 *   3. Traverse forward to find the end flag.
 *   4. Once end flag is found, subtract FCS_LEN (32 bits) from the space
 *      before it - remainder is the stuffed data.
 *   5. Continue scanning for the next start flag from there (handled by
 *      the caller's loop via *endIndex).
 */
Status extractNextFrame(const char stream[], int streamLen, int startIndex, int *endIndex,
                         StuffMode stuffMode, Decoded *decoded)
{
    int pos = startIndex;
    int i;
    int foundEnd = -1;
    Status destuffStatus;

    if(pos >= streamLen)
    {
        return STATUS_NO_MORE_FRAMES;
    }

    /* 1. Check for starting flag */
    if(pos + FLAG_LEN > streamLen || memcmp(stream + pos, FLAG_BITS, FLAG_LEN) != 0)
    {
        return STATUS_INVALID_START_FLAG;
    }

    pos += FLAG_LEN;

    /* 2. Skip address + control as one fixed 16-bit block */
    if(pos + ADDR_CTRL_LEN > streamLen)
    {
        return STATUS_FRAME_TOO_SHORT;
    }

    memcpy(decoded->address, stream + pos, ADDR_LEN);
    memcpy(decoded->control, stream + pos + ADDR_LEN, CTRL_LEN);
    pos += ADDR_CTRL_LEN;

    /* 3. Traverse to find the end flag */
    if(stuffMode == STUFF_BYTE)
    {
        i = pos;

        while(i + FLAG_LEN <= streamLen)
        {
            if(memcmp(stream + i, FLAG_BITS, FLAG_LEN) == 0)
            {
                foundEnd = i;
                break;
            }

            if(memcmp(stream + i, ESCAPE_BITS, FLAG_LEN) == 0)
            {
                i += 16;
            }
            else
            {
                i += 8;
            }
        }
    }
    else
    {
        for(i = pos; i + FLAG_LEN <= streamLen; i++)
        {
            if(memcmp(stream + i, FLAG_BITS, FLAG_LEN) == 0)
            {
                foundEnd = i;
                break;
            }
        }
    }

    if(foundEnd == -1)
    {
        return STATUS_NO_END_FLAG_FOUND;
    }

    /* 4. Subtract FCS_LEN from the space before the end flag; remainder is data */
    if(foundEnd - pos < FCS_LEN)
    {
        return STATUS_FRAME_TOO_SHORT;
    }

    {
        int stuffedLen = foundEnd - pos - FCS_LEN;

        memcpy(decoded->stuffedData, stream + pos, stuffedLen);
        decoded->stuffedLen = stuffedLen;

        memcpy(decoded->fcs, stream + pos + stuffedLen, FCS_LEN);
    }

    if(stuffMode == STUFF_BIT)
    {
        destuffStatus = bitDestuff(decoded->stuffedData, decoded->stuffedLen,
                                    decoded->binaryData, &decoded->binaryLen);
    }
    else
    {
        destuffStatus = byteDestuff(decoded->stuffedData, decoded->stuffedLen,
                                     decoded->binaryData, &decoded->binaryLen);
    }

    if(destuffStatus != STATUS_OK)
    {
        return destuffStatus;
    }

    /* endIndex points past the end flag, so the next call resumes scanning
       for the next start flag right after it */
    *endIndex = foundEnd + FLAG_LEN;

    return STATUS_OK;
}
