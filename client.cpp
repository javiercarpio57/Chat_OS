# include <iostream>
# include <cstdlib>
# include <pthread.h>
# include <unistd.h>
# include <string>
# include <bits/stdc++.h> 
# include <sys/socket.h> 
# include <arpa/inet.h> 
# include "mensaje.pb.h"
# include <chrono>

# define RESET   "\033[0m"
# define BLACK   "\033[30m"      // Black 
# define RED     "\033[31m"      // Red 
# define GREEN   "\033[32m"      // Green 
# define YELLOW  "\033[33m"      // Yellow 
# define BLUE    "\033[34m"      // Blue 
# define MAGENTA "\033[35m"      // Magenta 
# define CYAN    "\033[36m"      // Cyan 
# define WHITE   "\033[37m"      // White 
# define BOLDBLACK   "\033[1m\033[30m"      // Bold Black 
# define BOLDRED     "\033[1m\033[31m"      // Bold Red 
# define BOLDGREEN   "\033[1m\033[32m"      // Bold Green 
# define BOLDYELLOW  "\033[1m\033[33m"      // Bold Yellow 
# define BOLDBLUE    "\033[1m\033[34m"      // Bold Blue 
# define BOLDMAGENTA "\033[1m\033[35m"      // Bold Magenta 
# define BOLDCYAN    "\033[1m\033[36m"      // Bold Cyan 
# define BOLDWHITE   "\033[1m\033[37m"      // Bold White 

#define inactivoT 5

using namespace chat;
using namespace std;
using namespace chrono;

string getFirstWord (string phrase);
string getMessageFromPhrase (string phrase, string toErase);
void *showInfo ();
void *broadcastMessage (string message);
void *cambiarEstado (string nuevoEstado);
bool ifUsername (string word);
void *getUserInfo (string username);
void *sendMessageToUser (string username, string message);
void *exit();
void *sendBySocket(string msgToServer);
void *getAllUsers();

string input;
int seg;
string estadoActual;
int sock = 0;
bool isAlive = true;
bool askChangeStatus = false;
bool hasConnected = false;

int userId;

void *listen (void *args) {
    int valread;

    while (isAlive) {
        char buffer[1024] = {0}; 
        valread = read(sock, buffer, 1024);
        if ((buffer[0] != '\0') && (valread != 0)) {
            ServerMessage serverMessage;

            serverMessage.ParseFromString (buffer);

            switch (serverMessage.option()) {
            case 1: {
                string message = serverMessage.broadcast().message();
                int id = serverMessage.broadcast().userid();

                cout << CYAN << "(" << id << "): " << RESET << GREEN << message << RESET << endl;
                break;
            }
            case 2: {
                string message = serverMessage.message().message();
                int id = serverMessage.message().userid();

                cout << BLUE << "(" << id << " en privado):" << RESET << GREEN << message << RESET << endl;
                break;
            }
            case 3: {
                string error = serverMessage.error().errormessage();

                cout << RED << "ERROR: " << error << RESET << endl;
                break;
            }
            case 4: {
                userId = serverMessage.myinforesponse().userid();

                MyInfoAcknowledge *myInfoAcknowledge = new MyInfoAcknowledge;
                myInfoAcknowledge -> set_userid(userId);

                ClientMessage clientMessage;
                clientMessage.set_option (6);
                clientMessage.set_allocated_acknowledge (myInfoAcknowledge);

                string msgToServer;
                clientMessage.SerializeToString (&msgToServer);
                sendBySocket (msgToServer);

                hasConnected = true;
                break;
            }
            case 5: {
                cout << BLUE << "Los usuarios conectados son: " << RESET << endl;
                for (int i = 0; i < serverMessage.connecteduserresponse().connectedusers_size(); i++) {
                    ConnectedUser tmpUser = serverMessage.connecteduserresponse().connectedusers(i);
                    cout << BLUE << "----------------------------------" << RESET << endl;
                    cout << BLUE << "\tUSERNAME: " << tmpUser.username() << RESET << endl;
                    cout << BLUE << "\tSTATUS: " << tmpUser.status() << RESET << endl;
                    cout << BLUE << "\tUSER ID: " << tmpUser.userid() << RESET << endl;
                    cout << BLUE << "\tIP: " << tmpUser.ip() << RESET << endl;
                    cout << BLUE << "----------------------------------" << RESET << endl;
                }
                break;
            }
            case 6: {
                int id = serverMessage.changestatusresponse().userid();
                string status = serverMessage.changestatusresponse().status();

                cout << YELLOW << "Estado nuevo: " << status << RESET << endl;
                estadoActual = status;            
                break;
            }
            case 7: {
                string messageStatus = serverMessage.broadcastresponse().messagestatus();

                cout << YELLOW << "Broadcast message: " << messageStatus << RESET << endl;
                askChangeStatus = false;
                break;
            }
            case 8: {
                string messageStatus = serverMessage.directmessageresponse().messagestatus();

                cout << YELLOW << "Direct message: " << messageStatus << RESET << endl;
                break;
            }
            default:
                break;
            }
        }
    }

    cout << "\nTermine de escuchar.\n" << endl;
}

