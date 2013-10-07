//////////////////////////////////////////////////////////////////////////////
//****************************************************************************
//
//    FILE NAME: server.c
//
//    DECSRIPTION: Bugsy Inc. Distributed Systems Logger (Daisy-L) Server 
//
//    OPERATING SYSTEM: Linux UNIX only
//    TESTED ON:
//
//    CHANGE ACTIVITY:
//    Date        Who      Description
//    ==========  =======  ===============
//    09-11-2013  Rajath   Initial creation
//
//****************************************************************************
//////////////////////////////////////////////////////////////////////////////

/*
 * Header files
 */
    
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>

/*
 * Macros
 */

#define MY_PORT                3493
#define ZERO                   0
#define ERROR                  -1
#define LISTENNUM              5
#define NO_OF_ARGS             3
#define MINI_BUF_SZ            15
#define SMALL_BUF_SZ           100
#define MED_BUF_SZ             1024
#define FOUR_K_BUF_SZ          4096
#define EIGHT_K_BUF_SZ         8192 
#define IP_ADDR                "192.17.11.78"
#define HOME_DIR               "/home/sbrmnym2/"
#define RESULT_FILE_NAME       "result"
#define REDIRECTION            " > "

/*
 * Function Declarations
 */

void *serverThreadFunction(void *);

/********************************************************************
 * NAME: main
 *
 * DESCRIPTION: Main function of The Bugsy Inc. DLQ Server code.
 *              This is a MULTITHREADED CONCURRENT SERVER. Main has
 *              logic to open a socket, bind and listen with queue 
 *              length of 5. For each DLQ client connection that is
 *              accepted it spawns a new thread that takes care of
 *              serving that DLQ client.  
 *              
 * PARAMETERS: NONE
 * 
 * RETURN:
 * (int) ZERO if success
 *       ERROR otherwise
 * 
 ********************************************************************/
