// Server side C/C++ program to demonstrate Socket programming

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pwd.h>
#include <sys/wait.h>

// Constants
#define PORT 80

// Static function declarations
static void invoke_child(void);

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[102] = {0};
    char *hello = "Hello from server";

    printf("execve=0x%p\n", execve);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to port 80
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 80
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    invoke_child();

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for data from client\r\n");
    int valread = read(new_socket, buffer, 1024);
    (void)valread;
    printf("%s\n", buffer);

    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    return 0;
}

static void invoke_child(void) {
    pid_t id = fork();

    if (id > 0) {
        // Safely exit the parent
        while (wait(NULL) > 0) {
        }
        exit(0);
    } else if (id < 0) {
        // Error
        printf("Fork returned with pid_t %d\r\n", id);
        exit(EXIT_FAILURE);
    }

    // Child id == 0

    // uid_t uid = 0;
    // uid = getuid();
    // printf("Before: %d\r\n", uid);

    // Tested with both users (`nnaidu` and `nobody`)
    struct passwd *nobody_passwd = getpwnam("nobody");
    // const uid_t USER = 1000;
    // const uid_t USER = 65534;
    if (0 != setuid(nobody_passwd->pw_uid)) {
        perror("Invalid user id");
        exit(EXIT_FAILURE);
    }

    // uid = getuid();
    // printf("After: %d\r\n", uid);
}
