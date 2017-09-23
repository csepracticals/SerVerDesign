#include <stdio.h>
#include <stdlib.h>
#define TRUE   1
#define FALSE  0
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#if 1
#define N 2
#define ip1 "192.168.20.1"
#define ip2 "192.168.20.2"
#endif
#define tablesize 100


struct node 
{
	int data;
	struct node *link;
};
int num=0;
#if 0
int N=2;
char ip1[20];
char ip2[20];
#endif
struct HTEntry
{
	int data;
	struct node *link;
	
};

struct HTEntry Htable[tablesize]; 



//----------------------hash table functions-----------------

void initialiseHashtable()
{

	int i=0;

	for(i=0;i<tablesize;i++)
	{

		Htable[i].data=0;
  		Htable[i].link=NULL;
 	}
}


void appendNode(struct node *start,int data)
{
	
	while(start->link!=NULL)
	{
		
		start=start->link;
	}
 	start->link = (struct node *)malloc(sizeof(struct node));
 	start=start->link;
 	start->link=NULL;
 	start->data = data;
}




int addToHashtable(int key,int data)
{
 
	int maxlimit_key = (tablesize-1)*N+num;
	int relativeIndex,returnvalue;

	if(key % N == num && key <= maxlimit_key && key > -1)
	{
		// Key must satisfy the eqn K % N = num then only its for current node

		relativeIndex = (key - num)/N;   
		
		// case 1 : if hash table entry is empty 
		
		if(Htable[relativeIndex].data==0)
		{

     			Htable[relativeIndex].data = data;	
     			returnvalue=1;
   		}
  
		
		// case 2 : if exactly one entry in particular entry of hashtable (Ist Collision)

   		else if(Htable[relativeIndex].data!=0 && Htable[relativeIndex].link==NULL)
		{

    			Htable[relativeIndex].link = (struct node *)malloc(sizeof(struct node));
    			Htable[relativeIndex].link->data = data;
    			Htable[relativeIndex].link->link = NULL;
    			returnvalue=1;
   		}

		// case 3: Subsequent Collisions

		else
		{
     			appendNode(Htable[relativeIndex].link,data);
      			returnvalue=1;
   		}

 		printf("\nRESULT: AT KEY : %d, VALUE INSERTED : %d IN HASH TABLE SUCCESS\nENTER NEW GET/PUT REQUEST :",key,data);
 	}
	
 	else
	{
  		printf("\nERROR:KEY = %d,VALUE = %d CANNOT ADD IN TABLE, MAX KEY LIMIT = %d\nENTER NEW GET/PUT 				REQUEST :",key,data,maxlimit_key);
  		
		returnvalue=0;
 	}
 	return returnvalue;
}



int fetchValueFromHT(int key)		//fun() to retrieve value corresponding to a key from hash table
{
	
	int relativeIndex = (key-num)/N,return_value;		//calculate index of key  in hash table

	if(Htable[relativeIndex].link == NULL)					//first value in the list
		return_value =  Htable[relativeIndex].data;

  	else
	{
    		struct node *start = Htable[relativeIndex].link;
     		while(start->link!=NULL)					
      			start = start->link;					// progess pointer to last element
     		return_value =  start->data;					//assign last element 
   	}   
	return return_value;
}


void displayHtable()
{

	int from_key = num,to_key = (tablesize-1)*N + num;

	if(from_key > to_key)
	{
		
		int temp = from_key;
		from_key = to_key;
		to_key = temp ;
	}

	printf("\n-----Hash Table Contents(%d--%d)------------\n",from_key,to_key);

	if(from_key % N == num && to_key % N == num)
	{

 		int i,from=(from_key - num)/N,to = (to_key - num)/N,fetchValue,key;

 		for(i=from;i<=to;i++)
		{

			key = i*N+num;
			fetchValue = fetchValueFromHT(key);
  			if(fetchValue !=0)
  				printf("\nkey : %d ===== value : %d\n",key,fetchValueFromHT(key));
 		}
	}
 	else
		printf("invalid keys , hash table cannot be displayed\n");

 	printf("-----------------------------------------------------\nENTER NEW GET/PUT REQUEST:");
}




