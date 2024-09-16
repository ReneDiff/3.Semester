/* A simple server in the internet domain using TCP
The port number is passed as an argument 
Based on example: https://www.linuxhowtos.org/C_C++/socket.htm 

Modified: Michael Alrøe
Modified: Rene Sand
Extended to support file server!
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "iknlib.h"

#define STRBUFSIZE 256

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

/**
 * @brief Sends a file to a client socket
 * @param clientSocket Socket stream to client
 * @param fileName Name of file to be sent to client
 * @param fileSize Size of file
 */
void sendFile(int clientSocket, const char* fileName, long fileSize)
{
	printf("Sending: %s, size: %li\n", fileName, fileSize);
    
	// ny kode:
	// Åbn filen i binær tilstand
    FILE *fp = fopen(fileName, "rb");
    if (fp == NULL) {
        error("Error opening file");
    }

    char buffer[BUFSIZE] = {0};
    long totalBytesSent = 0;
    int bytesRead;

    // Læs filen i segmenter på 1000 bytes og send dem til klienten
    while ((bytesRead = fread(buffer, sizeof(char), BUFSIZE, fp)) > 0) {
        if (send(clientSocket, buffer, bytesRead, 0) == -1) {
            error("Error sending file");
        }
        totalBytesSent += bytesRead;
        printf("Sent %ld bytes\n", totalBytesSent);
    }

    fclose(fp);
    printf("File transfer complete\n");
	
	
}


int main(int argc, char *argv[])
{
	printf("Starting server...\n");

	// ny kode:

	 int serverSocket, clientSocket;
    socklen_t clientLen;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[STRBUFSIZE] = {0};

    // Opret server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        error("Error opening socket");
    }

    // Indstil serveradressen
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind server socket til port 9000
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        error("Error on binding");
    }

    // Lyt efter forbindelser
    listen(serverSocket, 5);
    clientLen = sizeof(clientAddr);

    while (1) {  // Iterativ server, der kan håndtere flere klientforespørgsler
        printf("Waiting for client...\n");

        // Accepter klientforbindelse
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &clientLen);
        if (clientSocket < 0) {
            error("Error on accept");
        }

        printf("Client connected\n");

        // Læs filnavn fra klienten
        readTextTCP(clientSocket, buffer, STRBUFSIZE);
        printf("Requested file: %s\n", buffer);

        // Tjek om filen findes, og send filstørrelsen
        long fileSize = getFilesize(buffer);
        if (fileSize == 0) {
            writeTextTCP(clientSocket, "File not found");
            printf("File not found: %s\n", buffer);
        } else {
            char fileSizeStr[256];
            sprintf(fileSizeStr, "%ld", fileSize);
            writeTextTCP(clientSocket, fileSizeStr);

            // Send filen til klienten
            sendFile(clientSocket, buffer, fileSize);
        }

        close(clientSocket);  // Luk klientforbindelsen
        printf("Client disconnected, ready for new connection.\n");
    }

    close(serverSocket);  // Luk serversocket (bliver aldrig ramt pga. løkken)


	return 0; 
}
