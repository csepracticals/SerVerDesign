#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>

/*Server process is running on this port no. Client has to send data to this port no*/
#define SERVER_PORT     2000

char data_buffer[1024];


/*string helping functions*/

/*Remove the space from both sides of the string*/
void
string_space_trim(char *string){

    if(!string)
        return;

    char* ptr = string;
    int len = strlen(ptr);

    if(!len){
        return;
    }

    if(!isspace(ptr[0]) && !isspace(ptr[len-1])){
        return;
    }

    while(len-1 > 0 && isspace(ptr[len-1])){
        ptr[--len] = 0;
    }

    while(*ptr && isspace(*ptr)){
        ++ptr, --len;
    }

    memmove(string, ptr, len + 1);
}

typedef struct student_{

    char name[32];
    unsigned int roll_no;
    char hobby[32];
    char dept[32];
} student_t; 

student_t student[5] = {
    {"Abhishek", 10305042, "Programming", "CSE"},
    {"Nitin", 10305048, "Programming", "CSE"},
    {"Avinash", 10305041, "Cricket", "ECE"},
    {"Jack", 10305032, "Udemy Teaching", "Mechanical"},
    {"Cris", 10305030, "Programming", "Electrical"}};


static char *
process_GET_request(char *URL, unsigned int *response_len){

    printf("%s(%u) called with URL = %s\n", __FUNCTION__, __LINE__, URL);
    
    /*Let us extract the roll no of a students from URL using 
     * string handling 
     *URL : /College/IIT/?dept=CSE&rollno=10305042/
     * */
    char delimeter[2] = {'?', '\0'};

    string_space_trim(URL);
    char *token[5] = {0};

    token[0] = strtok(URL, delimeter);
    token[1] = strtok(0, delimeter);
    /*token[1] = dept=CSE&rollno=10305042*/
    delimeter[0] = '&';

    token[2] = strtok(token[1], delimeter);
    token[3] = strtok(0, delimeter);
    /*token[2] = dept=CSE, token[3] = rollno=10305042*/

    printf("token[0] = %s, token[1] = %s, token[2] = %s, token[3] = %s\n",
        token[0] , token[1], token[2], token[3]);

    delimeter[0] = '=';
    char *roll_no_str = strtok(token[3], delimeter);
    char *roll_no_value = strtok(0, delimeter);
    printf("roll_no_value = %s\n", roll_no_value);
    unsigned int roll_no = atoi(roll_no_value), i = 0;

    for(i = 0; i < 5; i++){
        if(student[i].roll_no != roll_no){
            continue;
        }
        break;
    }
    
    if(i == 5)
        return NULL;
    
    /*We have got the students of interest here*/
    char *response = calloc(1, 1024);

    strcpy(response,
        "<html>"
        "<head>"
            "<title>HTML Response</title>"
            "<style>"
            "table, th, td {"
                "border: 1px solid black;}"
             "</style>"
        "</head>"
        "<body>"
        "<table>"
        "<tr>"
        "<td>");

        strcat(response , 
            student[i].name
        );

        strcat(response ,
            "</td></tr>");
        strcat(response , 
                "</table>"
                "</body>"
                "</html>");

    unsigned int content_len_str = strlen(response);

    /*create HTML hdr returned by server*/
    char *header  = calloc(1, 248 + content_len_str);
    strcpy(header, "HTTP/1.1 200 OK\n");      
    strcat(header, "Server: My Personal HTTP Server\n"    );
    strcat(header, "Content-Length: "  ); 
    strcat(header, "Connection: close\n"   );
    //strcat(header, itoa(content_len_str)); 
    strcat(header, "2048");
    strcat(header, "\n");
    strcat(header, "Content-Type: text/html; charset=UTF-8\n");
    strcat(header, "\n");

    strcat(header, response);
    content_len_str = strlen(header); 
    *response_len = content_len_str;
    free(response);
    return header;
}

static char *
process_POST_request(char *URL, unsigned int *response_len){

    return NULL;
}

