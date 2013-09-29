///////////////////////////////////////////////////////////////////////////////////////////////////////
//*****************************************************************************************************
//
//     File Name: Client.c
//     Description: Bugsy Inc. Distributed Log Query (DLQ) Client
//
//     OPERATING SYSTEM: lINUX flavors only
//     Tested ON: EWS machines
//
//     CHANGE ACTIVITY:
//     Date	   Who      Description
//     ==========  =======  ===============	
//     09-11.2013  KARTHIK  Initial Creation
//******************************************************************************************************
//////////////////////////////////////////////////////////////////////////////////////////////////////// 

/*
 * Header files
 */
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>

/*
 * Function Declaration
 */

void thread_create(char *);
char* match_grep(int );

/*
 *  Macros
 */

#define IP_LENGTH       30
#define MAX_HOSTS       5
#define MAX_FILE_LENGTH 50
#define PORT            "3492"
#define ZERO            0
#define EIGHT_KB        8192
#define MAX_GREP_PATTERN_LEN 100
#define PORT_LENGTH     5
/* 
 * Global Variables
 */

char *port_no;
int n_hosts;
char ports[MAX_HOSTS][PORT_LENGTH];
char hosts[MAX_HOSTS][IP_LENGTH];
char file_names[MAX_HOSTS][MAX_FILE_LENGTH]={"MACHINE1_RESULTS.log","MACHINE2_RESULTS.log","MACHINE3_RESULTS.log","MACHINE4_RESULTS.log","MACHINE5_RESULTS.log"};

/*
 * Structure Definition Thread params
 */
struct thread_params 
  { 
    char *pattern;
    char *host_IP;
    char *port_no;
    char *file_name;
  };

/**************************************************************************************
 * Name : ClientThread								      *	
 * Description : This is a multi threaded program connecting to multiple servers.     *
 *               Each instance of the thread will execute this piece of code to create*
 *               socket, connet, send, recv data and write results to a file          *
 * PARAMETERS : Thread Parameters : grep pattern, IP and FILE name                    *
 * RETURN     :                                                                       *
 * (void *)                                                                           *
 *************************************************************************************/


void *ClientThread(void* params)
{
  struct thread_params *tparams;                     // Thread Params
  char *grep_pattern,                                // pointer holding grep pattern
       *ip_ptr,                                      // Pointer to the IP address
       *file_name,                                   // Pointer to the resultant file
       *port_no;				     // Pointer to the port number	
  int  len,                                          // used in various contexts
       ret,                                          // return value from connect
       socketfd,                                     // socket descriptor from socket API
       status;                                       // Status of getaddrinfo API

  
  struct addrinfo input_server_info;                 // To hold the input server info
  struct addrinfo *output_server_info;               // To hold the output server info 
                    
  tparams = (struct thread_params *)(params);        // get thread parameters
  grep_pattern = tparams->pattern;                   // get grep_pattern
  ip_ptr = tparams->host_IP;                         // initialise IP_PTR
  len=strlen(grep_pattern);                          // Initialise len of grep pattern
  file_name = tparams->file_name;                    // get the output file name 
  port_no   = tparams->port_no;                      // pointer to the port_no
  //Initialise Network parameters : input_server_info, output_server_info
  
  
  memset(&input_server_info,ZERO, sizeof(struct addrinfo));
  input_server_info.ai_family = AF_UNSPEC;
  input_server_info.ai_socktype = SOCK_STREAM;
  status = getaddrinfo(ip_ptr,port_no,&input_server_info,&output_server_info);
  //ERROR HANDLING
  if(status!=0){
       printf("Error Resolving host or invalid IP, ERRNO : %d\n",errno);
       goto closeconn;
  }
  socketfd = socket(output_server_info->ai_family, output_server_info->ai_socktype, output_server_info->ai_protocol);
  //EROR HANDLING
  if(status!=0){
       printf("\nUnable to create socket, ERRNO :%d\n",errno);
       goto closeconn;
  }
  ret=connect(socketfd, output_server_info->ai_addr, output_server_info->ai_addrlen);  
  printf("\n...Connecting to server : %s\n",ip_ptr);
  //ERROR HANDLING
  if(ret!=0){
       printf("\nUnable to connect to the server, possibly invalid IP, ERRNO:%d\n",errno);
       goto closeconn;
  }
  printf("\n...Sending information to search, to server:  %s\n",ip_ptr);
  printf("\n...grep_pattern : %s\n",grep_pattern);
  len = strlen(grep_pattern);
  len = send(socketfd,grep_pattern,len,0);
  if(len==-1 || len==0){
       printf("\nSend Fails\n");
       goto closeconn;
  }
  len = recieve_packet(socketfd,file_name,ip_ptr);
closeconn:
  close(socketfd);
  pthread_exit(NULL);
}

/*****************************************************************************************
 * Name        : recieve_packet								 *
 * Description : This function is to recieve the network packet from the networking stack.
 *               This also calculates the time taken for the whole file to be recieved.  *
 * Parameters  : socket descriptor and file name to which the recieved buffer needs to be*
 *               written								 *	
 * Return Value : int - returns the length of the recieved packet.	                 *
 * ***************************************************************************************/              

