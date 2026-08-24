#include "head.h"

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

void polynomialToString(const char bits[], int len, char out[])
{
    int i;
    int degree;
    int wrote = 0;
    int pos = 0;

    for(i = 0; i < len; i++)
    {
        if(bits[i] == '1')
        {
            degree = len - 1 - i;

            if(wrote)
            {
                pos += sprintf(out + pos, " + ");
            }

            if(degree == 0)
            {
                pos += sprintf(out + pos, "1");
            }
            else if(degree == 1)
            {
                pos += sprintf(out + pos, "x");
            }
            else
            {
                pos += sprintf(out + pos, "x^%d", degree);
            }

            wrote = 1;
        }
    }

    if(!wrote)
    {
        sprintf(out + pos, "0");
    }
}
void computeCRC16(char data[], int dataLen, char crcOut[CRC_LEN], char trace[])
{
    char *temp;
    int totalLen = dataLen + CRC_LEN;
    int i, j;
    int tpos = 0;
    char polyStr[POLY_STR_LEN];
    char dividendStr[POLY_STR_LEN];

    temp = (char *)malloc(totalLen);

    memcpy(temp, data, dataLen);
    memset(temp + dataLen, '0', CRC_LEN);

    if(trace != NULL)
    {
        polynomialToString(CRC16_POLY, CRC16_POLY_LEN, polyStr);
        polynomialToString(temp, totalLen, dividendStr);

        tpos += sprintf(trace + tpos,
            "  Generator polynomial : %s\n"
            "                          (bit pattern %s, degree %d)\n"
            "  Message padded with %d zero bits (for a degree-%d generator):\n"
            "    %.*s\n"
            "  Dividend polynomial  : %s\n\n"
            "  --- GF(2) long division (XOR at every '1' leading bit) ---\n",
            polyStr, CRC16_POLY, CRC16_POLY_LEN - 1,
            CRC_LEN, CRC_LEN,
            totalLen, temp,
            dividendStr);
    }

    for(i = 0; i < dataLen; i++)
    {
        if(temp[i] == '1')
        {
            if(trace != NULL)
            {
                tpos += sprintf(trace + tpos,
                    "  step %3d: leading bit=1 at position %d -> XOR with generator\n"
                    "            before: %.*s\n",
                    i + 1, i, totalLen - i, temp + i);
            }

            for(j = 0; j < CRC16_POLY_LEN; j++)
            {
                temp[i + j] = (((temp[i + j] - '0') ^ (CRC16_POLY[j] - '0')) ? '1' : '0');
            }

            if(trace != NULL)
            {
                tpos += sprintf(trace + tpos,
                    "            after : %.*s\n",
                    totalLen - i, temp + i);
            }
        }
        else if(trace != NULL)
        {
            tpos += sprintf(trace + tpos,
                "  step %3d: leading bit=0 at position %d -> no XOR, shift only\n",
                i + 1, i);
        }
    }

    memcpy(crcOut, temp + dataLen, CRC_LEN);

    if(trace != NULL)
    {
        sprintf(trace + tpos, "\n  Remainder (this is the FCS/CRC) : %.*s\n", CRC_LEN, crcOut);
    }

    free(temp);
}

int verifyCRC16(char data[], int dataLen, const char receivedCrc[CRC_LEN], char trace[])
{
    char computed[CRC_LEN];

    computeCRC16(data, dataLen, computed, trace);

    return (memcmp(computed, receivedCrc, CRC_LEN) == 0) ? 1 : 0;
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
        memset(frames[count].fcs, '0', CRC_LEN);
        frames[count].fcsLen = CRC_LEN;

        pos += chunkLen;
        count++;
    }

    return count;
}

Status buildFrame(Frame *frame)
{
    int j = 0;

    computeCRC16(frame->binaryData, frame->binaryLen, frame->fcs, frame->remainderTrace);
    frame->fcsLen = CRC_LEN;

    frame->stuffedLen = bitStuff(frame->binaryData, frame->binaryLen, frame->stuffedData);

    memcpy(frame->finalFrame + j, FLAG_BITS, FLAG_LEN);
    j += FLAG_LEN;

    memcpy(frame->finalFrame + j, frame->address, ADDR_LEN);
    j += ADDR_LEN;

    memcpy(frame->finalFrame + j, frame->control, CTRL_LEN);
    j += CTRL_LEN;

    memcpy(frame->finalFrame + j, frame->stuffedData, frame->stuffedLen);
    j += frame->stuffedLen;

    memcpy(frame->finalFrame + j, frame->fcs, frame->fcsLen);
    j += frame->fcsLen;

    memcpy(frame->finalFrame + j, FLAG_BITS, FLAG_LEN);
    j += FLAG_LEN;

    frame->finalLen = j;

    return STATUS_OK;
}

void injectSingleBitError(char frameBits[], int frameLen, int bitPosition)
{
    int target;
    int low  = FLAG_LEN;
    int high = frameLen - FLAG_LEN - 1;

    if(bitPosition >= low && bitPosition <= high)
    {
        target = bitPosition;
    }
    else
    {
        target = low + (rand() % (high - low + 1));
    }

    frameBits[target] = (frameBits[target] == '0') ? '1' : '0';
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

Status extractNextFrame(const char stream[], int streamLen, int startIndex, int *endIndex,
                         Decoded *decoded)
{
    int pos = startIndex;
    int i;
    int foundEnd = -1;
    Status destuffStatus;

    if(pos >= streamLen)
    {
        return STATUS_NO_MORE_FRAMES;
    }

    if(pos + FLAG_LEN > streamLen || memcmp(stream + pos, FLAG_BITS, FLAG_LEN) != 0)
    {
        return STATUS_INVALID_START_FLAG;
    }

    pos += FLAG_LEN;

    if(pos + ADDR_CTRL_LEN > streamLen)
    {
        return STATUS_FRAME_TOO_SHORT;
    }

    memcpy(decoded->address, stream + pos, ADDR_LEN);
    memcpy(decoded->control, stream + pos + ADDR_LEN, CTRL_LEN);
    pos += ADDR_CTRL_LEN;

    for(i = pos; i + FLAG_LEN <= streamLen; i++)
    {
        if(memcmp(stream + i, FLAG_BITS, FLAG_LEN) == 0)
        {
            foundEnd = i;
            break;
        }
    }

    if(foundEnd == -1)
    {
        return STATUS_NO_END_FLAG_FOUND;
    }

    if(foundEnd - pos < CRC_LEN)
    {
        return STATUS_FRAME_TOO_SHORT;
    }

    {
        int stuffedLen = foundEnd - pos - CRC_LEN;

        memcpy(decoded->stuffedData, stream + pos, stuffedLen);
        decoded->stuffedLen = stuffedLen;

        memcpy(decoded->fcs, stream + pos + stuffedLen, CRC_LEN);
        decoded->fcsLen = CRC_LEN;
    }

    destuffStatus = bitDestuff(decoded->stuffedData, decoded->stuffedLen,
                                decoded->binaryData, &decoded->binaryLen);

    if(destuffStatus != STATUS_OK)
    {
        return destuffStatus;
    }

    if(!verifyCRC16(decoded->binaryData, decoded->binaryLen, decoded->fcs, NULL))
    {
        return STATUS_CRC_MISMATCH;
    }

    *endIndex = foundEnd + FLAG_LEN;

    return STATUS_OK;
}
