#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_SIZE 100

// Structure to hold client information (socket, chosen movie, and quote index)
typedef struct
{
    int socket;               // Socket descriptor for communication with the server
    int chosen_movie;         // Index of the movie chosen by the client
    int quote_from_the_movie; // Index of the current quote being sent
} client_info;

// Function to create a UDP socket.
// Arguments:
//   family: The address family (AF_INET for IPv4 or AF_INET6 for IPv6)
// Returns:
//   The created socket descriptor, or -1 on error.
int create_socket(sa_family_t family)
{
    int clientSocket = socket(family, SOCK_DGRAM, 0); // Create a UDP socket
    if (clientSocket == -1)
    {
        perror("Erro ao criar o socket"); // Print an error message if socket creation fails
        exit(EXIT_FAILURE);               // Exit the program if there's an error
    }
    return clientSocket; // Return the socket descriptor on success
}

// Function to connect the socket to the server.
// Arguments:
//   clientSocket: The client socket descriptor.
//   family: The address family (AF_INET for IPv4 or AF_INET6 for IPv6).
//   ipAddress: The IP address of the server.
//   port: The port number of the server.
//   info: A client_info structure containing client information.
// Responsibilities:
//   1. Configures the server address structure based on the address family.
//   2. Sends the client_info structure to the server.
//   3. Receives and prints quotes from the server.
void connect_socket(int clientSocket, sa_family_t family, char *ipAddress, int port, client_info info)
{
    struct sockaddr_storage serverAddress; // Structure to hold the server address
    socklen_t addr_size;                   // Size of the server address structure

    memset(&serverAddress, 0, sizeof(serverAddress)); // Clear the serverAddress structure

    // Configure the server address based on the address family
    if (family == AF_INET)
    {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&serverAddress;
        addr_in->sin_family = family;                       // Set address family to IPv4
        addr_in->sin_port = htons(port);                    // Set the port number (converted to network byte order)
        inet_pton(family, ipAddress, &(addr_in->sin_addr)); // Convert the IP address string to binary form and store it
        addr_size = sizeof(struct sockaddr_in);
    }
    else if (family == AF_INET6)
    {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&serverAddress;
        addr_in6->sin6_family = family;                       // Set address family to IPv6
        addr_in6->sin6_port = htons(port);                    // Set the port number (converted to network byte order)
        inet_pton(family, ipAddress, &(addr_in6->sin6_addr)); // Convert the IP address string to binary form and store it
        addr_size = sizeof(struct sockaddr_in6);
    }

    // Send the client information to the server
    sendto(clientSocket, &info, sizeof(info), 0, (struct sockaddr *)&serverAddress, addr_size);

    char serverResponse[MAX_SIZE]; // Buffer to store the server's response

    // Receive and print quotes from the server
    for (int i = 0; i < 5; i++)
    {
        recvfrom(clientSocket, serverResponse, MAX_SIZE, 0, NULL, NULL); // Receive data from the server (blocking call)
        printf("%s\n", serverResponse);                                  // Print the received quote
    }

    close(clientSocket); // Close the client socket
}

// Function to allow the user to choose a movie and initiate communication with the server.
// Arguments:
//   ipVersion: The IP version (IPv4 or IPv6).
//   ipAddress: The IP address of the server.
//   port: The port number of the server.
// Responsibilities:
//   1. Presents a menu of movie options to the user.
//   2. Reads the user's choice.
//   3. Validates the user's input.
//   4. Creates a UDP socket based on the IP version.
//   5. Initializes a client_info structure.
//   6. Calls the connect_socket function to establish communication with the server.
void choose_movie(char *ipVersion, char *ipAddress, int port)
{
    while (1) // Loop to allow the user to choose multiple movies
    {
        printf("0 - Sair\n");
        printf("1 - Senhor dos aneis\n");
        printf("2 - O poderoso chefão\n");
        printf("3 - Clube da luta\n");

        int clientAction;
        if (scanf("%d", &clientAction) != 1) // Read the user's choice
        {
            fprintf(stderr, "Erro na entrada\n"); // Print an error message if input is invalid
            exit(EXIT_FAILURE);                   // Exit the program if there's an input error
        }

        if (clientAction == 0)
        {
            break; // Exit the loop if the user chooses to exit
        }
        else if (clientAction >= 1 && clientAction <= 3) // Validate the user's choice
        {
            sa_family_t family = strcmp(ipVersion, "ipv4") == 0 ? AF_INET : AF_INET6; // Determine the address family based on the IP version

            int clientSocket = create_socket(family); // Create a UDP socket

            client_info info = {clientSocket, clientAction, 0}; // Initialize the client_info structure

            connect_socket(clientSocket, family, ipAddress, port, info); // Connect to the server and receive quotes
        }
        else
        {
            printf("Opção inválida! Digite um número entre 1 e 3 para escolher um filme ou 0 para sair\n"); // Inform the user if the choice is invalid
        }
    }
}

// Main function.
// Arguments:
//   argc: The number of command-line arguments.
//   argv: An array of strings containing the command-line arguments.
// Responsibilities:
//   1. Validates the number of command-line arguments.
//   2. Extracts the IP version, IP address, and port number from the arguments.
//   3. Calls the choose_movie function to initiate client-server interaction.
int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Número inválido de argumentos\n");
        exit(EXIT_FAILURE); // Exit if the number of arguments is incorrect
    }

    char *ipVersion = argv[1]; // Get the IP version (e.g., "ipv4" or "ipv6")
    char *ipAddress = argv[2]; // Get the server's IP address
    int port = atoi(argv[3]);  // Convert the port number to an integer

    // Validate the IP version
    if (strcmp(ipVersion, "ipv4") != 0 && strcmp(ipVersion, "ipv6") != 0)
    {
        printf("Versão de IP inválida\n");
        exit(EXIT_FAILURE); // Exit if the IP version is invalid
    }

    choose_movie(ipVersion, ipAddress, port); // Start the client-server interaction

    return EXIT_SUCCESS; // Indicate successful program execution
}