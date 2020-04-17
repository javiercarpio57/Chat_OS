// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#define PORT 8080 

void reconnect(void ){

}

int main(int argc, char const *argv[]) 
{ 
    //Argv[1]: Nombre
    //Argv[2]: Username
    //Argv[3]: IP
    //Argv[4]: Port    

    char Nombre = argv[1];
    char Username = argv[2];
    char IP = argv[3];
    char Port = argv[4];
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    //char *hello = argv[1]; 
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(Port); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, IP, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    send(sock , hello , strlen(hello) , 0 ); 
    printf("Message from %s: \n", Username); 
    valread = read( sock , buffer, 1024); 
    printf("%s\n",buffer ); 
    return 0;
} 