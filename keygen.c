#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char* argv[]) {

    int input = atoi(argv[1]);
    char randStr[input];
    char charPool[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int i;

    memset(randStr, '\0', input);

    srand(time(0));

    for (i = 0; i < input; i++) {
        randStr[i] = charPool[rand() % 27];
    }

    printf("%s\n", randStr);

    return 0;
}