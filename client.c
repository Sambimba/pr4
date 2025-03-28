#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to server at %s:%d\n", server_ip, server_port);
    
    while (1) {
        int attempts = 0;
        
        printf("New game started. Try to guess number between 1 and 100.\n");
        
        while (1) {
            int guess;
            printf("Enter your guess (1-100): ");
            scanf("%d", &guess);
            
            // Проверка ввода
            if (guess < 1 || guess > 100) {
                printf("Please enter number between 1 and 100.\n");
                continue;
            }
            
            attempts++;
            
            // Отправка попытки
            int network_guess = htonl(guess);
            if (send(sock, &network_guess, sizeof(network_guess), 0) <= 0) {
                perror("send");
                close(sock);
                exit(EXIT_FAILURE);
            }
            
            // Получение ответа от сервера
            int response;
            ssize_t bytes_received = recv(sock, &response, sizeof(response), 0);
            if (bytes_received <= 0) {
                perror("recv");
                close(sock);
                exit(EXIT_FAILURE);
            }
            
            response = ntohl(response);
            
            if (response == 0) {
                printf("Congratulations! You guessed the number in %d attempts.\n", attempts);
                break;
            } else if (response < 0) {
                printf("Too low! Try again.\n");
            } else {
                printf("Too high! Try again.\n");
            }
        }
        
        // Спросить, хочет ли игрок сыграть еще
        printf("Play again? (1 - yes, 0 - no): ");
        int play_again;
        scanf("%d", &play_again);
        
        int network_play_again = htonl(play_again);
        if (send(sock, &network_play_again, sizeof(network_play_again), 0) <= 0) {
            perror("send");
            close(sock);
            exit(EXIT_FAILURE);
        }
        
        if (!play_again) {
            break;
        }
    }
    
    close(sock);
    return 0;
}
