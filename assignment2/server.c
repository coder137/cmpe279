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
static int arg_get_server_socket_fd(void);

static struct sockaddr_in get_default_socket_address(void);
static void parent_run();
static void child_run();

static void invoke_fork(void);
static void invoke_exec(int server_fd);

// State variables
// * NOTE, Do not read/write these state variables, use getter functions above
static program_state_e arg_pstate = PARENT;
static int arg_socket_fd = -1;

int main(int argc, char const *argv[]) {
    arg_process_cli(argc, argv);

    printf("execve=0x%p\n", execve);

    switch (arg_get_program_state()) {
    case PARENT:
        parent_run();
        break;
    case CHILD:
        child_run();
        break;
    default:
        perror("Invalid program state");
        exit(EXIT_FAILURE);
    }

    return 0;
}

// Main operations
static struct sockaddr_in get_default_socket_address(void) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    return address;
}

static void parent_run() {
    int server_fd;
    int opt = 1;

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

    struct sockaddr_in address = get_default_socket_address();

    // Forcefully attaching socket to the port 80
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    invoke_fork();
    invoke_exec(server_fd);
}

static void child_run() {
    struct sockaddr_in address = get_default_socket_address();
    int server_fd = arg_get_server_socket_fd();

    int new_socket;
    int addrlen = sizeof(address);
    char buffer[102] = {0};
    char *hello = "Hello from server";

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
}

// Invoke operations
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

    // Tested with both users (`nnaidu` and `nobody`)
    struct passwd *nobody_passwd = getpwnam("nobody");
    if (0 != setuid(nobody_passwd->pw_uid)) {
        perror("Invalid user id");
        exit(EXIT_FAILURE);
    }
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

            // TODO, Incorrect if not number
            // -s -c edge case
            arg_socket_fd = atoi(argv[next]);
            i = next;
            continue;
        }
    }
}

static program_state_e arg_get_program_state(void) { return arg_pstate; }
static int arg_get_server_socket_fd(void) { return arg_socket_fd; }
