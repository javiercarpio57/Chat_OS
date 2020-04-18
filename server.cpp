//g++ server.cpp mensaje.pb.cc -lprotobuf -lpthread -o server
#include <string>
#include <iostream>
#include <thread> 
#include <queue> 
#include <list>
#include "mensaje.pb.h"

using namespace std;
using namespace chat;

struct user {
   string username ;
   string ip;
   int userId;
   string status;
};
list<thread> threadList;
list<int> threadIdList;
list<user> userList;
list<queue<ClientMessage>> requestList;

int threadCount = 0;

user getUser(int id){
    std::list<user>::iterator it = userList.begin();
    user tempUser = *it;
    while (tempUser.userId != id) {
        std::advance(it, 1);
        tempUser = *it;
    }
    return *it;
}

void getConnectedUsers(connectedUserRequest cur){
    ConnectedUserResponse * response(new ConnectedUserResponse);
    if (cur.userid() == 0){
        //All users
        ConnectedUser * tempConectedUser;
        for (int i = 0; i < userList.size(); i++){
            user temporalUser = getUser(i); //Change for the new form
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
        user temporalUser = getUser(0); 
        tempConectedUser->set_userid(temporalUser.userId);
        tempConectedUser->set_username(temporalUser.username);
        tempConectedUser->set_status(temporalUser.status);
        tempConectedUser->set_ip(temporalUser.ip);
        
    }
}
void sendBroadcast(int id, string message){
    //Server response to sender 
    BroadcastResponse * response(new BroadcastResponse);
    response->set_messagestatus("Send");
    ServerMessage * m(new ServerMessage);
    m->set_option(7); 
    m->set_allocated_broadcastresponse(response);
    string binary;
    m->SerializeToString(&binary);
    //server response to everybody
    BroadcastMessage * globalResponse(new BroadcastMessage);
    globalResponse->set_message(message);
    globalResponse->set_userid(id);
    ServerMessage * gM(new ServerMessage);
    gM->set_option(1); 
    gM->set_allocated_broadcast(globalResponse);
    binary;
    gM->SerializeToString(&binary);
    //Add , send to everybody
}

void sendMessage(int id, string message){ 
    //Server response to sender 
    DirectMessageResponse * response(new DirectMessageResponse);
    response->set_messagestatus("Send");
    ServerMessage * m(new ServerMessage);
    m->set_option(7); 
    m->set_allocated_directmessageresponse(response);
    string binary;
    m->SerializeToString(&binary);
    //server response to everybody
    DirectMessage * directMessage(new DirectMessage);
    directMessage->set_message(message);
    directMessage->set_userid("id"); //fix proto should be int
    ServerMessage * pm (new ServerMessage);
    pm->set_option(1); 
    pm->set_allocated_message(directMessage);
    binary = "";
    pm->SerializeToString(&binary);
    //Add , send to user
}
//Thread code
void foo(user user, int id ) 
{
    MyInfoResponse * response(new MyInfoResponse);
    response->set_userid(id);
    ServerMessage * m(new ServerMessage);
    m->set_option(5); 
    m->set_allocated_myinforesponse(response);
    string binary;
    m->SerializeToString(&binary);
    printf("Response from server to client\n");
    //Asign requestList
    
    std::list<queue<ClientMessage>>::iterator it = requestList.begin();
    std::advance(it, id);
    queue<ClientMessage> request = *it;

    int acknowledgement = 0;
    //waiting for acknowledgement
    while(acknowledgement == 0){
        /*
        if (!request.empty()){
            ClientMessage temp = request.front();
            request.pop();
            if (temp.option() == ?") { //fix
                acknowledgement = 1
            } else {
                acknowledgement = -1 
            }
        }*/
        acknowledgement = 1 ;//Remove later
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
                    //getConnectedUsers(temp.connectedusers);
                break;
                case 3: //status change :Pending
                break;
                case 4: //sendBroadcast
                break;
                case 5: //sendMessage
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
    //Message setup
    MyInfoSynchronize * miInfo(new MyInfoSynchronize);
    miInfo->set_username("username123");
    miInfo->set_ip("127.0.0.1");
    ClientMessage * m(new ClientMessage);
    m->set_option(0);
    m->set_allocated_synchronize(miInfo);
    string binary;
    m->SerializeToString(&binary);

    queue<string> binaryList;
    binaryList.push(binary);
    // Message desynchronize
    ClientMessage m2;
    m2.ParseFromString(binaryList.front());
    binaryList.pop();
    //Fix: loop condition and add request list
    //while (true){
        if (m2.option() == 0){ //Change to entering message
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
        } else {
            //add message to corresponding pile
        }
    //}
    //Wait for all created threads to end
    
    std::thread temp;
    for (int i = 0; i< threadCount; i++){
        threadList.front().join();
        threadList.pop_front();
    }
    google::protobuf::ShutdownProtobufLibrary();
}