int recieve_packet(int socket_desc,char *fname, char *host_ip)
{
  struct timeval start,end;		// Define start and end to not the time taken to recieve the result
  int len;                              // To receive packet of legth 'len each time
  FILE *fptr;                           // FILE ptr to open to write the results
  int flag=0;                           // flag to know if we recieved something
  int sum=0;                            // total bytes recieved
  // start the clock 
  gettimeofday(&start,NULL);
 // printf("\nstart time (secs) %ld\n",start.tv_sec);
 // printf("\nstart time (usecs) %ld\n",start.tv_usec);
  memset(&start,ZERO,sizeof(struct timeval));
  // recieve buffer every time of len 8192
  fptr = fopen(fname, "wb");
  printf("\n...Recieving Search Results from %s\n",host_ip);
  char *buf=(char *)malloc(EIGHT_KB);

  while((len=recv(socket_desc,buf,EIGHT_KB,ZERO))!=0){
     if(len == -1){
      printf("\nError recieved ERRNO:%d",errno);
     }
     flag=1;
     sum=sum+EIGHT_KB;
     fprintf(fptr,"%s",buf);
     memset(buf,ZERO,EIGHT_KB);
  }
 
 if(len==-1 && flag==0){
        printf("\nDid not recieve any packer from the server, recv closed ERRNO :%d\n",errno);
        goto closerecv;
  }
 
 if(sum==0 && flag==0){
  	printf("\nDid not recieve any packet from the server, remote grep command failed \n");
        goto closerecv;
 }
 // end the clock
 gettimeofday(&start,NULL);
 //printf("\nstart time (secs) %ld\n",start.tv_sec);
 //printf("\nstart time (usecs) %ld\n",start.tv_usec);
  
 printf("\n...RESULTS FILE CREATED FOR SERVER_IP %s AT %s",host_ip,fname);
closerecv :
 //new line added
 free(buf);
 fclose(fptr);
}
/*******************************************************************************************
 * Name        : main									   *	
 * Description : this funtion initialises the IP addresses and assigns them to appropriate *
 *               host. It also prints out the options that are possible to the user for    *
 *               querying the log file from the remote hosts.				   *
 *											   *
 * Parameters : Argc and argv to pass the IP addresses as argumnts			   *
 * Return value : void									   *	
 * *****************************************************************************************/
void main(int argc, char* argv[])

{
  int input;			//input from the user
  char *grep_pattern;           //recieve the grep_pattern
  int i;                        //iterator
  char *str_sub=NULL;

  if(argc == 1){
     printf("\nUsage : ./a.out <ip1> <port1> <ip2> <port2> ...... <ipn> <portn>\n");
     printf("\ninvalid number of arguments\n");
     exit(0);
  }
  if((argc%2)==0){
     printf("\nUsage : ./a.out <ip1> <port1> <ip2> <port2> ...... <ipn> <portn>\n");
     printf("\ninvalid number of arguments entered\n");
     exit(0);
  }
   
  n_hosts = (argc-1)/2;
  
  int j=0;
  int k=0;
  // populate the hosts data structure from the arguments passed
  for (i=0;i<(argc-1);i++){  // iterate through all the parameters

     if(i%2==0){
	     char *temp_ip = hosts[j];
	     char *temp_ip1 = argv[i+1];
	     strcpy(temp_ip, temp_ip1);
	     printf("\nIP:%s ",hosts[j]);
             char *ptr= strstr(hosts[j],".");
             if(ptr==NULL){
                  printf("\nUsage : ./a.out <ip1> <port1> <ip2> <port2> ...... <ipn> <portn>\n");
                  printf("\nInvalid IP passed, doesn't look like a valid IP\n");
                  exit(0);
             }
	     j++;
     }
    if(i%2==1){
	     char *temp_port = ports[k];
	     char *temp_port1= argv[i+1];
             strcpy(temp_port, temp_port1);
             printf("PORT:%s\n",ports[k]);
             char *ptr= strstr(ports[j],".");
             if(ptr){
                  printf("\nUsage : ./a.out <ip1> <port1> <ip2> <port2> ...... <ipn> <portn>\n");
                  printf("\nInvalid port passed, doesn't look like a port IP\n");
                  exit(0);
             }
             k++;
    }		

  }
  // Display the various available options for the user
  printf("**********************************************************************\n");
  printf("\n\nFollowing are the Different Keys of the messages :\n");
  printf("INFO \n");
  printf("ERROR \n");
  printf("WARNING \n");
  printf("SEVERE \n");
  printf("**********************************************************************\n");
  printf("*********************YOU HAVE THE FOLLOWING OPTIONS*******************\n");
  printf("\npress the folowing numbers to receive the corresponding output\n");
  printf(" 1 -> Number of occurrences of type INFO \n");
  printf(" 2 -> Number of occurrences of type ERROR \n");
  printf(" 3 -> Number of occurrences of type WARNING \n");
  printf(" 4 -> Number of occurrences of type SEVERE \n");
  
  printf("\n 5 -> Search for a particular string in all types of error messages\n");

  printf("\n 6 -> Display all the error messages corresponding to error type INFO\n");
  printf(" 7 -> Display all the error messages corresponding to error type ERROR\n");
  printf(" 8 -> Display all the error messages corresponding to error type WARNING\n");
  printf(" 9 -> Display all the error messages corresponding to error type SEVERE\n"); 

  printf(" 10 -> Enter the specific key that you want to search\n");
  printf(" 11 -> Enter the specific value that you want to search\n");
  printf("***********************************************************************\n");
  
  scanf("%d",&input);

  /* call match_grep to construct grep pattern */

  grep_pattern=match_grep(input);
    
  /* create threads filling in the appropriate thread params */
 
  thread_create(grep_pattern);
  free(grep_pattern);  
 }

