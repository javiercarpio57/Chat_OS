
#include <string>
#include <iostream>
#include "mensaje.pb.h"
using namespace std;
using namespace chat;

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // Se crea instacia tipo MiInfoRequest y se setean los valores deseables
    MiInfoRequest* miInfo(new MiInfoRequest);
    miInfo->set_username("username123");
    miInfo->set_ip("12356789");

    // Se crea instancia de Mensaje, se setea los valores deseados
    Mensaje m;
    m.set_option("0");
    m.set_allocated_miinforeq(miInfo);

    // Se serializa el message a string
    string binary;
    m.SerializeToString(&binary);

    // SEND CON SOKETS
    /*
    char cstr[binary.size() + 1];
    strcpy(cstr, binary.c_str());

    send(sock , cstr , strlen(cstr) , 0 );
    */

    /* -----------------------------------------------------------------------------------------------------------------
        YA CON EL "MENSAJE" SERIALIZADO A STRING SE PUEDE ENVIAR UTILIZANDO SOCKETS CON send()
        EL RECEPTOR RECIBIRÁ EL STRING SERIALIZADO, Y ESTE LO PUEDE DESEREALIZAR A UN OBJETO "MENSAJE".
        HACER DE CUENTA QUE LO DE ARRIBA ES EL PROGRAMA DEL CLIENTE Y LO DE ABAJO ES EL SERVIDOR
    */ //---------------------------------------------------------------------------------------------------------------

    // RECEIVE CON SOKETS
    /*

    valread = read( new_socket , buffer, 8192);
	string ret(buffer, 8192);
    
    */

    // Se deserealiza el string a un objeto Mensaje
    Mensaje m2;
    m2.ParseFromString(binary);

    // Se puede accesar a los valores de la siguiente manera:
    cout << "Option: " << m2.option() << endl;
    cout << "Username: " << m2.miinforeq().username() << endl;
    cout << "ip: " << m2.miinforeq().ip() << endl;

    google::protobuf::ShutdownProtobufLibrary();
    return 1;
}