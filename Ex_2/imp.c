#include "head.h"
void initializeFrame(struct Frame *frame)
{
    frame->data = NULL;
    frame->dataLength = 0;
}
void readFile(char fileName[], struct Frame *frame)
{
    FILE *fp;
    char ch;
    int size = 1;
    fp = fopen(fileName,"r");
    if(fp == NULL)
    {
        printf("\nUnable to Open File.\n");
        exit(1);
    }
    frame->data = (char *)malloc(sizeof(char));
    if(frame->data == NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    while((ch = fgetc(fp)) != EOF)
    {
        if(ch != '\n' && ch != ' ')
        {
            frame->data = (char *)realloc(frame->data,sizeof(char)*(size+1));
            if(frame->data == NULL)
            {
                printf("\nMemory Allocation Failed.\n");
                exit(1);
            }
            frame->data[size-1]=ch;
            size++;
        }
    }
    frame->data[size-1]='\0';
    frame->dataLength=size-1;
    fclose(fp);
}
void writeFile(char fileName[], char data[])
{
    FILE *fp;
    fp=fopen(fileName,"w");
    if(fp==NULL)
    {
        printf("\nUnable to Create File.\n");
        exit(1);
    }
    fputs(data,fp);
    fclose(fp);
    printf("\nData Written Successfully to %s\n",fileName);
}
void twoDParitySender(char data[])
{
    int i,j;
    int cols=7;
    int length;
    int rows;
    int **matrix;
    int *rowParity;
    int *colParity;
    FILE *fp;
    length=strlen(data);
    rows=(length+cols-1)/cols;
    matrix=(int **)malloc(rows*sizeof(int *));
    if(matrix==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    for(i=0;i<rows;i++)
    {
        matrix[i]=(int *)malloc(cols*sizeof(int));
        if(matrix[i]==NULL)
        {
            printf("\nMemory Allocation Failed.\n");
            exit(1);
        }
    }
    rowParity=(int *)malloc(rows*sizeof(int));
    colParity=(int *)malloc(cols*sizeof(int));
    if(rowParity==NULL || colParity==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    int k=0;
    for(i=0;i<rows;i++)
    {
        for(j=0;j<cols;j++)
        {
            if(k<length)
            {
                matrix[i][j]=data[k]-'0';
                k++;
            }
            else
            {
                matrix[i][j]=0;
            }
        }
    }
    for(j=0;j<cols;j++)
    {
        colParity[j]=0;
    }
    printf("\nSENDER SIDE");
    printf("\nOriginal Data\n");
    for(i=0;i<rows;i++)
    {
        for(j=0;j<cols;j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("\n");
    }
    for(i=0;i<rows;i++)
    {
    	int count=0;
	for(j=0;j<cols;j++){
		if(matrix[i][j]==1){
	    		count++;
			}
    	}
	if(count % 2 == 0)
    	{
        	rowParity[i]=1;
    	}
    	else
    	{
        	rowParity[i]=0;
    	}
    }
    for(j=0;j<cols;j++)
    {
        int count=0;
        for(i=0;i<rows;i++)
        {
            if(matrix[i][j]==1)
            {
               count++;
            }
        }
        if(count % 2 == 0)
        {
            colParity[j]=1;
        }
        else
        {
            colParity[j]=0;
        }
    }
    printf("\nData With Row Parity\n\n");
    for(i=0;i<rows;i++)
    {
        for(j=0;j<cols;j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("| %d\n",rowParity[i]);
    }
    printf(" ");
    for(j=0;j<cols;j++)
    {
        printf("%d ",colParity[j]);
    }
    printf("\n");
    fp=fopen("transmission.txt","w");
    if(fp==NULL)
    {
        printf("\nUnable to Create transmission.txt\n");
        exit(1);
    }
    for(i=0;i<rows;i++)
    {
        for(j=0;j<cols;j++)
        {
            fprintf(fp,"%d",matrix[i][j]);
        }
        fprintf(fp,"%d",rowParity[i]);
    }
    for(j=0;j<cols;j++)
    {
        fprintf(fp,"%d",colParity[j]);
    }
    fclose(fp);
    printf("\nTransmission File Created Successfully.\n");
    for(i=0;i<rows;i++)
    {
        free(matrix[i]);
    }
    free(matrix);
    free(rowParity);
    free(colParity);
}
void twoDParityReceiver(void)
{
    FILE *fp;
    int i,j;
    int cols=7;
    int rows;
    int length;
    char *received;
    int **matrix;
    int *rowParity;
    int *colParity;
    int *calculatedRowParity;
    int *calculatedColParity;
    int errorRow=-1;
    int errorCol=-1;
    received=(char *)malloc(sizeof(char)*10000);
    if(received==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    fp=fopen("transmission.txt","r");
    if(fp==NULL)
    {
        printf("\nUnable to Open transmission.txt\n");
        free(received);
        exit(1);
    }
    fscanf(fp,"%s",received);
    fclose(fp);
    length=strlen(received);
    rows=(length-cols)/9;
    matrix=(int **)malloc(rows*sizeof(int *));
    if(matrix==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        free(received);
        exit(1);
    }
    for(i=0;i<rows;i++)
    {
        matrix[i]=(int *)malloc(cols*sizeof(int));
        if(matrix[i]==NULL)
        {
            printf("\nMemory Allocation Failed.\n");
            exit(1);
        }
    }
    rowParity=(int *)malloc(rows*sizeof(int));
    calculatedRowParity=(int *)malloc(rows*sizeof(int));
    colParity=(int *)malloc(cols*sizeof(int));
    calculatedColParity=(int *)malloc(cols*sizeof(int));
    if(rowParity==NULL ||
       calculatedRowParity==NULL ||
       colParity==NULL ||
       calculatedColParity==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    int k=0;
    for(i=0;i<rows;i++)
    {
        for(j=0;j<cols;j++)
        {
            matrix[i][j]=received[k]-'0';
            k++;
        }
        rowParity[i]=received[k]-'0';
        k++;
    }
    for(j=0;j<cols;j++)
    {
        colParity[j]=received[k]-'0';

        k++;
    }
    printf("\n RECEIVER SIDE");
    printf("\nReceived Data\n\n");
    for(i=0;i<rows;i++)
    {
        for(j=0;j<cols;j++)
        {
            printf("%d ",matrix[i][j]);
        }
        printf("| %d\n",rowParity[i]);
    }
    printf(" ");
    for(j=0;j<cols;j++)
    {
        printf("%d ",colParity[j]);
    }
    printf("\n\n");
    for(i=0;i<rows;i++)
    {
        int count=0;
        for(j=0;j<cols;j++)
        {
            if(matrix[i][j]==1)
                count++;
        }
        calculatedRowParity[i]=count%2;
    }
    for(j=0;j<cols;j++)
    {
        int count=0;
        for(i=0;i<rows;i++)
        {
            if(matrix[i][j]==1)
                count++;
        }
        calculatedColParity[j]=count%2;
    }
    for(i=0;i<rows;i++)
    {
        if(calculatedRowParity[i]!=rowParity[i])
        {
            errorRow=i;
            break;
        }
    }
    for(j=0;j<cols;j++)
    {
        if(calculatedColParity[j]!=colParity[j])
        {
            errorCol=j;
            break;
        }
    }
    if(errorRow==-1 && errorCol==-1)
    {
        printf("\nNo Error Detected.\n");
    }
    else if(errorRow!=-1 && errorCol!=-1)
    {
        printf("\nError Detected.\n");
        printf("\nError Bit Position\n");
        printf("Row    : %d\n",errorRow+1);
        printf("Column : %d\n",errorCol+1);
        if(matrix[errorRow][errorCol]==0)
            matrix[errorRow][errorCol]=1;

        else
            matrix[errorRow][errorCol]=0;
        printf("\nCorrected Data\n\n");
        for(i=0;i<rows;i++)
        {
            for(j=0;j<cols;j++)
            {
                printf("%d ",matrix[i][j]);
            }
            printf("\n");
        }
    }
    else
    {
        printf("\nMultiple Bit Error Detected.\n");
    }
    for(i=0;i<rows;i++)
    {
        free(matrix[i]);
    }
    free(matrix);
    free(rowParity);
    free(calculatedRowParity);
    free(colParity);
    free(calculatedColParity);
    free(received);
}
void checksumSender(char data[])
{
    FILE *fp;
    int i,j;
    int length;
    int rows;
    int *block;
    int sum=0;
    int carry;
    int checksum;
    length=strlen(data);
    rows=(length+K-1)/K;
    block=(int *)malloc(rows*sizeof(int));
    if(block==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    printf("\nSENDER SIDE");
    printf("\nData Blocks\n\n");
    for(i=0;i<rows;i++)
    {
        block[i]=0;
        for(j=0;j<K;j++)
        {
            int pos=i*K+j;
            block[i]<<=1;
            if(pos<length)
            {
                if(data[pos]=='1')
                    block[i]|=1;
                printf("%c",data[pos]);
            }
            else
            {
                printf("0");
            }
        }
        printf("\n");
    }
    printf("\nBinary Addition\n");
    for(i=0;i<rows;i++)
    {
        sum+=block[i];
        while(sum>255)
        {
            carry=sum>>8;

            sum=(sum&255)+carry;
        }
    }
    printf("\nSum : ");
    for(i=7;i>0;i--)
    {
        printf("%d",(sum>>i)&1);
    }
    checksum=(~sum)&255;
    printf("\nChecksum : ");
    for(i=7;i>0;i--)
    {
        printf("%d",(checksum>>i)&1);
    }
    fp=fopen("transmission.txt","w");
    if(fp==NULL)
    {
        printf("\nUnable to Create transmission.txt\n");
        free(block);
        exit(1);
    }
    fprintf(fp,"%s",data);
    for(i=7;i>0;i--)
    {
        fprintf(fp,"%d",(checksum>>i)&1);
    }
    fclose(fp);
    printf("\n\nTransmission File Created Successfully.\n");
    free(block);
}
void checksumReceiver(void)
{
    FILE *fp;
    char *received;
    char *data;
    char *checksumBits;
    int length;
    int dataLength;
    int rows;
    int *block;
    int receivedChecksum=0;
    int i,j;
    int sum=0;
    int carry;
    received=(char *)malloc(10000*sizeof(char));
    checksumBits=(char *)malloc((K+1)*sizeof(char));
    if(received==NULL || checksumBits==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    fp=fopen("transmission.txt","r");

    if(fp==NULL)
    {
        printf("\nUnable to Open transmission.txt\n");
        free(received);
        free(checksumBits);
        exit(1);
    }
    fscanf(fp,"%s",received);
    fclose(fp);
    length=strlen(received);
    dataLength=length-K;
    data=(char *)malloc((dataLength+1)*sizeof(char));
    if(data==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    strncpy(data,received,dataLength);
    data[dataLength]='\0';
    strcpy(checksumBits,received+dataLength);
    rows=(dataLength+K-1)/K;
    block=(int *)malloc(rows*sizeof(int));
    if(block==NULL)
    {
        printf("\nMemory Allocation Failed.\n");
        exit(1);
    }
    printf("\nRECEIVER SIDE\n");
    printf("\nReceived Data Blocks\n\n");

    for(i=0;i<rows;i++)
    {
        block[i]=0;
        for(j=0;j<K;j++)
        {
            int pos=i*K+j;
            block[i]<<=1;
            if(pos<dataLength)
            {
                if(data[pos]=='1')
                    block[i]|=1;
                printf("%c",data[pos]);
            }
            else
            {
                printf("0");
            }
        }
        printf("\n");
    }
    printf("\nReceived Checksum : %s\n",checksumBits);
    for(i=0;i<K;i++)
    {
        receivedChecksum<<=1;
        if(checksumBits[i]=='1')
            receivedChecksum|=1;
    }
    for(i=0;i<rows;i++)
    {
        sum+=block[i];
        while(sum>255)
        {
            carry=sum>>8;
            sum=(sum&255)+carry;
        }
    }
    sum+=receivedChecksum;
    while(sum>255)
    {
        carry=sum>>8;
        sum=(sum&255)+carry;
    }
    printf("\nFinal Sum : ");
    for(i=7;i>0;i--)
    {
        printf("%d",(sum>>i)&1);
    }
    sum=(~sum)&255;
    printf("\nOne's Complement : ");
    for(i=7;i>0;i--)
    {
        printf("%d",(sum>>i)&1);
    }
    if(sum==0)
    {
        printf("\n\nNo Error Detected.\n");
    }
    else
    {
        printf("\n\nError Detected.\n");
    }
    free(received);
    free(data);
    free(checksumBits);
    free(block);
}


