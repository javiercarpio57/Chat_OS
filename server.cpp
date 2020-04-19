//g++ server.cpp mensaje.pb.cc -lprotobuf -lpthread -o server
//g++ client.cpp mensaje.pb.cc -lprotobuf -lpthread -o client
#include <string>
#include <iostream>
#include <thread> 
#include <queue> 
#include <list>

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
# include "mensaje.pb.h"

#include "mensaje.pb.h"

using namespace std;
using namespace chat;

#define PORT 8080 

struct user {
   string username ;
   string ip;
   int userId;
   string status;
   int socket;
};


//int sock = 0;
list <thread> threadList;
list <int> threadIdList;
list <user> userList;
list <queue<ClientMessage>> requestList;
queue<string> binaryList;

int threadCount = 0;

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

void sendBySocket (string msg, int sock) {
    //mutex
    //configurar
    char buffer[1024] = {0};
    strcpy(buffer, msg.c_str());

    send(sock, buffer, msg.size() + 1, 0);
}

user getUser(int id){
    std::list<user>::iterator it = userList.begin();
    user tempUser = *it;
    while (tempUser.userId != id) {
        std::advance(it, 1);
        tempUser = *it;
    }
    return *it;
}

int getUserPos(int id){
    std::list<user>::iterator it = userList.begin();
    user tempUser = *it;
    int cont = 0;
    while (tempUser.userId != id) {
        std::advance(it, 1);
        tempUser = *it;
        cont++;
    }
    return cont;
}

void changeStatusInList(int id, string status){
    std::list<user>::iterator it = userList.begin();
    user tempUser = *it;
    int cont = 0;
    while (tempUser.userId != id) {
        std::advance(it, 1);
        tempUser = *it;
    }
    tempUser.status = status;
    //Sustitute value
    userList.erase(it);
    userList.insert(it, tempUser);
    //Ctrate message
    
}

void getConnectedUsers(connectedUserRequest cur, int socket){
    ConnectedUserResponse * response(new ConnectedUserResponse);
    if (cur.userid() == 0){
        //All users
        ConnectedUser * tempConectedUser;
        for (int i = 0; i < userList.size(); i++){
            user temporalUser = getUser(i); 
            tempConectedUser = response->add_connectedusers();
            tempConectedUser->set_userid(temporalUser.userId);
            tempConectedUser->set_username(temporalUser.username);
            tempConectedUser->set_status(temporalUser.status);
            tempConectedUser->set_ip(temporalUser.ip);
        }
    } else {
        //Single user
        ConnectedUser * tempConectedUser;
        tempConectedUser = response->add_connectedusers();
        user temporalUser = getUser(cur.userid()); 
        tempConectedUser->set_userid(temporalUser.userId);
        tempConectedUser->set_username(temporalUser.username);
        tempConectedUser->set_status(temporalUser.status);
        tempConectedUser->set_ip(temporalUser.ip);
    }
    ServerMessage * m(new ServerMessage);
    m->set_option(7); 
    m->set_allocated_connecteduserresponse(response);
    string binary;
    m->SerializeToString(&binary);
    sendBySocket(binary, socket);
}

void sendBroadcast(int id, string message, int socket){ ///FIx broadcast
    //Server response to sender 
    BroadcastResponse * response(new BroadcastResponse);
    response->set_messagestatus("Send");
    ServerMessage * m(new ServerMessage);
    m->set_option(7); 
    m->set_allocated_broadcastresponse(response);
    string binary;
    m->SerializeToString(&binary);
    sendBySocket(binary, socket);
    //server response to everybody
    BroadcastMessage * globalResponse(new BroadcastMessage);
    globalResponse->set_message(message);
    globalResponse->set_userid(id);
    ServerMessage * gM(new ServerMessage);
    gM->set_option(1); 
    gM->set_allocated_broadcast(globalResponse);
    binary;
    gM->SerializeToString(&binary);
    for (int i = 0; i < userList.size(); i++){
        user temporalUser = getUser(i);
        sendBySocket(binary, temporalUser.socket);
    }
}//Add , send to everybody

