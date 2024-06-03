#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_MOVIES 3
#define NUM_QUOTES 5

// A 2D array to store movie quotes. Each row represents a movie, and each column represents a quote.
char *movie_sentences[NUM_MOVIES][NUM_QUOTES] = {
    {"Um anel para a todos governar", "Na terra de Mordor onde as sombras se deitam", "Não é o que temos, mas o que fazemos com o que temos", "Não há mal que sempre dure", "O mundo está mudando, senhor Frodo"},
    {"Vou fazer uma oferta que ele não pode recusar", "Mantenha seus amigos por perto e seus inimigos mais perto ainda", "É melhor ser temido que amado", "A vingança é um prato que se come frio", "Nunca deixe que ninguém saiba o que você está pensando"},
    {"Primeira regra do Clube da Luta: você não fala sobre o Clube da Luta", "Segunda regra do Clube da Luta: você não fala sobre o Clube da Luta", "O que você possui acabará possuindo você", "É apenas depois de perder tudo que somos livres para fazer qualquer coisa", "Escolha suas lutas com sabedoria"}};

// Structure to hold client information (socket, chosen movie, and quote index)
typedef struct
{
    int socket;               // Socket descriptor for communication with the client
    int chosen_movie;         // Index of the movie chosen by the client
    int quote_from_the_movie; // Index of the current quote being sent
} client_info;

int client_count = 0; // Global variable to track the number of active clients
pthread_mutex_t lock; // Mutex to synchronize access to the client_count variable

// Function to handle errors when receiving data from a client.
// Prints an error message and exits the current thread.
void handle_client_error(int bytes_received)
{
    if (bytes_received == -1)
    {
        perror("Erro ao receber dados do cliente");
        pthread_exit(NULL); // Exit the thread if there's an error
    }
}

// Function to increment the global client count.
// Uses a mutex to ensure thread-safe access to the client_count variable.
void increment_client_count()
{
    pthread_mutex_lock(&lock); // Acquire the lock before modifying client_count
    client_count++;
    pthread_mutex_unlock(&lock); // Release the lock after modifying client_count
}

// Function to decrement the global client count.
// Uses a mutex to ensure thread-safe access to the client_count variable.
void decrement_client_count()
{
    pthread_mutex_lock(&lock); // Acquire the lock before modifying client_count
    client_count--;
    pthread_mutex_unlock(&lock); // Release the lock after modifying client_count
}

// Function to handle a client connection.
// Arguments:
//   arg: A pointer to a client_info structure containing client information.
// Responsibilities:
//   1. Receives client information from the client_info structure.
//   2. Increments the global client count.
//   3. Iterates through the quotes of the chosen movie and sends them to the client.
//   4. Decrements the global client count.
//   5. Frees the allocated memory for the client_info structure.
//   6. Exits the thread.
void *handle_client(void *arg)
{
    client_info *info = (client_info *)arg;                                         // Cast the argument to a client_info pointer
    int serverSocket = info->socket;                                                // Extract the server socket descriptor
    struct sockaddr_storage clientAddress = *(struct sockaddr_storage *)(info + 1); // Extract the client address
    socklen_t addr_size = sizeof(clientAddress);                                    // Get the size of the client address structure

    // Receive the client's chosen movie and quote index
    int bytes_received = recvfrom(serverSocket, info, sizeof(*info), 0, (struct sockaddr *)&clientAddress, &addr_size);

    handle_client_error(bytes_received); // Check for errors during data reception

    increment_client_count(); // Increment the count of active clients

    // Iterate through and send quotes from the chosen movie
    for (int i = 0; i < NUM_QUOTES; i++)
    {
        char *sentence = movie_sentences[info->chosen_movie - 1][i];                                           // Get the quote based on the chosen movie index
        sendto(serverSocket, sentence, strlen(sentence) + 1, 0, (struct sockaddr *)&clientAddress, addr_size); // Send the quote to the client
        sleep(3);                                                                                              // Introduce a 3-second delay between sending quotes
    }

    decrement_client_count(); // Decrement the count of active clients

    free(info); // Release the memory allocated for the client_info structure

    return NULL; // Exit the thread
}