void
setup_tcp_server_communication(){

    /*Step 1 : Initialization*/
    /*Socket handle and other variables*/
    /*Master socket file descriptor, used to accept new client connection only, no data exchange*/
    int master_sock_tcp_fd = 0, 
        sent_recv_bytes = 0, 
        addr_len = 0, 
        opt = 1;

    /*client specific communication socket file descriptor, 
     * used for only data exchange/communication between client and server*/
    int comm_socket_fd = 0;     
    /* Set of file descriptor on which select() polls. Select() unblocks whever data arrives on 
     * any fd present in this set*/
    fd_set readfds;             
    /*variables to hold server information*/
    struct sockaddr_in server_addr, /*structure to store the server and client info*/
                       client_addr;

    /*step 2: tcp master socket creation*/
    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
    {
        printf("socket creation failed\n");
        exit(1);
    }
    
    //set master socket to allow multiple connections
    if (setsockopt(master_sock_tcp_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))<0){
        printf("TCP socket creation failed for multiple connections\n");
        exit(EXIT_FAILURE);
    }

    /*Step 3: specify server Information*/
    server_addr.sin_family = AF_INET;/*This socket will process only ipv4 network packets*/
    server_addr.sin_port = htons(SERVER_PORT);/*Server will process any data arriving on port no 2000*/
    
    /*3232249957; //( = 192.168.56.101); Server's IP address, 
    //means, Linux will send all data whose destination address = address of any local interface 
    //of this machine, in this case it is 192.168.56.101*/
    server_addr.sin_addr.s_addr = INADDR_ANY; 

    addr_len = sizeof(struct sockaddr);

    /* Bind the server. Binding means, we are telling kernel(OS) that any data 
     * you recieve with dest ip address = 192.168.56.101, and tcp port no = 2000, pls send that data to this process
     * bind() is a mechnism to tell OS what kind of data server process is interested in to recieve. Remember, server machine
     * can run multiple server processes to process different data and service different clients. Note that, bind() is 
     * used on server side, not on client side*/

    if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("socket bind failed\n");
        return;
    }

    /*Step 4 : Tell the Linux OS to maintain the queue of max length to Queue incoming
     * client connections.*/
    if (listen(master_sock_tcp_fd, 5)<0)  
    {
        printf("listen failed\n");
        return;
    }

    /* Server infinite loop for servicing the client*/

    while(1){

        /*Step 5 : initialze and dill readfds*/
        FD_ZERO(&readfds);                     /* Initialize the file descriptor set*/
        FD_SET(master_sock_tcp_fd, &readfds);  /*Add the socket to this set on which our server is running*/

        printf("blocked on select System call...\n");


        /*Step 6 : Wait for client connection*/
        /*state Machine state 1 */
        
        /*Call the select system call, server process blocks here. Linux OS keeps this process blocked untill the data arrives on any of the file Drscriptors in the 'readfds' set*/
        select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL); 

        /*Some data on some fd present in set has arrived, Now check on which File descriptor the data arrives, and process accordingly*/
        if (FD_ISSET(master_sock_tcp_fd, &readfds))
        { 
            /*Data arrives on Master socket only when new client connects with the server (that is, 'connect' call is invoked on client side)*/
            printf("New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");

            /* step 7 : accept() returns a new temporary file desriptor(fd). Server uses this 'comm_socket_fd' fd for the rest of the
             * life of connection with this client to send and recieve msg. Master socket is used only for accepting
             * new client's connection and not for data exchange with the client*/
            /* state Machine state 2*/
            comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
            if(comm_socket_fd < 0){
                /* if accept failed to return a socket descriptor, display error and exit */
                printf("accept error : errno = %d\n", errno);
                exit(0);
            }

            printf("Connection accepted from client : %s:%u\n", 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            while(1){
                printf("Server ready to service client msgs.\n");
                /*Drain to store client info (ip and port) when data arrives from client, sometimes, server would want to find the identity of the client sending msgs*/
                memset(data_buffer, 0, sizeof(data_buffer));

                /*Step 8: Server recieving the data from client. Client IP and PORT no will be stored in client_addr
                 * by the kernel. Server will use this client_addr info to reply back to client*/

                /*Like in client case, this is also a blocking system call, meaning, server process halts here untill
                 * data arrives on this comm_socket_fd from client whose connection request has been accepted via accept()*/
                /* state Machine state 3*/
                sent_recv_bytes = recvfrom(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0,
                        (struct sockaddr *)&client_addr, &addr_len);

                /* state Machine state 4*/
                printf("Server recvd %d bytes from client %s:%u\n", sent_recv_bytes, 
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                if(sent_recv_bytes == 0){
                /* If server recvs empty msg from client, server may close the connection and wait
                 * for fresh new connection from client - same or different*/
                close(comm_socket_fd);
                break; /*goto step 5*/

                }




                /****************************************************************/
                 /*BEGIN : Implement the HTTP request processing functionality */
                /****************************************************************/
                
                printf("Msg recieved : %s\n", data_buffer);
                char *request_line = NULL;
                char del[2] = "\n", 
                     *method = NULL,
                     *URL = NULL;
                request_line = strtok(data_buffer, del); /*Extract out the request line*/
                del[0] = ' ';
                method = strtok(request_line, del);     /*Tokenize the request line on the basis of space, and extract the first word*/
                URL = strtok(NULL, del);                /*Extract the URL*/
                printf("Method = %s\n", method);
                printf("URL = %s\n", URL);
                char *response = NULL;
                unsigned int response_length = 0 ;

                if(strncmp(method, "GET", strlen("GET")) == 0){
                    response = process_GET_request(URL, &response_length);
                }
                else if(strncmp(method, "POST", strlen("POST")) == 0){
                    response = process_POST_request(URL, &response_length);
                }
                else{
                    printf("Unsupported URL method request\n");
                    close(comm_socket_fd);
                    break;
                }

                /****************************************************************/
                 /*END : Implement the HTTP request processing functionality */
                /****************************************************************/




                /* Server replying back to client now*/
                if(response){
                    printf("response to be sent to client = \n%s", response);
                    sent_recv_bytes = sendto(comm_socket_fd, response, response_length, 0,
                            (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                    free(response);
                    printf("Server sent %d bytes in reply to client\n", sent_recv_bytes);
                    //close(comm_socket_fd);
                    //break;
                }
                /*Goto state machine State 3*/
            }
        }
    }/*step 10 : wait for new client request again*/    
}

int
main(int argc, char **argv){

    setup_tcp_server_communication();
    return 0;
}
