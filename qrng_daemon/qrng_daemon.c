#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <qrng.h>

#define SOCKET_PATH "/tmp/qrng_sock"
#define LOG_FILE "qrng_log"


static int server_sock = 0;


static void start_server(void);
static void run_server(int client_sock);

static void initialize_qrng(void);
static void generate_numbers(int32_t *numbers, size_t length);

int main(void) {


    start_server();
    initialize_qrng();
    while(1) {
        int client_sock = 0;

        run_server(client_sock);
    }
       
    
    close(server_sock);
    unlink(SOCKET_PATH);
    qrng_close();
    return 0;
}

void start_server(void)
{
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    
    // Create socket
    if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    
    
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);
    
    // Bind the socket
    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }
    
    // Listen for incoming connections
    if (listen(server_sock, 5) == -1) {
        perror("listen error");
        close(server_sock);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    printf("Waiting...\n");

}

void run_server(int client_sock)
{
    // Accept connection
    if ((client_sock = accept(server_sock, NULL, NULL)) == -1) {
        perror("accept error");
   
    } else {
        printf("Consumer connected.\n");

        size_t numbers_requested = 0;
        
        if (recv(client_sock, &numbers_requested, sizeof(size_t), 0) > 0) {
            printf("Consumer requested: %d numbers\n", numbers_requested);
        
            int32_t *numbers = malloc(numbers_requested * sizeof(int32_t));
            if (numbers == NULL) {
                perror("Failed to allocate memory!");
            }
            else {
                size_t i = 0;
                generate_numbers(numbers, numbers_requested);

                // Send the numbers to the consumer
                send(client_sock, numbers, numbers_requested*sizeof(int), 0);
                free(numbers);
            }
        }
        close(client_sock);
        printf("Consumer disconnected.\n");
    
    }

}

void initialize_qrng(void)
{
    int retval = qrng_open("random.cs.upt.ro");
    if (retval) {
        exit(EXIT_FAILURE);
    }
}

void generate_numbers(int32_t *numbers, size_t length)
{
    size_t i = 0;
    
    if(qrng_random_int32(0,255,length,numbers) == 0) {
        printf("Generated numbers:\n");
        for (i = 0; i < length; i++) {
            printf("%u ", numbers[i]);
        }
    }
    else {
        printf("Error here");
    }
           

}