// Function to create a socket and listen for connections.
// Arguments:
//   family: The address family (AF_INET for IPv4 or AF_INET6 for IPv6)
//   port: The port number to listen on.
// Responsibilities:
//   1. Creates a UDP socket.
//   2. Binds the socket to the specified port and address family.
//   3. Enters a loop to continuously listen for incoming connections.
//   4. Creates a new thread to handle each incoming client connection.
void create_server_socket_and_listen(sa_family_t family, int port)
{
    int serverSocket;
    struct sockaddr_storage serverAddress;
    socklen_t addr_size;

    serverSocket = socket(family, SOCK_DGRAM, 0); // Create a UDP socket

    if (serverSocket == -1)
    {
        perror("Erro ao criar o socket");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddress, 0, sizeof(serverAddress)); // Clear the serverAddress structure

    // Configure the server address based on the address family
    if (family == AF_INET)
    {
        ((struct sockaddr_in *)&serverAddress)->sin_family = family;          // Set address family to IPv4
        ((struct sockaddr_in *)&serverAddress)->sin_port = htons(port);       // Set the port number (converted to network byte order)
        ((struct sockaddr_in *)&serverAddress)->sin_addr.s_addr = INADDR_ANY; // Bind to any available IPv4 address
        addr_size = sizeof(struct sockaddr_in);
    }
    else if (family == AF_INET6)
    {
        ((struct sockaddr_in6 *)&serverAddress)->sin6_family = family;                  // Set address family to IPv6
        ((struct sockaddr_in6 *)&serverAddress)->sin6_port = htons(port);               // Set the port number (converted to network byte order)
        inet_pton(family, "::", &(((struct sockaddr_in6 *)&serverAddress)->sin6_addr)); // Bind to any available IPv6 address
        addr_size = sizeof(struct sockaddr_in6);
    }

    // Bind the socket to the address and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, addr_size) == -1)
    {
        perror("Erro ao associar o socket ao endereço");
        exit(EXIT_FAILURE);
    }

    // Main loop to listen for incoming connections
    while (1)
    {
        struct sockaddr_storage clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        // Wait for data to arrive from a client (blocking call)
        recvfrom(serverSocket, NULL, 0, MSG_PEEK, (struct sockaddr *)&clientAddress, &clientAddressLength);

        // Create a new thread to handle the client connection
        pthread_t tid;
        client_info *info = malloc(sizeof(client_info) + sizeof(struct sockaddr_storage)); // Allocate memory for client_info and clientAddress
        info->socket = serverSocket;                                                       // Store the server socket descriptor in client_info

        memcpy(info + 1, &clientAddress, sizeof(clientAddress)); // Copy the client address into the allocated memory
        pthread_create(&tid, NULL, handle_client, info);         // Create a new thread and pass the client_info as an argument
    }

    close(serverSocket); // Close the server socket when the loop exits (which it shouldn't in a typical server implementation)
}

// Function to print the number of active clients every 4 seconds.
// Arguments:
//   arg: A void pointer (not used in this case).
// Responsibilities:
//   1. Enters an infinite loop.
//   2. Sleeps for 4 seconds.
//   3. Acquires the mutex lock to access the client_count variable safely.
//   4. Prints the current number of active clients.
//   5. Releases the mutex lock.
void *print_client_count(void *arg)
{
    while (1)
    {
        sleep(4); // Sleep for 4 seconds
        pthread_mutex_lock(&lock);              // Acquire the lock before accessing client_count
        printf("Clientes: %d\n", client_count); // Print the client count
        pthread_mutex_unlock(&lock);            // Release the lock after accessing client_count
    }
}

// Main function.
// Arguments:
//   argc: The number of command-line arguments.
//   argv: An array of strings containing the command-line arguments.
// Responsibilities:
//   1. Validates the number of command-line arguments.
//   2. Extracts the IP version (IPv4 or IPv6) and port number from the arguments.
//   3. Initializes the mutex.
//   4. Creates a thread to print the client count.
//   5. Calls the create_server_socket_and_listen function to start the server.
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Número inválido de argumentos\n");
        exit(EXIT_FAILURE);
    }

    char *ipVersion = argv[1]; // Get the IP version (e.g., "ipv4" or "ipv6")
    int port = atoi(argv[2]);  // Convert the port number to an integer

    // Validate the IP version
    if (strcmp(ipVersion, "ipv4") != 0 && strcmp(ipVersion, "ipv6") != 0)
    {
        printf("Versão de IP inválida\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&lock, NULL); // Initialize the mutex

    pthread_t tid;
    pthread_create(&tid, NULL, print_client_count, NULL); // Create a thread to print the client count

    // Start the server based on the IP version
    if (strcmp(ipVersion, "ipv4") == 0)
    {
        create_server_socket_and_listen(AF_INET, port); // Start the server in IPv4 mode
    }
    else if (strcmp(ipVersion, "ipv6") == 0)
    {
        create_server_socket_and_listen(AF_INET6, port); // Start the server in IPv6 mode
    }

    return EXIT_SUCCESS;
}