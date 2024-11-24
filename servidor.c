#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 1024
#define NUM_POKEMON 9

//Definimos un Arreglo de Pokemones
const char *pokemon_db[NUM_POKEMON] = {
    "Bulbasaur: Tipo Planta/Veneno",
    "Ivysaur: Tipo Planta/Veneno",
    "Venusaur: Tipo Planta/Veneno",
    "Charmander: Tipo Fuego",
    "Charmeleon: Tipo Fuego",
    "Charizard: Tipo Fuego/Volador",
    "Squirtle: Tipo Agua",
    "Wartortle: Tipo Agua",
    "Blastoise: Tipo Agua"
};

//Mensaje de error
void error(const char *msg) {
    perror(msg);
    exit(1);
}

//Obtener pokemon por ID
char* get_pokemon_info(int pokemon_id) {
    static char response[200];

    if (pokemon_id >= 1 && pokemon_id <= NUM_POKEMON) {
        snprintf(response, sizeof(response), "%s", pokemon_db[pokemon_id - 1]);
        return response;
    } else {
        snprintf(response, sizeof(response), "Pokemon no encontrado.");
        return response;
    }
}

void handle_client(int newsockfd) {
    char buffer[MAX_BUFFER_SIZE];
    //Para evitar datos residuales, inicializa bytes en 0
    bzero(buffer, sizeof(buffer));

    // Leo la solicitud del cliente
    int n = read(newsockfd, buffer, MAX_BUFFER_SIZE);
    if (n < 0) {
        error("ERROR al leer del socket");
    }
    //Agrega caracter nulo de fin de cadena
    buffer[n] = '\0'; 
    //Puntero que apunta al inicio del buffer
    char *cleaned_buffer = buffer;
    //Eliminar caracteres no deseados a traves del Buffer
    while (*cleaned_buffer == ' ' || *cleaned_buffer == '\n' || *cleaned_buffer == '\r') {
        cleaned_buffer++; 
    }
    //Imprimir la solicitud recibida
    printf("Solicitud recibida: '%s'\n", cleaned_buffer);

    //Declaracion de variables de respuesta
    char *response = NULL;
    char header[200];

    /* Verifico si la solicitud es valida (si comienza con "GET /POKEMON/")
    Compara los primeros 13 Caracteres para ver si es GET /POKEMON/ */
    if (strncmp(cleaned_buffer, "GET /POKEMON/", 13) == 0) {
        int pokemon_id = 0;
        //Extraigo el ID del pokemon, apunto al texto que sigue despues de GET /POKEMON/
        if (sscanf(cleaned_buffer + 13, "%d", &pokemon_id) == 1) {
            //Imprimo el ID del Pokemon extraido
            printf("ID de Pokemon extraido: %d\n", pokemon_id);
            //Llamo a funcion get pokemon info para obtener los datos con el ID del pokemon
            response = get_pokemon_info(pokemon_id);

            // Preparar la respuesta HTTP 200 OK
            snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n");
            //Envio el encabezado y el cuerpo de la respuesta
            write(newsockfd, header, strlen(header));
            write(newsockfd, response, strlen(response));
        } else {
            // Si el ID no es valido, se responde con un error 400
            response = "Formato incorrecto. Usa GET /POKEMON/<ID>";
            snprintf(header, sizeof(header),
                     "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\n\n");
            write(newsockfd, header, strlen(header));
            write(newsockfd, response, strlen(response));
        }
    } else {
        // Si el comando no es reconocido, se responde con un error 404
        response = "Comando no reconocido.";
        snprintf(header, sizeof(header),
                 "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n");
        write(newsockfd, header, strlen(header));
        write(newsockfd, response, strlen(response));
    }
    //Cierro conexion
    close(newsockfd);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

     // Crea un socket para comunicacion. AF_INET indica IPv4, SOCK_STREAM para TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR al abrir el socket");

    // Inicializa la estructura del servidor en memoria con ceros
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // Obtener el numero de puerto del primer argumento del programa
    portno = atoi(argv[1]);
    // Configura la familia de direcciones (IPv4)
    serv_addr.sin_family = AF_INET;
    // Configura la direccion IP para aceptar conexiones de cualquier direccion (INADDR_ANY)
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // Configura el numero de puerto en formato de red
    serv_addr.sin_port = htons(portno);

    // Asocia el socket al puerto y direccion especificados en serv_addr
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR en bind");

    /* Configurar el socket en modo pasivo para escuchar conexiones
    El segundo argumento indica el tamaño de la cola de conexiones pendientes */
    listen(sockfd, 5);
    printf("Servidor esperando conexiones...\n");

    // Obtener el tamaño de la estructura del cliente.
    clilen = sizeof(cli_addr);
    while (1) {
        // Aceptar una conexion entrante y crea un nuevo socket para la comunicacion
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR en accept");

        handle_client(newsockfd);
    }
    // Cierra el socket principal del servidor
    close(sockfd);
    return 0;
}
