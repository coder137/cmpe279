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

typedef enum {
    PARENT,
    CHILD,
} program_state_e;

// Static function declarations
static void arg_process_cli(int argc, char const *argv[]);
static program_state_e arg_get_program_state(void);
static int arg_get_server_socket_fb(void);

static void invoke_fork(void);
static void invoke_exec(int server_fd);

// State variables
// * NOTE, Do not read/write these state variables, use getter functions above
// TODO, Seperate this into modules later
static program_state_e arg_pstate = PARENT;
static int arg_socket_fd = -1;

int main(int argc, char const *argv[]) {

    // ? debugging
    printf("Arguments: %d\r\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("%s\r\n", argv[i]);
    }
    printf("-------\r\n");

    arg_process_cli(argc, argv);

    printf("%d\r\n", arg_get_program_state());
    printf("%d\r\n", arg_get_server_socket_fb());

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

    printf("Server FD: %d\r\n", server_fd);
    invoke_fork();
    invoke_exec(server_fd);

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

static void invoke_fork(void) {
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

static void invoke_exec(int server_fd) {
    char server_fd_str[12] = {0};
    sprintf(server_fd_str, "%d", server_fd);

    char *argv[] = {"./server", "-c", "-s", server_fd_str, NULL};
    int err = execv("./server", argv);
    if (err < 0) {
        perror("Failed to exec ./server");
        exit(EXIT_FAILURE);
    }
}

// Argument Processing
static void arg_process_cli(int argc, char const *argv[]) {
    // check for -s and -c arguments
    for (int i = 0; i < argc; i++) {
        const char *current = argv[i];
        if (strcmp(current, "-c") == 0) {
            arg_pstate = CHILD;
            continue;
        } else if (strcmp(current, "-s") == 0) {
            int next = i + 1;
            if (next >= argc) {
                perror("Add the server socket fd after -s");
                exit(EXIT_FAILURE);
            }

            // TODO, Unsafe if not number
            // -s -c edge case
            arg_socket_fd = atoi(argv[next]);
            i = next;
            continue;
        }
    }
}

static program_state_e arg_get_program_state(void) { return arg_pstate; }
static int arg_get_server_socket_fb(void) { return arg_socket_fd; }