void sendMessage(int ids,int idr , string message, int socket){ 
    //Server response to sender 
    DirectMessageResponse * response(new DirectMessageResponse);
    response->set_messagestatus("Send");
    ServerMessage * m(new ServerMessage);
    m->set_option(7); 
    m->set_allocated_directmessageresponse(response);
    string binary;
    m->SerializeToString(&binary);
    sendBySocket(binary, socket);
    //server response to person
    DirectMessage * directMessage(new DirectMessage);
    directMessage->set_message(message);
    directMessage->set_userid(0); //fix proto should be int
    ServerMessage * pm (new ServerMessage);
    pm->set_option(1); 
    pm->set_allocated_message(directMessage);
    binary = "";
    pm->SerializeToString(&binary);

    user temporalUser = getUser(idr);
    sendBySocket(binary, temporalUser.socket);
}


void changeStatus(int id, string status, int socket){ 
    //Server response to sender 
    changeStatusInList(id, status);
    //server response to everybody
    ChangeStatusResponse * changeStatusResponse(new ChangeStatusResponse);
    changeStatusResponse->set_status(status);
    changeStatusResponse->set_userid(0);
    ServerMessage * pm (new ServerMessage);
    pm->set_option(7); 
    pm->set_allocated_changestatusresponse(changeStatusResponse);
    string binary;
    pm->SerializeToString(&binary);
    sendBySocket(binary, socket);
    //Add , send to user
}

//Thread code
void foo(user user, int id ) 
{
    /*
    printf("Hola\n");
    int mySock = createSocket();
    MyInfoResponse * response(new MyInfoResponse);
    response->set_userid(id);
    ServerMessage * m(new ServerMessage);
    m->set_option(5); 
    m->set_allocated_myinforesponse(response);
    string binary;
    m->SerializeToString(&binary);
    //Asign requestList
    */
    int mySock = user.socket;
    //Get request list 
    std::list<queue<ClientMessage>>::iterator it = requestList.begin();
    std::advance(it, id);
    queue<ClientMessage> request = *it;

    int acknowledgement = 0;
    printf("Response from server to client\n");
    //waiting for acknowledgement
    while(acknowledgement == 0){
        
        if (!request.empty()){
            ClientMessage temp = request.front();
            request.pop();
            if (temp.option() == 8) {
                acknowledgement = 1 ;
            } else {
                acknowledgement = -1 ;
            }
        }
        //acknowledgement = 1 ;//Remove later
    }
    if (acknowledgement == -1) {
        ;
    }
    //**Fix: Add condition for failed acknowledgement**
    printf("Acknowledgement was recive\n");
    int working = 0;
    //waiting for request from user
    
    while(working == 0){
        if (!request.empty()){
            ClientMessage temp = request.front();
            request.pop();
            switch (temp.option()) {
                case 2: 
                   getConnectedUsers(temp.connectedusers(), mySock);
                break;
                case 3: 
                    changeStatus(user.userId, temp.changestatus().status(), mySock);
                break;
                case 4: 
                    sendBroadcast(user.userId, temp.broadcast().message(), mySock);
                break;
                case 5: 
                    sendMessage(user.userId, temp.directmessage().userid(),temp.broadcast().message(), mySock);
                break;
                default:
                ;
                    //Error handling
            }
        }
    }
}
/*
void root() {
    // Message desynchronize
    ClientMessage m2;
    while (true){
        if (!binaryList.empty()){
            //printf("server: %s\n", binaryList.front());
            m2.ParseFromString(binaryList.front());
            binaryList.pop();
            if (m2.option() == 1){ //Change to entering message
                printf("User created\n");
                //Lookup for username
                
                user tempUser ;
                tempUser.username = m2.synchronize().username();
                tempUser.ip = m2.synchronize().ip();
                tempUser.userId = threadCount;
                tempUser.status = "";
                
                threadIdList.push_back(threadCount);
                userList.push_back(tempUser);
                
                queue<ClientMessage> tempQueue;
                requestList.push_back(tempQueue);
                
                threadList.push_back(thread(foo, tempUser, threadCount));
                threadCount ++ ;
                printf("Thread added\n");
            } else {
                //Get position of user 
                /*
                int pos = getUserPos(m2.userid());
                //Add to user thread request queue
                std::list<queue<ClientMessage>>::iterator it = requestList.begin();
                std::advance(it, pos);
                queue<ClientMessage> request = *it;
                request.push(m2);
            }
        }
    }
    //Wait for all created threads to end
    /*
    std::thread temp;
    for (int i = 0; i< threadCount; i++){
        threadList.front().join();
        threadList.pop_front();
    }
}
/*
int main (int argc, char **argv) { 
    //FIX user id cannot be 0
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    //Message setup
    /*
    MyInfoSynchronize * miInfo(new MyInfoSynchronize);
    miInfo->set_username("username123");
    miInfo->set_ip("127.0.0.1");
    ClientMessage * m(new ClientMessage);
    m->set_option(0);
    m->set_allocated_synchronize(miInfo);
    string binary;
    m->SerializeToString(&binary);
    binaryList.push(binary);
    
    
    int sock, valread; 
    struct sockaddr_in address;  
    char buffer[1024] = {0}; 
        
    thread m (root); 
    sock = createSocket("Mi ip");
  
    ClientMessage m2;
    while(true){

        valread = read(sock, buffer, 1024);
        if ((buffer[0] != '\0') && (valread != 0)) {
            //printf("main: %s\n", buffer);
            //m2.ParseFromString(buffer);
            //printf("main: %d\n", binaryList.size());
            binaryList.push(buffer);
            //printf("main: %s\n", buffer);
            buffer[1024] = {0}; 
        }
    } 

    m.join(); 
    google::protobuf::ShutdownProtobufLibrary();
}*/

