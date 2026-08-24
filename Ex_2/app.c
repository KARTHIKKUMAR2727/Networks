#include "head.h"
int main()
{
    struct Frame frame;
    int choice;
    char fileName[100];
    initializeFrame(&frame);
    printf("\nEnter Input File Name : ");
    scanf("%s",fileName);
    readFile(fileName,&frame);
    if(frame.dataLength==0)
    {
        printf("\nNo Data Found in File.\n");
        free(frame.data);
        return 0;
    }
    while(1)
    {
        printf("\nMenu");
        printf("\n1.2-D Parity Check");
        printf("\n2.Checksum");
        printf("\n3.Exit");
        printf("\n\nEnter Your Choice : ");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:
                twoDParitySender(frame.data);
                twoDParityReceiver();
                break;
            case 2:
                checksumSender(frame.data);
                checksumReceiver();
                break;
            case 3:
                printf("\nProgram Terminated.\n");
                free(frame.data);
                return 0;
            default:
                printf("\nInvalid Choice.\n");
        }
    }
    return 0;
}

