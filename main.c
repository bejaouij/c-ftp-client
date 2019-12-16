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

#define GREEN_TEXT_COLOR_CODE "\x1b[32m"
#define RED_TEXT_COLOR_CODE "\x1b[31m"
#define CYAN_TEXT_COLOR_CODE "\x1b[36m"
#define BRIGHT_BLUE_TEXT_COLOR_CODE "\x1b[94m"
#define RESET_TEXT_COLOR_CODE "\x1b[0m"

int main() {
	/* Initialize the client side socket and connect it to the gateway */
    int socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);

    if(socket < 0) {
            perror("Socket Creation Error");
    }

    struct sockaddr_in adServ;
    adServ.sin_family = AF_INET;
    adServ.sin_port = htons(PORT);
        
	if(inet_pton(AF_INET, SERVER_ADDRESS, &(adServ.sin_addr)) == -1) {
		perror("Address Conversion Error");
	}

    socklen_t IgA = sizeof(struct sockaddr_in);

    if(connect(socketDescriptor, (struct sockaddr*) &adServ, IgA) == -1) {
        perror("Connection Error");
    }

    printf("Communication established\n");
	/********************/

    /* Close the server side socket */
    if(close(socketDescriptor) == -1) {
        perror("Socket Closing Error");
    }

    printf(RED_TEXT_COLOR_CODE "CONNECTION CLOSED\n" RESET_TEXT_COLOR_CODE);
    /********************/
}