//void thread1(){
    
void thread2(){
    int flag = 1;
    int valread; 
    char buffer[1024] = {0}; 
    while(true){
        std::list<user>::iterator it = userList.begin();
        user tempUser = *it;
        std::list <queue<ClientMessage>>::iterator it2 = requestList.begin();
        queue<ClientMessage> tempQueue= *it2;
        ClientMessage m;
        for (int i = 0; i < userList.size(); i++){
            valread = read(tempUser.socket, buffer, 1024);
            if ((buffer[0] != '\0') && (valread != 0)) {
                //printf("main: %s\n", buffer);
                //m2.ParseFromString(buffer);
                //printf("main: %d\n", binaryList.size());
                m.ParseFromString(buffer);
                tempQueue.push(m);
                //printf("main: %s\n", buffer);
                buffer[1024] = {0}; 
            }
            std::advance(it, 1);
            std::advance(it2, 1);
            tempUser = *it;
            tempQueue= *it2;
        }   
    } 
}

int main (int argc, char **argv) { 
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    //start thread 2
    std::thread t2 (thread2); 
    while(true){
        //Establish socket 
        int socket = createSocket();
        int flag = 1;
        int valread; 
        char buffer[1024] = {0}; 
        ClientMessage m;

        while(flag){
            valread = read(socket, buffer, 1024);
            if ((buffer[0] != '\0') && (valread != 0)) {
                //printf("main: %s\n", buffer);
                //m2.ParseFromString(buffer);
                //printf("main: %d\n", binaryList.size());
                m.ParseFromString(buffer);
                //printf("main: %s\n", buffer);
                buffer[1024] = {0}; 
                flag = 0;
            }
        } 
        
        user tempUser ;
        tempUser.username = m.synchronize().username();
        tempUser.ip = m.synchronize().ip();
        tempUser.userId = threadCount;
        tempUser.status = "";
        tempUser.socket = socket;

        threadIdList.push_back(tempUser.userId);
        userList.push_back(tempUser);
        printf("User created\n");

        queue<ClientMessage> tempQueue;
        requestList.push_back(tempQueue);
                
        threadList.push_back(thread(foo, tempUser, threadCount));
        threadCount ++ ;
        printf("Thread added\n");
        google::protobuf::ShutdownProtobufLibrary();
    }
    t2.join();
}


