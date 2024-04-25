#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <sys/ipc.h>
// #include <sys/shm.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 12345
#define DUMMY_DATA_FILE_PATH "paxxy_data.csv"
// #define PATH "/tmp"
// #define KEY 2333
int server_socket;
FILE* file;


typedef struct{
    int id;
    float ra;
    float ll;
    float la;
    float v1;
    int as1;
    int as2;
    int as3;
    int as4;

}DataObject;

void handle_termination(int signum){
    printf("Server is shutting down");
    close(server_socket);
    exit(EXIT_SUCCESS);
}

// void runSubprocess(){
//     pid_t pid = fork();

//     if(pid = -1){
//         perror("Fork failed");
//         exit(EXIT_FAILURE);
//     }

//     if(pid = 0){
//         //Child process
//         execl("./a", "./a", NULL);
//         perror("Exec failed");
//         exit(EXIT_FAILURE);
//     }

//     else{
//         //Parent process
//         waitpid(pid, NULL, 0);
//     }
// }

int main() {
    signal(SIGINT, handle_termination);

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept a connection
    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    int status;
    recv(client_socket, &status, sizeof(int), 0);

    if(status == 1)
    {
        // Open and read the Dummy data CSV file at 500 Hz
        FILE* file = fopen(DUMMY_DATA_FILE_PATH, "r");
        if (!file) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), file) != NULL) {
            DataObject data;
            sscanf(line, "%d,%f,%f,%f,%f,%d,%d,%d,%d", &data.id,&data.ra,&data.ll,&data.la,&data.v1,&data.as1,&data.as2,&data.as3,&data.as4); 
            // Send data to the Python client
            send(client_socket, &data,sizeof(DataObject), 0);

            // Introduce a delay to achieve 500 Hz
            if(data.id%3000==0){
                sleep(6);
            }
            // 1 second divided by 500u
        }
    }
    else if(status == 2)
    {
        // Open and read the Actual data CSV file at 500 Hz
        // runSubprocess();

        // key_t key = ftok(PATH, KEY);
        // if(key == -1){
        //     perror("ftok failed");
        //     exit(EXIT_FAILURE);
        // }

        // int shmid = shmget(key, 8, 0);
        // if(shmid == -1){
        //     perror("shm failed");
        //     exit(EXIT_FAILURE);
        // }

        // char *buffer = (char *)shmat(shmid, NULL, 0);
        // char start = buffer[1];

        // while(start == "1"){
        //     char *buffer = (char *)shmat(shmid, NULL, 0);
        //     if((intptr_t)buffer == -1){
        //         perror("shmat failed");
        //         exit(EXIT_FAILURE);
        //     }
        //     char start = buffer[1];
        // }

        // char csv_number = buffer[2];

        // char path = strcat("ADS_data", csv_number);
        // path = strcat(path, ".csv");

        FILE* file = fopen(path, "r");
        if (!file) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), file) != NULL) {
            DataObject data;
            sscanf(line, "%d,%f,%f,%f,%f,%d,%d,%d,%d", &data.id, &data.ra,&data.ll,&data.la,&data.v1,&data.as1,&data.as2,&data.as3,&data.as4); 
            // Send data to the Python client
            send(client_socket, &data,sizeof(DataObject), 0);

            // Introduce a delay to achieve 500 Hz
            if(data.id%3000==0){
                sleep(6);
            }
            // 1 second divided by 500u
        }
    }

    // Close the file and sockets
    fclose(file);
    close(client_socket);
    close(server_socket);

    return 0;
}