/************************************************************************************************
 * Name : Thread_create										*
 * Description : Instantiate thread parameters based on the number of servers that this client  *
 * 		 needs to connect to. Create p_thread to connect every server and get the results.
 * Return    : Void										*	
 * **********************************************************************************************/

void thread_create(char* grep_pattern)
{ 
  int i;				// iterator
  pthread_t threadID[n_hosts];          // pthreads initialisation [array of thread IDs]
  struct thread_params *obj[n_hosts];   // thread_params for every thread object
  struct timeval start,end;
  gettimeofday(&start,NULL);
  printf("\nSTART time (secs) %ld\n",start.tv_sec);
  printf("\nSTART time (usecs) %ld\n",start.tv_usec);  
  // create client threads to connect to the servers
  for (i=0;i<n_hosts;i++) {
        obj[i]=(struct thread_params *)malloc(sizeof(struct thread_params));
        obj[i]->pattern = grep_pattern;
        obj[i]->host_IP = hosts[i];
        obj[i]->port_no = ports[i];
        obj[i]->file_name = file_names[i]; 
  	pthread_create(&threadID[i],NULL,ClientThread,obj[i]);
  }
  // Main proc waits for all the threads to complete
  for(i=0;i<n_hosts;i++){
      pthread_join(threadID[i],NULL);
  } 
  gettimeofday(&start,NULL);
  printf("\n\nEND time (secs) %ld\n",start.tv_sec);
  printf("\nEND time (usecs) %ld\n",start.tv_usec);

  // free all the thread objects
  for (i=0;i<n_hosts;i++) {
       free(obj[i]);
  }  
}
/*********************************************************************************
 * Name : match_grep								 *			
 * Description : This function matches the input to the appropriate grep pattern *
 * Return value: Returns character pointer. [pattern pointer]	                 *
 * ******************************************************************************/
char* match_grep(int input)						
{
  // match the grep pattern using a long SWTICH 
  char search_pattern[MAX_GREP_PATTERN_LEN];    	 //used for case 5 
  char *pattern=(char *)malloc(MAX_GREP_PATTERN_LEN);    //allocate memory to hold the grep pattern
 
  switch(input){

  	case 1 : strcpy(pattern,"grep 'INFO' ~/log.txt | wc -l ");
                 break;

        case 2 : strcpy(pattern,"grep 'ERROR' ~/log.txt | wc -l ");
                 break;
        
        case 3 : strcpy(pattern,"grep 'WARNING' ~/log.txt | wc -l ");
                 break;

        case 4 : strcpy(pattern,"grep 'SEVERE' ~/log.txt | wc -l ");
                 break;

        case 5 : printf("Enter the pattern you want to search\n");
                 scanf("%s",search_pattern);
                 fflush(stdin);
                 strcat(pattern,"grep '");
                 strcat(pattern,search_pattern);
                 strcat(pattern,"' ~/log.txt ");
                 break;
       
       case 6 : strcpy(pattern,"grep 'INFO' ~/log.txt ");
                break;

       case 7 : strcpy(pattern,"grep 'ERROR' ~/log.txt ");
                break;

       case 8 : strcpy(pattern,"grep 'WARNING' ~/log.txt ");
                break;
       
       case 9 : strcpy(pattern,"grep 'SEVERE' ~/log.txt ");
                break;
 
       case 10 :printf("Enter the specific Key you want to search\n");
                scanf("%s",search_pattern);
                fflush(stdin);
                strcat(pattern,"grep '^");
                strcat(pattern,search_pattern);
                strcat(pattern,"[ ]*:' ~/log.txt ");
                break;

       case 11 : printf("Enter the specific value you want to search\n");
                 scanf("%s",search_pattern);
                 fflush(stdin);
                 strcat(pattern,"grep '^.*:.*");
                 strcat(pattern,search_pattern);
                 strcat(pattern,"' ~/log.txt ");
                 break;   
       default : printf("wrong option entered..bye !\n");
                exit(0);
  }               
  return pattern;
}  