int main(int argc, char *argv[])
{
  
    int sd,                                  // Socket Descriptor
	rc;                                  // return code 
    
    char buffer[SMALL_BUF_SZ],               // Buffer
	 hostName[SMALL_BUF_SZ],             // Host name
         ipAddressBuffer[SMALL_BUF_SZ];      // Buffer to hold IP address
    
    socklen_t len;                           // length
    
    struct sockaddr_in serverAddress,        // Server Address
                       clientAddress;        // Client Address

    pthread_t thread;                        // pthread

    // Command line argument check
    if ( argc != NO_OF_ARGS )
    {
        printf("\nINVALID USAGE\n");
        printf("\nUSAGE: %s <PORT_NO> <IP_ADDRESS>\n", argv[0]);
        return ERROR;
    }
    
    // Open a socket
    sd = socket(AF_INET, SOCK_STREAM, ZERO);
    if ( sd < 0 ) 
    {
        printf("\nError while creating socket\n");
        printf("\nExiting.... ... .. .\n");
        return ERROR;
    }

    // Init server info
    memset(&serverAddress, ZERO, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
    // Port no
    //serverAddress.sin_port = htons(MY_PORT);
    serverAddress.sin_port = htons(atoi(argv[1]));
    // Address
    //sprintf(ipAddressBuffer, "%s", IP_ADDR);
    sprintf(ipAddressBuffer, "%s", argv[2]);
    serverAddress.sin_addr.s_addr = inet_addr(ipAddressBuffer);
    // Zero the rest of the struct
    memset(&(serverAddress.sin_zero), '\0', 8);

    // Bind the socket
    rc = bind(sd, (struct sockaddr*)&serverAddress, sizeof(struct sockaddr));
    if ( ERROR == rc )
    {
	    printf("\nBind fails with error no = %d\n", errno);
	    printf("\nExiting.... ... .. .\n");
	    return ERROR;
    }

    // Listen on port
    listen(sd, LISTENNUM);

    // Beginning of the Main Server Processing Loop
    for(;;)
    {
	    int *ptrSd,                            // Pointer to socket descriptor 
                clientFd;                          // Socket descriptor 		  
        
        ptrSd = (int *)malloc(sizeof(int));

        printf("\n\t\t************************************************************************");
        printf("\n\t\t I am Bugsy Inc. Multithreaded Systems Logger (Daisy-L) .... ... .. . . .");
        printf("\n\t\t************************************************************************\n");
        len = sizeof(clientAddress);
        
        // Accept connection request from client
        clientFd = accept(sd, (struct sockaddr *) &clientAddress, &len);
        if ( clientFd < ZERO )
	{
    	    if ( EINTR == errno )
	    {
	        printf("\nInterrupted system call ??\n"); 
                continue;
            } // End of if ( EINTR == errno )
        } // End of if ( clientFd < ZERO )

        *ptrSd = clientFd;

        printf("\n\t\t\t==========================================");
        printf("\n\t\t\tDLQ Client Information\n");
        printf("\n\t\t\tConnection from %s", inet_ntop(AF_INET, &clientAddress.sin_addr, buffer, sizeof(buffer)));
        printf("\n\t\t\t==========================================\n");
         
        // Spawn a thread
        rc = pthread_create(&thread, NULL, serverThreadFunction, (void *)ptrSd);
        if ( rc != ZERO )
	{
            printf("\nThread creation failed\n");
            printf("\nError code: %d\n", rc);
            printf("\nConnection failed to %s\n", inet_ntop(AF_INET, &clientAddress.sin_addr, buffer, sizeof(buffer)));
            printf("\nExiting.... ... .. .\n");
            return ERROR;
        }

	} // End of for(;;)

       close(sd);

} // End of main()

/********************************************************************
 * NAME: serverThreadFunction
 *
 * DESCRIPTION: Function invoked by the thread spawned by the main
 *              function thread. 
 *
 * LOG FILE DESIGN: - Key Value pair
 *                  - Format is:
 *                    "Key := Value":
 *                  - "Types of Keys" are:
 *                    -> INFO
 *                    -> WARNING
 *                    -> ERROR
 *                    -> SEVERE
 *                  - "Value" is a sentence describing an event
 *
 * PARAMETERS:
 * (void *) clientFd - Socket descriptor
 *
 * RETURN: NONE
 *
 ********************************************************************/
void * serverThreadFunction(
                            void *clientFd       // Socket descriptor
			   )
{

    int numOfBytesRecv,                          // Number of bytes received
	numOfBytesSent,                          // Number of bytes sent
	rc,                                      // Return code
	byteCount = 0,                           // Byte counter
    	*ptr = (int *)clientFd,                  // Pointer to socket descriptor
	clientFdNew = *ptr;                      // Socket descriptor

    char clientCommand[SMALL_BUF_SZ],            // Buffer containing command passed by the client
	 logFileLocation[SMALL_BUF_SZ],          // Buffer containing log file location
	 resultFileName[SMALL_BUF_SZ],           // Buffer containing result file name
	 buffer[SMALL_BUF_SZ],                   // Buffer
	 singleByte,                             // Single byte buffer
 	 *resultToBeSentToClient;                // Result buffer to be sent to client 

    pid_t pid = getpid();                        // Process id

    pthread_t tid = pthread_self();              // Thread id

    FILE *fp;                                    // File pointer 

    struct timeval start,                        // Timer - start
                   end;                          // Timer - end

    long timeDiff;                               // Time difference

    // Detach this thread  
    //pthread_detach(pthread_self());

    //////////////////////////////////////////
    // STEP 1: Receive command from the client
    //////////////////////////////////////////

    // The client itself constructs the command. So receive the buffer 
    // and execute it as it is

    // Start the timer 
    gettimeofday(&start,NULL);
    printf("\nStart time (secs): %ld\n", start.tv_sec);
    printf("\nStart time (usecs): %ld\n", start.tv_usec);

    numOfBytesRecv = recv(clientFdNew, clientCommand, SMALL_BUF_SZ, ZERO);
    if ( ZERO == numOfBytesRecv )
    {
        printf("\nConnection closed by client. Zero bytes received\n");
        printf("\nExiting this thread.... ... .. . .\n");
	pthread_exit(NULL);
    }

    // TODO: make log file location dynamic
    // Prepare log file location 
    // sprintf(logFileLocation, "%s%s", HOME_DIR, LOG_FILE_NAME);
    
    /////////////////////////////////////////////////     
    // STEP 2: Execute the command sent by the client
    /////////////////////////////////////////////////

    // 1) Prepare a random file name containing the results of the command passed by the client
    sprintf(resultFileName, "%s%s.%u.%u", HOME_DIR, RESULT_FILE_NAME, pid, tid);
    // 2) Prepare the final command to be run i.e. $"command" > "resultFileName" 
    strcat(clientCommand, REDIRECTION);
    strcat(clientCommand, resultFileName);    
    // For Debugging. Uncomment only if required
    printf("\nFinal command: %s\n", clientCommand);
    // 3) Execute the command 
    rc = system(clientCommand);
    if ( rc != ZERO )
    {
        printf("\nExecution of grep command from client failed with error code = %d and return code = %d\n", errno, rc);
        printf("\nExiting this thread.... ... .. . .\n");
        // Close the socket descriptor. Error handling done by client
        close(clientFdNew);
	pthread_exit(NULL);
    }

    // Debug. Uncomment if required
    printf("\nRC of the grep command: %d\n", rc);

    //////////////////////////////////////////////
    // STEP 3: Send the results back to the client
    //////////////////////////////////////////////
    
    // Open the file in read mode
    // Debug. Uncomment if required
    printf("\nResults file name to be opened: %s\n", resultFileName);
    fp = fopen(resultFileName, "r");
    if ( NULL == fp )
    {
        printf("\nUnable to open %s; Error code: %d \n", resultFileName, errno);
        printf("\nExiting this thread.... ... .. . .\n");
        // Close the socket descriptor. Error handling done by client
        close(clientFdNew);
        pthread_exit(NULL);
    }    

    // Send the results in blocks of 4K
    // Allocate memory to the buffer to be sent to Client
    // Results are sent in blocks of 4K
    resultToBeSentToClient = (char *)malloc((sizeof(char))*EIGHT_K_BUF_SZ);
    char *counter = resultToBeSentToClient;
    while ( (singleByte = fgetc(fp)) != EOF )
    {
        *counter = singleByte;
        counter++;
        byteCount++;
        if ( EIGHT_K_BUF_SZ == byteCount )
        {
	    numOfBytesSent = send(clientFdNew, resultToBeSentToClient, EIGHT_K_BUF_SZ, ZERO);
            if ( ZERO == numOfBytesSent )
	    {
                printf("\nZero bytes sent\n");
                printf("\nExiting this thread.... ... .. . .\n");
                // Close the socket descriptor. Error handling done by client
                close(clientFdNew);
	        pthread_exit(NULL);
	    } // End of if ( ZERO == numOfBytesSent )
            memset(resultToBeSentToClient, '\0', EIGHT_K_BUF_SZ);
            byteCount = 0;
            counter = resultToBeSentToClient;
	} // End of if ( FOUR_K_BUF_SZ == byteCount )
    } // End of while ( (singleByte = fgetc(fp)) != EOF )
    // If the results file contained less than 4K then we have to send now
    if (strlen(resultToBeSentToClient))
    {
        numOfBytesSent = send(clientFdNew, resultToBeSentToClient, byteCount, ZERO);
        if ( ZERO == numOfBytesSent )
	{
	    printf("\nZero bytes sent\n");
	    printf("\Exiting this thread.... ... .. . .\n");
            // Close the socket descriptor. Error handling done by client
            close(clientFdNew);
	    pthread_exit(NULL);
	} // End of if ( ZERO == numOfBytesSent )
    }
    
    memset(&start, 0, sizeof(struct timeval));
    // End the timer 
    gettimeofday(&end ,NULL);
    printf("\nEnd time (secs): %ld\n", end.tv_sec);
    printf("\nEnd time (usecs): %ld\n", end.tv_usec); 

    // Debug
    printf("\nExiting thread function\n");
    // Close the socket\
    // Free heap memory
    // Exit thread
    close(clientFdNew);
    free(resultToBeSentToClient);
    pthread_exit(NULL);

} // End of serverThreadFunction() 
