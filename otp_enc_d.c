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

char encryptedText[70001]; //Final encrypted text to be returned

//Function that does the encryption:
void encrypt (char plainText[70001], char keyText[70001]) {
	int i = 0, textNum = 0, keyNum = 0, encryptNum = 0;
	memset(encryptedText, '\0', 70001); //Clear out enrypted text string

	for (i = 0; plainText[i]; i++) { //Loop through the plain text
		if (plainText[i] != ' ') { //If the char is anything but a space
			textNum = ((int)plainText[i] - 65); //Convert to ascii value
		} else {
			textNum = ((int)plainText[i] - 6); //Convert to ascii value
		}
		if (keyText[i] != ' ') { //Do the same thing for the key
			keyNum = ((int)keyText[i] - 65);
		} else {
			keyNum = ((int)keyText[i] - 6);
		}
		encryptNum = textNum + keyNum; //Add the key value to the plain text value
		if (encryptNum > 26) {
			encryptNum = encryptNum - 27; //If the addition ends up being more than 26, subtract 27
		}
		if (encryptNum != 26) {
			encryptNum = encryptNum + 65; //Convert back from ascii values
		} else {
			encryptNum = encryptNum + 6; //Convert back from ascii values
		}
		encryptedText[i] = (char)encryptNum; //Create encrypted text
	}
}

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[]) { //Start of main function
	//Create and initialize variables
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
			case -1: //If there's an error forking
				error("ERROR forking");
				break;
		
			case 0: //Child process code
				i = 0;
				while (1) {
					// Get the message from the client and display it
					memset(buffer, '\0', 70001); //Clear out buffer
					charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0); // Read the client's message from the socket
					if (charsRead < 0) error("ERROR reading from socket");
					if (charsRead == 0) break;
					i++; //Increment how many times we've looped
					if (i == 1) { //The first loop, corresponds with the plain text
						memset(plainText, '\0', 70001);
						strcpy(plainText, buffer);
						//Send a Success message back to the client
						charsRead = send(establishedConnectionFD, "I'm otp_enc_d.c", 15, 0); // Send ID back
						if (charsRead < 0) error("ERROR writing to socket");
					} else if (i == 2) { //The second loop, corresponds with the key
						if (!strcmp(buffer, "No connection for you, buddy")) {
							break; //Stop connection if connecting with wrong client
						}
						memset(keyText, '\0', 70001);
						strcpy(keyText, buffer);
						encrypt(plainText, keyText); //Encrypt
						charsRead = send(establishedConnectionFD, encryptedText, strlen(encryptedText), 0); //Send it back
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