//--------------------------------------- hash table functions ----------------------------------------------


struct machine_id 
{
	char *ip_address;
	int tcpportno;
	int udpportno;
}
node[20];


int forYou(int num,char request[])		//check whether request is for current node or not
{
	
	int k;
	if(getOrPut(request)==1)				//check whether request is get or put

		k = extractKeyFromGet(request);			
 	else
 		k=extractKeyFromPut(request);		

 	if((k % N) == num)			

 		return 1;					//return 1 if for current node
 	else
 		return 0;					//return 0 if not for current node
}


// ----------------------- String handling functions---------------------------------


int extractKeyFromPut(char request[])		//method for extracting key from put request.
{	
	
	char *b = strstr(request, "("),*c = strstr(request, ",");
	int position = b - request,position1 = c - request,k;
	char to[4];
	strncpy(to, request + position+1, position1 - position);   //for extracting the first integer no. k of put(k,x)
 
	k = atoi(to);		//converting to int
	fflush(stdout);
	return k;
}

int extractKeyFromGet(char request[])		//method for extracting key from get request.
{	

	char *b = strstr(request, "("),*c = strstr(request, ")");
	int position = b - request,position1 = c - request,k;
	char to[4];
	strncpy(to, request + position+1, position1 - position);   //for extracting the first integer no. k of put(k,x) 
	k = atoi(to);
	fflush(stdout);
	return k;
}


int extractValueFromPut(char request[])
{

	char *b = strstr(request, ","),*c = strstr(request, ")");
	int position = b - request,position1 = c - request,k;
	char to[32];
	memset(to, 0 , 32);
	strncpy(to, request + position+1, position1 - position);   //for extracting the first integer no. k of put(k,x) 
	k = atoi(to);
	return k;
}


void itoa(int n,char buff[])
{
	int i =0,j;

	if(n==0)
	{
		
		buff[0] = '0';
		buff[1] = '\0';
  		i=1;
 	}

 	while(n>0)
	{
		  		
		int rem = n %10;
  		n=n/10;
  		buff[i]=rem+48;			
		i++;
	} 

	buff[i]='\0';

	for(j=0;j<i/2;j++)
	{

 		char temp = buff[j];
 		buff[j] = buff[i-j-1];
 		buff[i-j-1] = temp;
		
	}

}



char *forwardedData(char inputbuff[],char flag)			//Called to prepare data to be forwaded to next node
{	
	int key,tcpportno;
	char keybuff[5],portbuff[5],fl[2];
	fl[0]=flag;
	fl[1]='\0';
	char *outputbuff=(char *)malloc(sizeof(char)*40),nodebuff[3];

 	if(getOrPut(inputbuff)==1)
  		key = extractKeyFromGet(inputbuff);		//extract key from data get before preparing sending data
 	else
		key = extractKeyFromPut(inputbuff);		//extract key from data get before preparing sending data

	tcpportno=node[num].tcpportno;		

	itoa(tcpportno,portbuff);					//convert integer port number to character array

	itoa(key,keybuff);						//convert integer key to character array

	strcpy(outputbuff,"xxx(");   					//start forwarding data with xxx( 

	strcat(outputbuff,keybuff);					

	strcat(outputbuff,",");						

	strcat(outputbuff,portbuff);					

	strcat(outputbuff,")");   					

	itoa(num,nodebuff);				//convert integer node number to character array

	strcat(outputbuff,nodebuff);					 

	strcat(outputbuff,"[");						

	strcat(outputbuff,node[num].ip_address);	

	strcat(outputbuff,",");						

	strcat(outputbuff,fl);						

	strcat(outputbuff,"]");						

	outputbuff[strlen(outputbuff)+1] = '\0';			//end 

	return outputbuff;					//return the address of data in memory which is to be forwaded
}


