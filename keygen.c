#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//Generates a random key of specified length. Only A - Z and space characters.
int main (int argc, char* argv[]) {

    int input = atoi(argv[1]); //Convert input into number
    char randStr[input + 1];
    memset(randStr, '\0', input + 1); //Initialize string

    char charPool[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; //Pool of valid characters
    int i;

    srand(time(0)); //Seed random generator

    for (i = 0; i < input; i++) { //Loop through the given amount of times
        randStr[i] = charPool[rand() % 27]; //Randomly generate a char
    }

    printf("%s\n", randStr); //Print the final result

    return 0;
}