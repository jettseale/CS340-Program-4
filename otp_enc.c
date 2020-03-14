#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <limits.h>
#include <stdbool.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[]) //Start of main function
{
	// Check to make sure the correct number of arguments was entered
	if (argc != 4) {
		fprintf(stderr, "You didn't enter the correct number of arguments. The syntax is: otp_enc {plaintext filename} {key filename} {port number}\n");
		exit(0);
	}

	// Check to make sure a valid integer was entered for the port number
	if (atoi(argv[3]) == 0) {
		fprintf(stderr, "You didn't enter a valid integer for the port number.\n");
		exit(0);
	}

	//Set up variables:
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[70001];
	memset(buffer, '\0', 70001);
	
	char plainTextContents[70001];
	char keyContents[70001];
	memset(plainTextContents, '\0', 70001);
	memset(keyContents, '\0', 70001);
	char c;
	int i = 1;
	int isNL = 0;
	FILE* plainTextFile;
	FILE* keyFile;

	// Grab the plain text contents
	plainTextFile = fopen(argv[1], "r");
	if (plainTextFile == NULL) {
		fprintf(stderr, "Error opening file\n");
		exit(0);
	}
	c = fgetc(plainTextFile);
	plainTextContents[0] = c;
	while (c != EOF) {
		c = fgetc(plainTextFile);
		plainTextContents[i] = c;
		i++;
	}
	fclose(plainTextFile);

	// Remove the newline from the end of the plain text
	if (plainTextContents[strlen(plainTextContents) - 2] == '\n') {
		plainTextContents[strlen(plainTextContents) - 2] = '\0';
	}

	// Grab the key contents
	c = '\0';
	i = 1;
	keyFile = fopen(argv[2], "r");
	if (keyFile == NULL) {
		fprintf(stderr, "Error opening file\n");
		exit(0);
	}
	c = fgetc(keyFile);
	keyContents[0] = c;
	while (c != EOF) {
		c = fgetc(keyFile);
		keyContents[i] = c;
		i++;
	}
	fclose(keyFile);

	// Remove the newline from the end of the key
	if (keyContents[strlen(keyContents) - 2] == '\n') {
		keyContents[strlen(keyContents) - 2] = '\0';
	}

	// Make sure the key is long enough for the plain text
	if (strlen(keyContents) < (strlen(plainTextContents) - 1)) {
		fprintf(stderr, "ERROR: The key you entered is too short.\n");
		exit(1);
	}

	//Make sure there are no bad characters for the plain text
	for (i = 0; i < strlen(plainTextContents); i++) {
		if ((int)plainTextContents[i] > 90 || (int)plainTextContents[i] < 65 && (int)plainTextContents[i] != 32) {
			fprintf(stderr, "ERROR: There are bad characters in the provided plain text file.\n");
			exit(1);
		}
	}

	//Make sure there are no bad characters for the key
	for (i = 0; i < strlen(keyContents); i++) {
		if ((int)keyContents[i] > 90 || (int)keyContents[i] < 65 && (int)keyContents[i] != 32) {
			fprintf(stderr, "ERROR: There are bad characters in the provided key file.\n");
			exit(1);
		}
	}
    
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Send message to server
	charsWritten = send(socketFD, plainTextContents, strlen(plainTextContents), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");

	// // Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	// Send message to server
	charsWritten = send(socketFD, keyContents, strlen(keyContents), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	
	printf("%s\n", buffer); //Print the final result

	close(socketFD); // Close the socket
	
	return 0;
}