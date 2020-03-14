#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

char decryptedText[70001]; //Final decrypted text

//Function that does the decrypting
void decrypt (char plainText[70001], char keyText[70001]) {
	int i = 0, textNum = 0, keyNum = 0, decryptNum = 0;
	memset(decryptedText, '\0', 70001);

	for (i = 0; plainText[i]; i++) { //Loop through the encrypted text
		if (plainText[i] != ' ') { //If it's anything but a space
			textNum = ((int)plainText[i] - 65); //Convert from ascii value
		} else {
			textNum = ((int)plainText[i] - 6); //Convert from ascii value
		}
		if (keyText[i] != ' ') { //Do the same thing with the key text
			keyNum = ((int)keyText[i] - 65);
		} else {
			keyNum = ((int)keyText[i] - 6);
		}
		decryptNum = textNum - keyNum; //Subtract the key value from the text value
		if (decryptNum < 0) {
			decryptNum = decryptNum + 27; // If the subtraction turns out to be less than 0, add 27
		}
		if (decryptNum != 26) {
			decryptNum = decryptNum + 65; //Convert back to ascii value
		} else {
			decryptNum = decryptNum + 6; //Convert back to ascii value
		}
		decryptedText[i] = (char)decryptNum; //create decrypted text
	}
}

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]) { //Start of main function
	//Initialize and create variables
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[70001];
	struct sockaddr_in serverAddress, clientAddress;
	pid_t childPid = -5;
	int childExitStatus = -5;
	char plainText[70001];
	char keyText[70001];
	memset(plainText, '\0', 70001);
	memset(keyText, '\0', 70001);
	int i = 0;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	while (1) {
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//Fork new child process
		childPid = fork();
		switch (childPid) {
			case -1: //Error forking
				error("ERROR forking");
				break;
		
			case 0: //Child process code
			i = 0;
				while (1) {
					// Get the message from the client and display it
					memset(buffer, '\0', 70001);
					charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0); // Read the client's message from the socket
					if (charsRead < 0) error("ERROR reading from socket");
					if (charsRead == 0) break;
					i++;
					if (i == 1) { //If it's the first loop (corresponds with text)
						memset(plainText, '\0', 70001);
						// printf("SERVER: Length of string recieved: %d\n", strlen(buffer));
						strcpy(plainText, buffer);
						// printf("SERVER: Length of string copied: %d\n", strlen(plainText));
						//Send a Success message back to the client
						charsRead = send(establishedConnectionFD, "I'm otp_dec_d.c", 15, 0); // Send ID back
						if (charsRead < 0) error("ERROR writing to socket");
					} else if (i == 2) { //If it's the second loop (corresponds with key)
						if (!strcmp(buffer, "No connection for you, buddy")) { // Stop connection if it's the wrong client
							break;
						}
						memset(keyText, '\0', 70001);
						strcpy(keyText, buffer);
						decrypt(plainText, keyText); //Decrypt the text
						charsRead = send(establishedConnectionFD, decryptedText, strlen(decryptedText), 0); //Send final result
						if (charsRead < 0) error("ERROR writing to socket");
						break;
					}
				}
				break;
			
			default:
				waitpid(childPid, &childExitStatus, 0); //Wait for child to finish
				close(establishedConnectionFD); // Close the existing socket which is connected to the client
				childPid = -5;
				childExitStatus = -5;
				break;
		}
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}