char *extractIpaddress(char *buff,char a,char b)
{
	
	char *ptr = (char *)malloc(20);
	int i=0,j=0;

	while(buff[i]!=a)
	{
		i++;
	}
 	i=i+1;

  	while(buff[i]!=b)
	{
    		
		ptr[j++]=buff[i];
    		i++;
  	}
 	ptr[j]='\0';
 	return ptr;
}


int extractNodeno(char * buff)
{

	char *nodeno=extractIpaddress(buff,')','[');
 	return atoi(nodeno);
}



//------------------------------- String handling functions--------------------------------

void forwardUDP( int destination_node ,char sendString[] )
{

	destination_node = destination_node % N;		//destination node to which data is to be forwaded
	int sock;
	struct sockaddr_in server_addr;
	struct msghdr msg;
	struct iovec iov;
				
	struct hostent *host;			//hostent predefined structure use to store info about host
	host = (struct hostent *) gethostbyname(node[destination_node].ip_address);//gethostbyname returns a pointer to hostent 
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{

		perror("socket");
		exit(1);
	}

	//destination address structure
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(node[destination_node].udpportno);		
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);	//host->h_addr gives address of host
	bzero(&(server_addr.sin_zero),8);

#if 0
    memset(&msg, 0 , sizeof(struct msghdr));
    memset(&iov, 0, sizeof(struct iovec));
    msg.msg_name = (void *)&server_addr;
    msg.msg_namelen = sizeof(struct sockaddr_in);
    printf("sendString = %s\n", sendString);
    iov.iov_base = (void *)sendString;
    iov.iov_len = strlen(sendString);
    msg.msg_iov = &iov;
    printf("len = %d\n", strlen(sendString));
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
#endif

	sendto(sock, sendString, strlen(sendString), 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));

	//sendto() function shall send a message through a connectionless-mode socket. 
	printf("\nFORWARD REQUEST : '%s' has been forwarded to node ---->%d\n",sendString,destination_node);  
	close(sock);
} 

int getOrPut(char rec_buff[])			//method to check  whether put or get request.
{

	if(rec_buff[0] == 'g' || rec_buff[0] == 'G')
  		return 1;
	else
  		return 0;
}

int main()
{
 
	int opt=TRUE,TransferValue = 0;
	int master_socket,sock_udp;
	struct sockaddr_in address,server_addr_udp,client_addr;
	int addrlen,addr_len;					
	int new_socket,maxfd;
	int loop,valread;
	//int num;
	char buffer[1025],replyBuffer[1024];  	//data buffer of 1K 
#if 0
	printf("Enter total number of nodes: ");
	scanf("%d",&N);
	printf("Enter the node number:");	
	scanf("%d",&num);
	printf("Enter current node IP:");
	scanf("%s",ip1);	
	printf("Enter next node IP:");
	scanf("%s",ip2);
#endif
	fd_set readfds;				//	Socket file descriptors we want to wake up for, using select()

	node[num].ip_address = ip1;					
	node[num].tcpportno = 2000+num*2;			
	node[num].udpportno = node[num].tcpportno+1;		
	node[(num+1)%N].ip_address = ip2;				
	node[(num+1)%N].tcpportno = 2000+(num+1)%N*2;		
	node[(num+1)%N].udpportno = node[(num+1)%N].tcpportno+1;

	//working with UDP

	system("clear");
	printf ("       INSTRUCTIONS \n\n  =================NODE %d=======================\n",num);
	puts("   1.'put' request format : PUT(<integer>,<integer>)\n");
	puts("   2.'get' request format : GET(<integer>)\n");
	puts("   3.To print Hash Table : 'r'\n");
	puts("-----------------------------------\n\nENTER GET/PUT REQUEST :");
	
	initialiseHashtable();				
	if ((sock_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)		//Creating a UDP Socket
	{		 

            perror("Socket");						
            exit(1);
        }
	//current node address
	server_addr_udp.sin_family = AF_INET;  		//AF_INET represents the address family INET for IPv4 protocol
	server_addr_udp.sin_port = htons(node[num].udpportno);//convert port in host byte order to N/W byte order
	server_addr_udp.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr_udp.sin_zero),8);	//bzero() sets all values in a buffer to zero.


  	if (setsockopt(sock_udp, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))<0)
	{
		
		
    		perror("setsockopt");
    		exit(EXIT_FAILURE);
  	}
  	if (setsockopt(sock_udp, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt))<0)
	{
		
		
    		perror("setsockopt");
    		exit(EXIT_FAILURE);
  	}