void *user (void *args) {
    cout << "Si desea terminar el chat, escribe: 'salir'" << endl;
    cout << "Para obtener mas informacion sobre el uso del chat, escribe: 'info'" << endl;

    while (isAlive) {
        getline (cin, input);
        seg = 0;

        string word = getFirstWord (input);
        cout << "Accion: " << word << endl;


        string message = getMessageFromPhrase (input, word);
        cout << "Mensaje: " << message << endl;

        if (word == "info")
            showInfo ();
        else if (word == "broadcast")
            broadcastMessage (message);
        else if (word == "estado")
            cambiarEstado (message);
        else if (word == "salir")
            exit();
        else if (word == "usuarios")
            getAllUsers ();
        else
            if (ifUsername) {
                if (message == "")
                    getUserInfo (word);
                else
                    sendMessageToUser(word, message);
            } else {
                cout << endl;
            }
    }
}

void *checkState(void *args) {
    while (isAlive) {
        // if (hasConnected) {
        //     if (seg < inactivoT) {
        //         sleep (1);
        //         seg++;
        //     } else {
        //         if ((estadoActual != "INACTIVO") && (!askChangeStatus)) {
        //             cambiarEstado("INACTIVO");
        //             askChangeStatus = true;
        //         }
        //     }
        // }
    }
}

void *sendBySocket (string msg) {
    char buffer[1024] = {0};
    strcpy(buffer, msg.c_str());

    send (sock, buffer, msg.size() + 1, 0);
}

int connectToServer (string nombre, string username, string ip, string puerto) {
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(stoi(puerto)); 
       
    char newIP[ip.size() + 1];
    strcpy(newIP, ip.c_str());

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, newIP, &serv_addr.sin_addr)<=0)  
    { 
        cout << RED << "\nInvalid address/ Address not supported \n" << RESET << endl;
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        cout << RED << "\nConnection Failed\n" << RESET << endl;
        return -1; 
    }
}

// Si el usuario se puede inscribir al server...
// 1: Success
// 0: Error. Ya existe el usuario u otro error.
int sendInfoToServer(string nombre, string username, string ip, string puerto) {
    connectToServer (nombre, username, ip, puerto);

    // Aqui se pone el codigo para enviar al server.
    // ConnectedUser miUsuario;
    // miUsuario.set_username (username);
    // miUsuario.set_status (estadoActual);
    // miUsuario.set_ip (ip);

    // Enviar al servidor miUsuario.
    MyInfoSynchronize * myInfo(new MyInfoSynchronize);
    myInfo->set_username(username);
    myInfo->set_ip(ip);

    ClientMessage clientMessage;
    clientMessage.set_option (1);
    clientMessage.set_allocated_synchronize(myInfo);
    
    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
    return 1;
}

void *broadcastMessage (string message) {
    // Aqui se envia un mensaje a todos los usuarios
    BroadcastRequest *broadcastMessage = new BroadcastRequest();
    broadcastMessage->set_message (message);

    ClientMessage clientMessage;
    clientMessage.set_option(4);
    clientMessage.set_allocated_broadcast (broadcastMessage);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
}

void *cambiarEstado (string nuevoEstado) {
    // Aqui se cambia a otro estado
    ChangeStatusRequest *changeStatus = new ChangeStatusRequest();
    changeStatus->set_status (nuevoEstado);

    ClientMessage clientMessage;
    clientMessage.set_option (3);
    clientMessage.set_allocated_changestatus (changeStatus);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
    cout << "Cambiando de estado..." << endl;
}

bool ifUsername (string word) {
    // si es usuario: true
    // si no es usuario: false
    return true;
}

