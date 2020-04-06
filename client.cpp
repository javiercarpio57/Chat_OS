# include <iostream>
# include <cstdlib>
# include <pthread.h>
# include <unistd.h>
# include <string>
# include "mensaje.pb.h"

using namespace chat;
using namespace std;

// int opcion = 0;
string input;

void *listen (void *args) {
    while (input != "salir") {
        cout << "Estoy escuchando." << endl;
        sleep(2);
    }
    cout << "Termine de escuchar." << endl;
}

void *user (void *args) {
    while (input != "salir") {
        cout << "Si desea terminar el chat, escribe: 'salir'" << endl;
        getline (cin, input);
    }
    
}

int main () {

    pthread_t threadListen;
    pthread_t threadUser;

    if (pthread_create(&threadListen, NULL, listen, NULL) || pthread_create(&threadUser, NULL, user, NULL)) {
        cout << "Error: unable to create threads." << endl;
        exit(-1);
    }

    pthread_join (threadListen, NULL);
    pthread_join (threadUser, NULL);

}


