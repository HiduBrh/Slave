#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVER_PORT 50001
#define SERVER_PORT_EXCHANGE 50002
#define SERVER_REPLY_SIZE 20
#define SLAVE_COMMANDE "SLAVE "
#define NEED_COMMANDE "NEED "
#define BAD "BAD"
#define BAD_COMMAND "BAD COMMAND"
#define NOSUCHFILE "NOSUCHFILE"
#define SEND_SIZE 1000000
#define BACKLOG_LENGTH 1000
#define CONTENT "CONTENT "

char *criteres[5]={"film","code","texte","musique","classeur"};

int get_random(int min,int max){
    return  rand()%(max-min) +min;
}

int main(int argc , char *argv[])
{
    int sock,sock_exchange,client_socket,c,pid,read_size;
    struct sockaddr_in server, exchange,client;
    char recu[SEND_SIZE-5] , server_reply[SERVER_REPLY_SIZE], toSend[SEND_SIZE];

//creation des sockets
    sock = socket(AF_INET , SOCK_STREAM , 0);
    sock_exchange=socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1 || sock_exchange == -1)
    {
        printf("Could not create sockets");
    }
    puts("Sockets created");
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    exchange.sin_addr.s_addr = inet_addr("127.0.0.1");
    exchange.sin_family = AF_INET;
    exchange.sin_port = htons(SERVER_PORT_EXCHANGE);

//Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");
//authentification from server
    sprintf(recu, "%d", get_random(50001,99999));
    strncpy(toSend,SLAVE_COMMANDE,strlen(SLAVE_COMMANDE));
    strcat(toSend,recu);
    if( send(sock , toSend , strlen(toSend) , 0) < 0)
    {
        puts("Authentification failed");
        return 1;
    }
    memset(toSend, 0, sizeof(toSend));
    memset(recu, 0, sizeof(recu));
//Receive a reply from the server
    if( recv(sock , server_reply , SERVER_REPLY_SIZE , 0) < 0)
    {
        puts("Authentification failed");
    }
    puts(server_reply);
    if(strstr(server_reply,BAD)) {
        close(sock);
        return 1;
    }
    //generations des files ids
// to dO
//
    close(sock);
///////////////////////exchanging part
//Bind
    if( bind(sock_exchange,(struct sockaddr *)&exchange , sizeof(exchange)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

//Listen
    listen(sock_exchange, BACKLOG_LENGTH);
    while(1) {
        //Accept and incoming connection
        puts("Waiting for incoming connections...");
        c = sizeof(struct sockaddr_in);

        //accept connection from an incoming client
        client_socket = accept(sock_exchange, (struct sockaddr *) &client, (socklen_t *) &c);
        if (client_socket < 0) {
            perror("accept failed");
            return 1;
        }
        puts("Connection accepted");

        //deleguer a un sous processus
        if ((pid = fork()) == 0) {
            printf(" process %d \n", pid);
            //Receive a message from client
            while ((read_size = recv(client_socket, server_reply, SERVER_REPLY_SIZE, 0)) > 0) {
                puts(server_reply);
                if (strstr(server_reply, NEED_COMMANDE)) {
                    char criteres[SEND_SIZE - 5];
                    strncpy(recu, SEND_SIZE + 5, SEND_SIZE - 5);
                    recu[SERVER_REPLY_SIZE - 5] = '\0';
                    puts(recu);
                    char * file="contenu du fichier";
                    //Send the message back to client
                    char file_size[20];
                    sprintf(file_size, "%d", 123456789);
                    strncpy(toSend,CONTENT,strlen(CONTENT));
                    strncpy(toSend,file_size,strlen(file_size));
                    strncpy(toSend,file,strlen(file));
                    write(client_socket, toSend, strlen(toSend));
                    memset(toSend, 0, sizeof(toSend));
                    memset(recu, 0, sizeof(recu));

                } else {
                    write(client_socket, BAD_COMMAND, strlen(BAD_COMMAND));
                }

            }

            if (read_size == -1) {
                perror("recv failed");
            }
            close(sock_exchange);
            return 0;
        }
    }
}