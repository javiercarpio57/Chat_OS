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

string input;
int seg;
string estadoActual;
int sock = 0;

void *listen (void *args) {
    int sock = 0, valread;
    char buffer[1024] = {0}; 

    while ((input != "salir") || read(sock, buffer, 1024) != 0) {
        if (buffer[0] != '\0')
            printf("Heyyy: %s %d\n", buffer[0], sizeof(buffer));
    }

    cout << "Termine de escuchar." << endl;
}

void *user (void *args) {
    cout << "Si desea terminar el chat, escribe: 'salir'" << endl;
    cout << "Para obtener mas informacion sobre el uso del chat, escribe: 'info'" << endl;

    while (input != "salir") {
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
    while (input != "salir") {
        // if (seg < inactivoT) {
        //     sleep (1);
        //     seg++;
        //     // cout << "Seg: " << seg << endl;
        // } else {
        //     if (estadoActual != "INACTIVO")
        //         cambiarEstado("INACTIVO");
            
        // }
    }
}

// Si el usuario se puede inscribir al server...
// 1: Success
// 0: Error. Ya existe el usuario u otro error.
int sendInfoToServer(string nombre, string username, string ip, string puerto) {
    struct sockaddr_in serv_addr;

    char buffer[1024] = {0}; 
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
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

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
    //  ...... send(.....)


    return 1;
}

void *broadcastMessage (string message) {
    // Aqui se envia un mensaje a todos los usuarios
    BroadcastRequest broadcastMessage;
    broadcastMessage.set_message (message);

    ClientMessage clientMessage;
    clientMessage.set_option(4);
    clientMessage.set_allocated_broadcast (&broadcastMessage);

}

void *cambiarEstado (string nuevoEstado) {
    // Aqui se cambia a otro estado
    ChangeStatusRequest changeStatus;
    changeStatus.set_status (nuevoEstado);

    ClientMessage clientMessage;
    clientMessage.set_option (3);
    clientMessage.set_allocated_changestatus (&changeStatus);

    cout << "Estado nuevo: " << nuevoEstado << endl;
    estadoActual = nuevoEstado;
}

bool ifUsername (string word) {
    // si es usuario: true
    // si no es usuario: false
    return true;
}

void *getUserInfo (string username) {
    cout << BLUE << "Obteniendo info de " << username << "..." << RESET << endl;
    connectedUserRequest userRequest;
    // userRequest.set_userid(0) // Hay que asignarle un valor para el usuario.
    userRequest.set_username (username);

    ClientMessage clientMessage;
    clientMessage.set_option (2);
    clientMessage.set_allocated_connectedusers (&userRequest);
}

void *sendMessageToUser (string username, string message) {
    DirectMessageRequest directMessage;
    directMessage.set_username (username);
    directMessage.set_message (message);

    ClientMessage clientMessage;
    clientMessage.set_option (5);
    clientMessage.set_allocated_directmessage (&directMessage);

    
}

void *exit() {
    // Si se hace alguna accion para salir.
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


