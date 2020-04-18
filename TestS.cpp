// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
# include "mensaje.pb.h"

#define PORT 8080 

using namespace chat;
using namespace std;

int createSocket () {
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    // char *hello = "Hello from server"; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 

    cout << AF_INET << endl;
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    return new_socket;
}

int main(int argc, char const *argv[]) 
{ 
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    char buffer[1024] = {0}; 

    while (true) {
        int socket = createSocket();
        int valread = read( socket , buffer, 1024); 

        // ------------ PRUEBA DE RECEPCION DE PROTO -----------------------
        ClientMessage cliente;
        string msg;

        cliente.ParseFromString(buffer);

        cout << cliente.option () << endl;
        cout << cliente.synchronize().ip() << endl;
        cout << cliente.synchronize().username() << endl;
        cout << socket << endl;
        cout << "-------------------------------------" << endl;
        // -----------------------------------------------------------------
    }
    

    // printf("%s\n",buffer ); 


    // send(new_socket , hello , strlen(hello) , 0 ); 
    // printf("Hello message sent from Server \n"); 
    return 0; 

    google::protobuf::ShutdownProtobufLibrary();
} 