void *getUserInfo (string username) {
    cout << BLUE << "Obteniendo info de " << username << "..." << RESET << endl;
    connectedUserRequest *userRequest = new connectedUserRequest();
    // userRequest.set_userid(0) // Hay que asignarle un valor para el usuario.
    userRequest -> set_username (username);

    ClientMessage clientMessage;
    clientMessage.set_option (2);
    clientMessage.set_allocated_connectedusers (userRequest);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
}

void *sendMessageToUser (string username, string message) {
    DirectMessageRequest *directMessage = new DirectMessageRequest();
    directMessage -> set_username (username);
    directMessage -> set_message (message);

    ClientMessage clientMessage;
    clientMessage.set_option (5);
    clientMessage.set_allocated_directmessage (directMessage);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
}

void *getAllUsers () {
    connectedUserRequest *userRequest = new connectedUserRequest();
    userRequest -> set_userid(0);

    ClientMessage clientMessage;
    clientMessage.set_option (2);
    clientMessage.set_allocated_connectedusers (userRequest);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
}

void *exit() {
    // Si se hace alguna accion para salir.
    cout << "Saliendo..." << endl;
    isAlive = false;
    exit(0);
}

// Obtiene la primera palabra de 'phrase'
string getFirstWord (string phrase) {
    istringstream ss (phrase);
    string word;
    ss >> word;

    return word;
}

// Obtiene el mensaje de 'phrase'
string getMessageFromPhrase (string phrase, string toErase) {
    size_t pos = phrase.find(toErase);

    if (pos != string::npos) 
        phrase.erase (pos, toErase.length() + 1);

    return phrase;
}

void *showInfo () {
    cout << YELLOW << "---------------------------------- INFO ----------------------------------" << RESET << endl;
    cout << GREEN << "Este es un chat creado para el curso de sistemas operativos." << RESET << endl;
    cout << GREEN "Debes escribir un mensaje utilizando palabras clave para enviar los mensajes correctamente." << RESET << endl;
    cout << GREEN "Las palabras clave estan encerradas en comillas simples ('')." << RESET << endl;
    cout << GREEN "\t" << BOLDYELLOW << "'info'" << GREEN << ": para solicitar informacion de como usar el chat." << RESET << endl;
    cout << GREEN "\t" << BOLDYELLOW << "'salir'" << GREEN << ": para desconectarse del chat." << RESET << endl;
    cout << GREEN "\t" << BOLDYELLOW << "'usuarios'" << GREEN << ": obtener todos los usuarios conectados." << RESET << endl;
    cout << GREEN "\t" << BOLDYELLOW << "<username>" << GREEN << ": al ingresar el username de un usuario conectado, puedes ver informacion de el. " << RESET << endl;
    cout << GREEN "\t" << BOLDYELLOW << "<username> <mensaje>" << GREEN << ": para enviar el <mensaje> al usuario <username>. " << RESET << endl;
    cout << GREEN "\t" << BOLDYELLOW << "'broadcast' <mensaje>" << GREEN << ": para enviar el <mensaje> a todos los usuarios conectados." << RESET << endl;
    cout << GREEN "\t" << BOLDYELLOW << "'estado' <nuevo estado>" << GREEN << ": para cambiar tu estado actual a <nuevo estado>" << RESET << endl;

    cout << YELLOW << "-------------------------------------------------------------------------" << RESET << endl;
}

int main (int argc, char **argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    if (argc > 4) {

        string nombre = argv[1];
        string username = argv[2];
        string ip = argv[3];
        string puerto = argv[4];

        cout << "Nombre: " << nombre << endl;
        cout << "Username: " << username << endl;
        cout << "IP: " << ip << endl;
        cout << "puerto: " << puerto << endl;
        seg = 0;

        estadoActual = "ACTIVO";

        cout << "---------------------------------" << endl;

        // Se inicializa el usuario con el server si existe.
        if (sendInfoToServer(nombre, username, ip, puerto) != 0) {
            pthread_t threadListen;
            pthread_t threadUser;
            pthread_t threadState;

            if (pthread_create(&threadListen, NULL, listen, NULL) 
            || pthread_create(&threadUser, NULL, user, NULL) 
            || pthread_create(&threadState, NULL, checkState, NULL)) {
                cout << "Error: unable to create threads." << endl;
                exit(-1);
            }

            pthread_join (threadListen, NULL);
            pthread_join (threadUser, NULL);
            pthread_join (threadState, NULL);
        }
    }

    google::protobuf::ShutdownProtobufLibrary();
    return 1;
}