#if 1
	if (bind(sock_udp,(struct sockaddr *)&server_addr_udp, sizeof(struct sockaddr)) == -1)//Binding UDP socket
	{		
            perror("Bind");	
            exit(1);
        }
#endif
        addr_len = sizeof(struct sockaddr);
	// working with TCP
	if ((master_socket = socket(AF_INET,SOCK_STREAM,0))==0) 		//Create TCP Socket
	{				
		perror("Create master_socket");
    		exit(EXIT_FAILURE);
  	}

	//set master socket to allow multiple connections 
  	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))<0)
	{
		
		
    		perror("setsockopt");
    		exit(EXIT_FAILURE);
  	}
#if 1
  	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt))<0)
	{
		
		
    		perror("setsockopt");
    		exit(EXIT_FAILURE);
  	}
#endif
	//current node address for tcp
	address.sin_family = AF_INET;			
  	address.sin_addr.s_addr = INADDR_ANY;		
	address.sin_port = htons(node[num].tcpportno);		

	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)	//bind the socket to port 
	{
    		perror("bind");
    		exit(EXIT_FAILURE);
  	}
	if (listen(master_socket, 5)<0)		//Specify maximum of 5 pending connections for the master socket 
	{
    		perror("listen");
    		exit(EXIT_FAILURE);
  	}



	while (1) {

   		FD_ZERO(&readfds);	//clears out the fd_set-readfds, so that it doesn't contain any file descriptors. 


   		FD_SET(sock_udp, &readfds);	//adds file descriptor "sock_udp" to the fd_set,select will return 


   		FD_SET(master_socket, &readfds);	//adds file descriptor "master_socket" to the fd_set,select will return


   		FD_SET(0,&readfds); 			// to read from standard input



   		if(master_socket > sock_udp)
    			maxfd = master_socket;
   		else
    			maxfd = sock_udp;

   		select(maxfd+1, &readfds, NULL, NULL, NULL);
  
		if (FD_ISSET(master_socket, &readfds))
		{

			/* Open the new socket as 'new_socket' */
      			
			addrlen=sizeof(address);
      
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, &addrlen))<0)
			{

				/* if accept failed to return a socket descriptor, display error and exit */
        			perror("accept");
        			exit(EXIT_FAILURE);
       			}

			/* inform user of socket number - used in send and receive commands */
          
      			if(TransferValue != 0)
			{ 				       // means put request was made
        		

          			// means put request was made on this node , and this node need to supply the value 'TransferValue' back to client
           			
				itoa(TransferValue,replyBuffer);// as sooon as conection is made, TCP server send value to client
            			send(new_socket,replyBuffer,strlen(replyBuffer), 0);   
            			read(new_socket, buffer, 1024);
            			printf("\n%s\n",buffer);
	   			TransferValue = 0 ;
           			close(new_socket);
        		}
        		else
			{
	          		// means get request was made and tcp server is only need to recieve value from client
            			if ((valread = read(new_socket, buffer, 1024)) < 0) 
            				close(new_socket);
        			
				else
				 {
					/* set the terminating NULL byte on the end of the data read */
        				buffer[valread] = 0;
					printf("\n%s\n",buffer); //m
        			}
        			close(new_socket);
			}
		 }      

		if(FD_ISSET(sock_udp, &readfds))	/*Check udp_socket has data available to be read */
		 {
 			char rec_buff[5000];
			int len = recvfrom(sock_udp,rec_buff,5000,0,(struct sockaddr *)&client_addr, &addr_len);
			//recvfrom() function receives a message 
			rec_buff[len] = '\0';
			printf("\n--------\nUDP PACKET RECIEVED FROM (IP ADDRESS : %s , PORT NO : %d , NODE NO : %d)  : ",
						inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),extractNodeno(rec_buff));

			//inet_ntoa() converts a n/w address in a struct in_addr to a dots-and-numbers format string		
			printf("%s\n",rec_buff);

			// Either get request is recieved or put request is recieved

			// Check whether the req is to be forwarded or to be processed by the current node itself

      			if(forYou(num,rec_buff) == 0)
			{

         			forwardUDP(num+1,rec_buff); // forward the request as it is to the next node
	 			puts("\n---------------------------------------\nENTER NEW GET/PUT REQUEST:");     
        		}
			else
			{
				printf("\nPROCESSING THE REQUEST ON THE CURRENT NODE : \n")
;
   				// this is the node where get/put request is to be processed, so fetching
				// extracting key from request

				int key = extractKeyFromPut(rec_buff),nodeno = extractNodeno(rec_buff);
		 
				// establish TCP conectiom with server whether it is get Or put message

        			int sock, bytes_recieved;  
        			char send_data[1024],recv_data[1024];
        			char flag = rec_buff[strlen(rec_buff)-2];	//extracted flag value from request
        			struct hostent *host;
        			struct sockaddr_in server_addr;  
        			host = gethostbyname(extractIpaddress(rec_buff,'[',','));//extracting originator IP from request 

        			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				{

            				perror("Socket");
            				exit(1);
        			}
				//To which connection is to be made
				server_addr.sin_family = AF_INET;     
        			server_addr.sin_port = htons(extractValueFromPut(rec_buff)); 
					//same format put(key,value) initially
					//now format put(key,port)  
        			server_addr.sin_addr = *((struct in_addr *)host->h_addr);
        			bzero(&(server_addr.sin_zero),8); 

        			if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1) //Make Connec
				{

            				perror("Connect");
            				exit(1);
        			}        
              			if(flag == 's')		//Get request and the value is to be fetched from hash table
				{

               				int valuefetched = fetchValueFromHT(key);
               				if(valuefetched !=0)
					{

               					strcpy(send_data,"value = ");
               					char valuebuff[6],nodebuff[4];
               					itoa(valuefetched,valuebuff);
               					strcat(send_data,valuebuff);
               					strcat(send_data,". Value retrieved from node no : ");
               					itoa(num,nodebuff);
               					strcat(send_data,nodebuff);
						strcat(send_data,"\n-------------------\nENTER NEW GET/PUT REQUEST:");
                			}
               				else
					{
               					strcpy(send_data,"RESULT: No hash entry to this key on node no : ");
               					char nodebuff[4];
               					itoa(num,nodebuff);
               					strcat(send_data,nodebuff);
      	       					strcat(send_data,"\n--------------------\nENTER NEW GET/PUT REQUEST :");

              				}
              				 send(sock,send_data,strlen(send_data), 0); //Send the fetched value
                                       printf("\n KEY RECIEVED %d ,\nREQUEST ORIGINALLY INVOKED ON NODE  %d , flag recieved = %c . VALUE SUPPLIED BACK ON TCP CONNECTION.\nENTER NEW GET/PUT REQUEST:",key,nodeno,rec_buff[strlen(rec_buff)-2]);
              			}
              			else
				{
              				bytes_recieved=recv(sock,recv_data,1024,0);	//Receiving Put value
              				recv_data[bytes_recieved] = '\0';
                                         printf("\n KEY RECIEVED IS: %d ,\nREQUEST ORIGINALLY INVOKED ON NODE  %d , flag recieved = %c\n",key,nodeno,rec_buff[strlen(rec_buff)-2]); 
              				
					printf("\nVALUE RECIEVED (ON TCP CONNECTION) FROM NODE NO %d = %s " ,nodeno, recv_data);
					// now insert the key,value in hash table of this node and send confirmation message 						back to parent node
					// on which the request was originallly invoked by the  user

               				if(addToHashtable(key,atoi(recv_data)))
               					strcpy(send_data,"RESULT: put operation has been done successfully.  Value added on node no :");
	       				else
               					strcpy(send_data,"RESULT: put operation failed. Maximum key limit Exceeded on node number  ");
               				char nodebuff[4];
               				itoa(num,nodebuff);
               				strcat(send_data,nodebuff);
	       				strcat(send_data,".\n------------------------------------\nENTER NEW GET/PUT REQUEST :");
               				send(sock,send_data,strlen(send_data), 0); 

				}	
        			close(sock);
        			fflush(stdout);
			} // else ends
		}


		//  working with console 


		if (FD_ISSET(0, &readfds))		//FD_ISSET()Returns a non-zero value if the bit for the file descriptor 							'0' is set in the file descriptor set pointed to by readfds, and 0 								otherwise.
		{
		
        		char rec_buff[5000];
			gets(rec_buff);		


 			if(rec_buff[0] == 'r' || rec_buff[0] == 'R')
			{

        			displayHtable();	
       			}

       			else if(forYou(num,rec_buff) == 0)//fun return 1 if for you;return 0 if not for you
			{							
         			char outputbuff[40],*out,flag;
         			int i =0;

           			if(getOrPut(rec_buff) == 0)		//fun() return 1 for get, 0 for put
				{   
					//value from put to be transferred at last
            				TransferValue = extractValueFromPut(rec_buff);

            				flag = 'r';	 // indicates that last node has to receive a value
          			}
          			else
            				flag = 's';	//indicates that last node will send a value
     				
        			out = forwardedData(rec_buff,flag);     //fun() to prepare the data to be forwaded
        			
				
				for(i=0;i<strlen(out);i++)
        				outputbuff[i] = *(out+i);	//data to be forwaded is assigned to outputbuff
 
        			outputbuff[i] = '\0';			

        			forwardUDP(num+1,outputbuff);	//forwading method called
        			free(out);                    
		        }
       
			else
			{
				printf("\nPROCESSING THE REQUEST HERE:\n-------------------\n");

				

    				if(getOrPut(rec_buff)==1)	//fun() return 1 for get, 0 for put 
				{
					
					// extract key from get request
     					int key = extractKeyFromGet(rec_buff),value;
					int maxkeylimit=(tablesize-1)*N+num;  //Compute maximum key limit

     					if(key <= maxkeylimit)
					{

   						value = fetchValueFromHT(key);	//call fun() to fetch value from hash table

     						if(value == 0)
      							
							printf("\nError - No value in Hash table for this key on Current node. 								\n---------------------\nEnter NEW GET/PUT REQUEST :"); 
     						
						else
      							printf("\n key = %d, value = %d on same node \n----------------------- 								-\nEnter GET/PUT REQUEST :",key,value);
      					}
     					else
					{
        					printf("\n Result :  Error - value cannot be fetched , maximum key is %d 							\n----------------------\nEnter NEW GET/PUT REQUEST :",maxkeylimit);
      					}
    				}
    				else
				{
      					// processing put request on the same node
       					addToHashtable(extractKeyFromPut(rec_buff),extractValueFromPut(rec_buff));
       				}

			} //else ends
			fflush(stdout);
                
		} // read from console ends here


	}  // while ends

} // main ends

