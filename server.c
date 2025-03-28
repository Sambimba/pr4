#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUFFER_SIZE 1024

void log_message(const char* client_ip, const char* message) {
    printf("%s:%s\n", client_ip, message);
}

void handle_client(int client_sock, struct sockaddr_in client_addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    
    char buffer[BUFFER_SIZE];
    srand(time(NULL));
    
    while (1) {
        int secret_number = rand() % 100 + 1;
        int attempts = 0;
        int guess;
        
        log_message(client_ip, "New game started. Number generated.");
        
        while (1) {
            ssize_t bytes_received = recv(client_sock, &guess, sizeof(guess), 0);
            if (bytes_received <= 0) {
                log_message(client_ip, "Client disconnected.");
                close(client_sock);
                return;
            }
            
            guess = ntohl(guess);
            attempts++;
            
            if (guess < secret_number) {
                int response = htonl(-1);
                send(client_sock, &response, sizeof(response), 0);
                log_message(client_ip, "Client guessed too low.");
            } else if (guess > secret_number) {
                int response = htonl(1);
                send(client_sock, &response, sizeof(response), 0);
                log_message(client_ip, "Client guessed too high.");
            } else {
                int response = htonl(0);
                send(client_sock, &response, sizeof(response), 0);
                sprintf(buffer, "Correct guess in %d attempts!", attempts);
                log_message(client_ip, buffer);
                break;
            }
        }
        
        int play_again;
        ssize_t bytes_received = recv(client_sock, &play_again, sizeof(play_again), 0);
        if (bytes_received <= 0) {
            log_message(client_ip, "Client disconnected.");
            close(client_sock);
            return;
        }
        
        play_again = ntohl(play_again);
        if (!play_again) {
            log_message(client_ip, "Client chose to quit.");
            close(client_sock);
            return;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);
    if (port < IPPORT_RESERVED) {
        fprintf(stderr, "Port must be >= %d\n", IPPORT_RESERVED);
        exit(EXIT_FAILURE);
    }
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_sock, 5) < 0) {
        perror("listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d. Waiting for connections...\n", port);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client connected from %s\n", client_ip);
        
        handle_client(client_sock, client_addr);
    }
    
    close(server_sock);
    return 0;
}
