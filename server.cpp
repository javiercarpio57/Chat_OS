//g++ server.cpp mensaje.pb.cc -lprotobuf -lpthread -o server
//g++ client.cpp mensaje.pb.cc -lprotobuf -lpthread -o client
#include <string>
#include <iostream>
#include <thread> 
#include <queue> 
#include <list>
#include <vector>
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
vector <int> userIdList;
vector <user> userList;
vector <queue<ClientMessage>> requestList;

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
    char buffer[msg.size() + 1] = {0};  
    strcpy(buffer, msg.c_str());
    //Prueba
    int bytesSen = send (sock, buffer, msg.size() + 1, 0);
    cout << bytesSen << ":" << msg.size() << ":" << (sizeof(buffer)/sizeof(*buffer)) << "\n";
}


user getUser(int id){
    user tempUser = userList[0];
    int cont = 0;
    while (tempUser.userId != id) {
        cont ++;
        tempUser = userList[cont];
    }
    return tempUser;
}

user getIdUsername(string username){
    user tempUser = userList[0];
    int cont = 0;
    //printf("largo: %d\n", userList.size());
    for (int i = 0; i < userList.size(); i++){
        string temp = userList[i].username;
        if (username.compare(temp) == 0) {
            tempUser = userList[i];
        }
    }
    return tempUser;
}

int getUserPos(int id){
    int cont = 0;
    cout << userIdList.size() << "\n";
    for (int i = 0; i < userIdList.size(); i++){
        if (userIdList[i] == id) {
            cont = i;
        }
    }
    return cont;
}

void changeStatusInList(int id, string status){
    user tempUser = userList[0];
    int cont = 0;
     for (int i = 0; i < userIdList.size(); i++){
        if (userIdList[i] == id) {
            tempUser = userList[cont];
            tempUser.status = status;
            userList[cont] = tempUser;
        }
    }
}

void getConnectedUsers(connectedUserRequest cur, int socket){
    ConnectedUserResponse * response(new ConnectedUserResponse);
    if (cur.userid() == 0){
        //All users
        
        for (int i = 0; i < userList.size(); i++){
            ConnectedUser * tempConectedUser;
            tempConectedUser = response->add_connectedusers();
            //cout << tempConectedUser << "," << i << "\n" ;
            user temporalUser = userList[i]; 

            tempConectedUser->set_userid(temporalUser.userId);
            tempConectedUser->set_username(temporalUser.username);
            tempConectedUser->set_status(temporalUser.status);
            tempConectedUser->set_ip(temporalUser.ip);
        }
    } else {
        /*
        //Single user
        ConnectedUser * tempConectedUser;
        tempConectedUser = response->add_connectedusers();
        user temporalUser = getUser(cur.userid()); 
        tempConectedUser->set_userid(temporalUser.userId);
        tempConectedUser->set_username(temporalUser.username);
        tempConectedUser->set_status(temporalUser.status);
        tempConectedUser->set_ip(temporalUser.ip);*/
    }
    
    ServerMessage * m(new ServerMessage);
    m->set_option(5); 
    m->set_allocated_connecteduserresponse(response);
    string binary;
    m->SerializeToString(&binary);
    sendBySocket(binary, socket);
    //cout << "el tamano es : "<< binary.length() << "\n";
    //cout << "Se mandaron : "<<m->connecteduserresponse().connectedusers_size() << "usuarios" << "\n";
    ServerMessage temp;
    temp.ParseFromString(binary);

    cout << "Se mandaron : "<<temp.connecteduserresponse().connectedusers_size() << "usuarios" << "\n";
    for (int i = 0; i < temp.connecteduserresponse().connectedusers_size(); i++) {
        ConnectedUser tmpUser = temp.connecteduserresponse().connectedusers(i);
        cout << "USERNAME: " << tmpUser.username() << endl;
        cout << "STATUS: " << tmpUser.status() << endl;
        cout << "USER ID: " << tmpUser.userid() << endl;
        cout << "IP: " << tmpUser.ip() << endl;
        cout << "----------------------------------" << endl;
    }
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
        user temporalUser = userList[i];
        printf("%d\n", temporalUser.userId);
        sendBySocket(binary, temporalUser.socket);
    }
}//Add , send to everybody

void sendMessage(string username , string message, int socket){ 
    //Server response to sender 
    DirectMessageResponse * response(new DirectMessageResponse);
    response->set_messagestatus("Send");
    ServerMessage * m(new ServerMessage);
    m->set_option(8); 
    m->set_allocated_directmessageresponse(response);
    string binary;
    m->SerializeToString(&binary);
    sendBySocket(binary, socket);
    //server response to person
    DirectMessage * directMessage(new DirectMessage);
    directMessage->set_message(message);
    directMessage->set_userid(0); //fix proto should be int
    ServerMessage * pm (new ServerMessage);
    pm->set_option(2); 
    pm->set_allocated_message(directMessage);
    binary = "";
    pm->SerializeToString(&binary);
    user temporalUser = getIdUsername(username);
    printf("%d\n", temporalUser.userId);
    sendBySocket(binary, temporalUser.socket);

}


