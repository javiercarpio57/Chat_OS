#include "SERVER.h"

using namespace std;

void main()
{
    //main locals
    long SUCCESFUL;
    WSAData WinSockData;
    WORD DLLVERSION;

    //WORDS = objects of a data size processir
    DLLVERSION = MAKEWORD(2, 1);

    //Start Winsock DLL
    SUCCESFUL = WSAStartup(DLLVERSION, &WinSockData);
    
    //Create Socket Structure
    SOCKADDRE_IN ADDRESS;
    int AddressSize = sizeof(ADDRESS);

    //Create Sockets
    SOCKET sock_LISTEN;
    SOCKET sock_CONNECTION;

    //socket Arguments: AF_INET = internet domain
    //SOCK_STREAM = connection oriented TCP (Not SOCK_DGRAM)
    sock_CONNECTION = socket(AF_INET, SOCK_STREAM, NULL);
    ADDRESS.sin_addr.s_addr = inet_addr("127.0.0.1");
    ADDRESS.sin_family = AF_INET;
    ADDRESS.sin_port = htons(444);

    sock_LISTEN = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sock_LISTE, (SOCKADDR*)&ADDRESS, sizeof(ADDRESS));
    listen(sock_LISTEN, SOMAXCONN);

    //Si conecta
    for (;;){
        cout << "\n\tSERVER: Waiting for connection...";

        if(sock_CONNECTION = accept(sock_LISTEN, (SOCKADDR*)&ADDRESS, &AddressSize))
        {
            cout << "\n\tA connection was found!" << endl;

            SUCCESFUL = send(sock_CONNECTION, "Welcome! You have connected to Banana SERVER!", 46, NULL);
        }
    }

}