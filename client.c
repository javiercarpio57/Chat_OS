#include <stdio.h>
// #include <unistd.h>

int main () {
    int option;

    printf("---------------------------\n");
    printf("1. Chatear con todos los usuarios.\n");
    printf("2. Enviar y/o recibir mensaje directo.\n");
    printf("3. Cambiar de status.\n");
    printf("4. Ver usuarios conectados.\n");
    printf("5. Desplegar informacion de usuario.");
    printf("6. Ayuda.");
    printf("7. Salir.");
    printf("---------------------------\n");
    scanf("%d", &option);

    printf("Opcion: %d", option);
    
    return (0);
}