void changeStatus(int id, string status, int socket){ 
    //Server response to sender 
    changeStatusInList(id, status);
    //server response to everybody
    ChangeStatusResponse * changeStatusResponse(new ChangeStatusResponse);
    changeStatusResponse->set_status(status);
    changeStatusResponse->set_userid(id);
    
    ServerMessage * pm (new ServerMessage);
    pm->set_option(6); 
    pm->set_allocated_changestatusresponse(changeStatusResponse);
    string binary;
    pm->SerializeToString(&binary);
    sendBySocket(binary, socket);
}

//Thread code
void foo(user user, int id ) 
{
    int mySock = user.socket;
    //Get request list 
    int mypos = getUserPos(id);
    queue<ClientMessage> request = requestList[mypos];
    
    int acknowledgement = 0;

    int valread; 
    char buffer[8096] = {0};
    
    
    MyInfoResponse * response(new MyInfoResponse);
    response->set_userid(id);
    ServerMessage * m(new ServerMessage);
    m->set_option(4); 
    m->set_allocated_myinforesponse(response);
    string binary;
    m->SerializeToString(&binary);
    sendBySocket(binary, mySock);
    
    printf("%d :Response from server to client\n", id);
    //printf("Prueba %d", getIdUsername(user.username).userId);
    ClientMessage mr;
    //waiting for acknowledgement
    while(acknowledgement == 0){
        valread = read(mySock, buffer, 8096);
        if ((buffer[0] != '\0') && (valread != 0)) {
            mr.ParseFromString(buffer);
            buffer[8096] = {0}; 
            if (mr.option() == 6) {
                acknowledgement = 1 ;
            } else {
                acknowledgement = -1 ;
            }
        }
        //acknowledgement = 1 ;//Remove later
    }
    if (acknowledgement == -1) {
        printf("Sali y falle\n");;
    }
    //**Fix: Add condition for failed acknowledgement**
    printf("Acknowledgement was recive\n");
    int working = 0;
    //waiting for request from user
    
    while(working == 0){
        ClientMessage temp;
            valread = read(mySock, buffer, 8096);
                if ((buffer[0] != '\0') && (valread != 0)) {
                    //m2.ParseFromString(buffer);
                    //printf("main: %d\n", binaryList.size());
                    temp.ParseFromString(buffer);
                    string prueba = buffer;
                    buffer[8096] = {0}; 
                    switch (temp.option()) {
                        
                        case 2: 
                            cout << temp.connectedusers().userid() << "\n"; 
                            cout << temp.connectedusers().username() << "\n";
                            getConnectedUsers(temp.connectedusers(), mySock);
                            printf("devolver usuarios \n");
                        break;

                        case 3: 
                            changeStatus(user.userId, temp.changestatus().status(), mySock);
                            printf("cambiar estado \n" );
                        break;
                        
                        case 4: 
                            cout << "Recibi largo: " << prueba.size() << "\n";
                            cout << temp.broadcast().message() << "," << temp.directmessage().message().length() << "\n"; 
                            sendBroadcast(user.userId, temp.broadcast().message(), mySock);
                            printf("broadcast \n");
                        break;
                        
                        case 5:
                            cout << "Recibi largo: " << prueba.size() << "\n";
                            cout << temp.directmessage().message() << "," << temp.directmessage().message().length() << "\n"; 
                            cout << temp.directmessage().username()  << "," << temp.directmessage().username().length()  << "\n";
                            cout << temp.directmessage().userid() << "\n";
                            sendMessage(temp.directmessage().username(),temp.directmessage().message(), mySock);
                            printf("mandar privado \n");
                        break;
                        default:
                        ;
                    //Error handling
            }
        }
    }
}

int main (int argc, char **argv) { 
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    //start thread 2
    //thread t2 (thread2);
    printf("Ya se creo la thread\n") ;
    while(true){
        //Establish socket 
        int socket = createSocket();
        int flag = 1;
        int valread; 
        char buffer[8096] = {0}; 
        ClientMessage m;

        while(flag){
            valread = read(socket, buffer, 8096);
            if ((buffer[0] != '\0') && (valread != 0)) {
                //printf("main: %s\n", buffer);
                //m2.ParseFromString(buffer);
                //printf("main: %d\n", binaryList.size());
                m.ParseFromString(buffer);
                //printf("main: %s\n", buffer);
                buffer[8096] = {0}; 
                flag = 0;
            }
        } 
        threadCount ++ ;
        user tempUser ;
        tempUser.username = m.synchronize().username();
        tempUser.ip = m.synchronize().ip();
        tempUser.userId = threadCount;
        tempUser.status = "ACTIVO";
        tempUser.socket = socket;
        
        //threadIdList.push_back(tempUser.userId);
        userIdList.push_back(tempUser.userId);
        userList.push_back(tempUser);
        printf("User created\n");

        queue<ClientMessage> tempQueue;
        requestList.push_back(tempQueue);
                
        threadList.push_back(thread(foo, tempUser, threadCount));
        
        printf("Thread added\n");
        google::protobuf::ShutdownProtobufLibrary();
    }
    //t2.join();
}


