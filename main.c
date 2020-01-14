#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 32456
#define SERVER_ADDRESS "127.0.0.1"

#define COMMAND_BUFFER_MAX_LENGTH 256
#define MESSAGE_BUFFER_MAX_LENGTH 2048
#define FILE_BUFFER_MAX_LENGTH 32768

#define FILE_LISTING_COMMAND "ls -A"

#define GREEN_TEXT_COLOR_CODE "\x1b[32m"
#define RED_TEXT_COLOR_CODE "\x1b[31m"
#define CYAN_TEXT_COLOR_CODE "\x1b[36m"
#define BRIGHT_BLUE_TEXT_COLOR_CODE "\x1b[94m"
#define RESET_TEXT_COLOR_CODE "\x1b[0m"

#define LINE_SEPARATOR "--------------------"

/**
 * Close the communication
 *
 * @param int* isGatewayEstablished Tells whether the connection is currently established.
 */
void _closeGateway(int* isGatewayEstablished);

/**
 * Retrieve stored server files.
 *
 * @param int serverSocketDescriptor Descriptor of the gateway server.
 * @param char* buffer Buffer where store the server response.
 */
void _getRemoteFiles(int serverSocketDescriptor, char* buffer);

/**
 * Display the commands list.
 */
void cli_help();

/**
 * Process the communication closing.
 *
 * @param int isGatewayEstablished Tell whether the connection is currently established.
 */
void cli_quit(int* isGatewayEstablished);

/**
 * Display local files.
 */
void cli_ls();

/**
 * Display remote files.
 *
 * @param int serverSocketDescriptor Descriptor of the gateway server.
 */
void cli_rls(int serverSocketDescriptor);

void cli_put(int serverSocketDescriptor, char filename[COMMAND_BUFFER_MAX_LENGTH]);

/**
 * Execute invalid command behavior.
 */
void cli_invalidCommand();

int main() {
	int isGatewayEstablished = 1;

	/* Initialize the client side socket and connect it to the gateway */
    int socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);

    if(socket < 0) {
        perror("Socket Creation Error");
        isGatewayEstablished = 0;
    }

    struct sockaddr_in adServ;
    adServ.sin_family = AF_INET;
    adServ.sin_port = htons(PORT);
        
	if(inet_pton(AF_INET, SERVER_ADDRESS, &(adServ.sin_addr)) == -1) {
		perror("Address Conversion Error");
		isGatewayEstablished = 0;
	}

    socklen_t IgA = sizeof(struct sockaddr_in);

    if(connect(socketDescriptor, (struct sockaddr*) &adServ, IgA) == -1) {
        perror("Connection Error");
        isGatewayEstablished = 0;
    } else {
        printf(GREEN_TEXT_COLOR_CODE "Communication established\n" RESET_TEXT_COLOR_CODE);
    }
	/********************/

	char commandBuffer[COMMAND_BUFFER_MAX_LENGTH];

	while(isGatewayEstablished == 1) {
	    fgets(commandBuffer, COMMAND_BUFFER_MAX_LENGTH, stdin);
        *strchr(commandBuffer, '\n') = '\0'; /* Remove the end of line character */

        if(strcmp(commandBuffer, "help") == 0) {
            cli_help();
        } else if(strcmp(commandBuffer, "quit") == 0) {
            cli_quit(&isGatewayEstablished);
        } else if(strcmp(commandBuffer, "ls") == 0) {
            cli_ls();
        } else if(strcmp(commandBuffer, "rls") == 0) {
            cli_rls(socketDescriptor);
        } else if(strncmp(commandBuffer, "put ", 4) == 0) {
            char filename[COMMAND_BUFFER_MAX_LENGTH];
            int i = 4;

            while(commandBuffer[i] != ' ' && commandBuffer[i] != '\n') {
                filename[i - 4] = commandBuffer[i];
                i++;
            }

            filename[i] = '\0';

            cli_put(socketDescriptor, filename);
        } else {
            cli_invalidCommand();
        }
	}

    /* Close the server side socket */
    if(close(socketDescriptor) == -1) {
        perror("Socket Closing Error");
    }

    printf(RED_TEXT_COLOR_CODE "CONNECTION CLOSED\n" RESET_TEXT_COLOR_CODE);
    /********************/
}

void _closeGateway(int* isGatewayEstablished) {
    *isGatewayEstablished = 0;
}

void _getRemoteFiles(int serverSocketDescriptor, char* messageBuffer) {
    int resSend, resRecv;

    char remoteCommand[4];
    strcpy(remoteCommand, "rls");

    if((resSend = send(serverSocketDescriptor, &remoteCommand, (4*sizeof(char)), 0)) == -1) {
        perror("Command Sending Error");
        return;
    }
    if(resSend == 0) {
        perror("Server Down");
        return;
    }
    if((resRecv = recv(serverSocketDescriptor, messageBuffer, (MESSAGE_BUFFER_MAX_LENGTH*sizeof(int)), 0)) == -1) {
        perror("Command Reception Error");
        return;
    }
    if(resRecv == 0) {
        perror("Server Down");
        return;
    }
}

void cli_help() {
    printf("%s\n", LINE_SEPARATOR);
    printf("help         - Display the commands list.\n");
    printf("quit         - Stop the communication.\n");
    printf("ls           - Display local files.\n");
    printf("rls          - Display remote files.\n");
    printf("get filename - Download the specified file and store it in the download folder.\n");
    printf("put filename - Upload the specified file.\n");
    printf("%s\n", LINE_SEPARATOR);
}

void cli_quit(int* isGatewayEstablished) {
    _closeGateway(isGatewayEstablished);
}

void cli_ls() {
    printf("%s\n", LINE_SEPARATOR);
    system(FILE_LISTING_COMMAND);
    printf("%s\n", LINE_SEPARATOR);
}

void cli_rls(int serverSocketDescriptor) {
    char messageBuffer[MESSAGE_BUFFER_MAX_LENGTH];
    _getRemoteFiles(serverSocketDescriptor, messageBuffer);

    printf("%s\n", LINE_SEPARATOR);
    printf("%s\n", messageBuffer);
    printf("%s\n", LINE_SEPARATOR);
}

void cli_put(int serverSocketDescriptor, char filename[COMMAND_BUFFER_MAX_LENGTH]) {
    FILE* pfile = fopen(filename, "r");

    if(pfile == NULL) {
        perror("File Opening Error");
    } else {
        int resSend;

        char remoteCommand[COMMAND_BUFFER_MAX_LENGTH];
        strcpy(remoteCommand, "put ");
        strcat(remoteCommand, filename);

        if((resSend = send(serverSocketDescriptor, &remoteCommand, (4*sizeof(char)), 0)) == -1) {
            perror("Command Sending Error");
            return;
        }
        if(resSend == 0) {
            perror("Server Down");
            return;
        }

        /*char bufferPart[FILE_BUFFER_MAX_LENGTH];

        char command[COMMAND_BUFFER_MAX_LENGTH];
        strcpy(command, "put ");
        strcat(command, filename);

        printf("%s", command);

        int resSend;

        if((resSend = send(serverSocketDescriptor, &command, COMMAND_BUFFER_MAX_LENGTH, 0)) == -1) {
            perror("Command Sending Error");
            return;
        }

        fclose(pfile);*/
    }
}

void cli_invalidCommand() {
    printf(RED_TEXT_COLOR_CODE "Invalid command. Type 'help' to display available commands.\n" RESET_TEXT_COLOR_CODE);
}