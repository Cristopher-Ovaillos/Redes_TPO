#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


//Funcion de manejo de errores
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr; //Estructura de la direccion del servidor
    struct hostent *server; //Esctructura para la info del host
    char buffer[1024]; //Buffer de envio de datos

    //Verificacion de cantidad de argumentos correctos
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <hostname> <puerto>\n", argv[0]);
        exit(1);
    }

    //Obtener numero de puerto
    portno = atoi(argv[2]);
    // Crear socket para la comunicacion. AF_INET para IPv4, SOCK_STREAM para TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR al abrir el socket");

    // Obtiene la direccion del servidor usando el nombre del host proporcionado
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no se puede encontrar el servidor %s\n", argv[1]);
        exit(1);
    }
    // Inicializa la estructura serv_addr con ceros
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // Configura la familia de direcciones (IPv4)
    serv_addr.sin_family = AF_INET;
     // Copia la direccion IP del servidor al campo sin_addr.s_addr
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    // Convierte el numero de puerto a formato de red y lo asigna
    serv_addr.sin_port = htons(portno);
    // Establecer conexion con el servidor
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR de conexión");
    }
    //Pedido de solicitud
    printf("Ingrese su mensaje: ");
    bzero(buffer, 1024); //Limpia buffer
    fgets(buffer, 1024, stdin); //Lee entrada de usuario

    // Elimina el caracter de nueva linea del final del mensaje
    buffer[strcspn(buffer, "\n")] = 0;
    
    // Envia el mensaje al servidor
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        error("ERROR en la escritura");
    }
     // Limpiar buffer antes de recibir la respuesta
    bzero(buffer, 1024);
    // Leer la respuesta del servidor
    n = read(sockfd, buffer, 1024);
    if (n < 0) {
        error("ERROR en la lectura");
    }
    //Imprimir respuesta
    printf("Respuesta del servidor: %s\n", buffer);
    
    //Cerrar socket
    close(sockfd);
    return 0;
}
