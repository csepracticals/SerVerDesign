#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include "common.h"
#include <arpa/inet.h>


#define MAX_CLIENT_SUPPORTED    32
#define SERVER_PORT     2000 /*Server process is running on this port no. Client has to send data to this port no*/

test_struct_t test_struct;
result_struct_t res_struct;
char data_buffer[1024];

int monitored_fd_set[32];

static void
intitiaze_monitor_fd_set(){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++)
        monitored_fd_set[i] = -1;
}

static void 
add_to_monitored_fd_set(int skt_fd){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){

        if(monitored_fd_set[i] != -1)
            continue;   
        monitored_fd_set[i] = skt_fd;
        break;
    }
}

static void
remove_from_monitored_fd_set(int skt_fd){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){

        if(monitored_fd_set[i] != skt_fd)
            continue;

        monitored_fd_set[i] = -1;
        break;
    }
}

static void
re_init_readfds(fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);
    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != -1){
            FD_SET(monitored_fd_set[i], fd_set_ptr);
        }
    }
}

static int 
get_max_fd(){

    int i = 0;
    int max = -1;

    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] > max)
            max = monitored_fd_set[i];
    }

    return max;
}

void
setup_tcp_server_communication(){

    /*Step 1 : Initialization*/
    /*Socket handle and other variables*/
    int master_sock_tcp_fd = 0, /*Master socket file descriptor, used to accept new client connection only, no data exchange*/
        sent_recv_bytes = 0, 
        addr_len = 0, 
        opt = 1;

    int comm_socket_fd = 0;     /*client specific communication socket file descriptor, used for only data exchange/communication between client and server*/
    fd_set readfds;             /*Set of file descriptor on which select() polls. Select() unblocks whever data arrives on any fd present in this set*/
    /*variables to hold server information*/
    struct sockaddr_in server_addr, /*structure to store the server and client info*/
                       client_addr;

    /* Just drain the array of monitored file descriptors (sockets)*/
    intitiaze_monitor_fd_set();

    /*step 2: tcp master socket creation*/
    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
    {
        printf("socket creation failed\n");
        exit(1);
    }

    /*Step 3: specify server Information*/
    server_addr.sin_family = AF_INET;/*This socket will process only ipv4 network packets*/
    server_addr.sin_port = SERVER_PORT;/*Server will process any data arriving on port no 2000*/
    server_addr.sin_addr.s_addr = INADDR_ANY; //3232249957; //( = 192.168.56.101); /*Server's IP address, means, Linux will send all data whose destination address = address of any local interface of this machine, in this case it is 192.168.56.101*/

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




    /*Add master socket to Monitored set of FDs*/
    add_to_monitored_fd_set(master_sock_tcp_fd);

    /* Server infinite loop for servicing the client*/


    while(1){

        /*Step 5 : initialze and dill readfds*/
        //FD_ZERO(&readfds);                     /* Initialize the file descriptor set*/
        re_init_readfds(&readfds);               /*Copy the entire monitored FDs to readfds*/
        //FD_SET(master_sock_tcp_fd, &readfds);  /*Add the socket to this set on which our server is running*/

        printf("blocked on select System call...\n");

        /*Step 6 : Wait for client connection*/
        /*state Machine state 1 */
        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL); /*Call the select system cal, server process blocks here. Linux OS keeps this process blocked untill the data arrives on any of the file Drscriptors in the 'readfds' set*/

        /*Some data on some fd present in monitored fd set has arrived, Now check on which File descriptor the data arrives, and process accordingly*/

        /*If Data arrives on master socket FD*/
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

            add_to_monitored_fd_set(comm_socket_fd); 
            printf("Connection accepted from client : %s:%u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
        else /* Data srrives on some other client FD*/
        {

            int i = 0, comm_socket_fd = -1;
            for(; i < MAX_CLIENT_SUPPORTED; i++){


                if(FD_ISSET(monitored_fd_set[i], &readfds)){/*Find the clinet FD on which Data has arrived*/

                    comm_socket_fd = monitored_fd_set[i];

                    memset(data_buffer, 0, sizeof(data_buffer));
                    sent_recv_bytes = recvfrom(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0, (struct sockaddr *)&client_addr, &addr_len);

                    printf("Server recvd %d bytes from client %s:%u\n", sent_recv_bytes,
                            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    if(sent_recv_bytes == 0){
                        /*If server recvs empty msg from client, server may close the connection and wait
                         * for fresh new connection from client - same or different*/
                        close(comm_socket_fd);
                        remove_from_monitored_fd_set(comm_socket_fd);
                        break; /*goto step 5*/

                    }


                    test_struct_t *client_data = (test_struct_t *)data_buffer;

                    /* If the client sends a special msg to server, then server close the client connection
                     * for forever*/
                    /*Step 9 */
                    if(client_data->a == 0 && client_data->b ==0){

                        close(comm_socket_fd);
                        remove_from_monitored_fd_set(comm_socket_fd);
                        printf("Server closes connection with client : %s:%u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        /*Goto state machine State 1*/
                        break;/*Get out of inner while loop, server is done with this client, time to check for new connection request by executing selct()*/
                    }

                    result_struct_t result;
                    result.c = client_data->a + client_data->b;

                    /* Server replying back to client now*/
                    sent_recv_bytes = sendto(comm_socket_fd, (char *)&result, sizeof(result_struct_t), 0,
                            (struct sockaddr *)&client_addr, sizeof(struct sockaddr));

                    printf("Server sent %d bytes in reply to client\n", sent_recv_bytes);
                }
            }
        }

    }/*step 10 : wait for new client request again*/    
}

int
main(int argc, char **argv){

    setup_tcp_server_communication();
    return 0;
}
