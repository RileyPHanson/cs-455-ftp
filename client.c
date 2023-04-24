#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_SIZE 1024

void receive_data(int sock_fd, char *buffer, size_t buffer_size) {
    int bytes_received = 0;
    while (bytes_received <= 0) {
        bytes_received = recv(sock_fd, buffer, buffer_size, 0);
    }
    buffer[bytes_received] = '\0';
}

int main() {
    // Get user input for server address, port
    char server_address[MAX_SIZE];
    int server_port;
    char file[MAX_SIZE];
    char dir[MAX_SIZE];
    
    printf("Enter server address: ");
    scanf("%s", server_address);
    printf("Enter server port: ");
    scanf("%d", &server_port);


    // Create a socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("invalid server address");
        exit(EXIT_FAILURE);
    }
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    // Receive the server's welcome message
    char buffer[MAX_SIZE];
    receive_data(sock_fd, buffer, MAX_SIZE);
    printf("%s", buffer);

    // Get the users commands
    int running = 1;
    char input[MAX_SIZE];
    char destination[MAX_SIZE];
    char fileOrDir[MAX_SIZE];
    int i;
    

    while(running) {
        printf("Please enter a command: CWD 'dir', STOP, RETR 'file'\n");
        fgets(input, sizeof(input), stdin);
        
        // Find the command keyword
        for (i = 0; i < sizeof(input); i++) {
            if (input[i] == ' ' || input[i] == '\0' || input[i] == '\n') {
                break;
            }
            destination[i] = input[i];
        }   
        destination[i] = '\0';

        // Stop if STOP command is used
        if(strcmp(destination, "STOP") == 0) {
            running = 0;
        }

        // Find the file/Dir
        for (i = 0; i < sizeof(input); i++) {
            if (input[i] == ' ') {
                int j = 0;
                for(i = i + 1; i < sizeof(input); i++) {
                    if(input[i] == '\n' || input[i] == '\0'){
                        break;
                    }
                    fileOrDir[j] = input[i];
                    j++;
                }   
                break;            
            }
        }   
        // Change Dir
        if (strcmp(destination, "CWD") == 0) {
            sprintf(buffer, "CWD %s\r\n", fileOrDir);
            send(sock_fd, buffer, strlen(buffer), 0);
            receive_data(sock_fd, buffer, MAX_SIZE);
            printf("%s", buffer);
        } 
        // Get file
        else if (strcmp(destination, "RETR") == 0) {
            sprintf(buffer, "RETR %s\r\n", fileOrDir);
            send(sock_fd, buffer, strlen(buffer), 0);
            receive_data(sock_fd, buffer, MAX_SIZE);
            printf("%s", buffer);

            // Open a file to write the data to
            FILE *fp = fopen(fileOrDir, "wb");
            if (fp == NULL) {
                perror("file opening failed");
                exit(EXIT_FAILURE);
            }

            // Read the data from the socket and write it to the file
            int bytes_received = 0;
            while ((bytes_received = recv(sock_fd, buffer, MAX_SIZE, 0)) > 0) {
                fwrite(buffer, 1, bytes_received, fp);
            }
            fclose(fp);
        }
    }

    // Close the socket
    close(sock_fd);

    return 0;

}
