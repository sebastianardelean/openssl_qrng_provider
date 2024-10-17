#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/qrng_sock"

int main() {
    int sock;
    struct sockaddr_un addr;
    int received_numbers[5]; // To store the received numbers

    int num_needed = 5;
    // Create socket
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Connect to the producer
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    // Send request to the producer
    char *request = "Need numbers";
    send(sock, &num_needed, sizeof(int), 0);

    // Receive numbers from the producer
    if (recv(sock, received_numbers, sizeof(received_numbers), 0) > 0) {
        printf("Received numbers: ");
        for (int i = 0; i < num_needed; i++) {
            printf("%d ", received_numbers[i]);
        }
        printf("\n");
    }

    close(sock);
    return 0;
}
