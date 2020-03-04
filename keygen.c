#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char* argv[]) {

    int input = atoi(argv[1]);
    char randStr[input + 1];
    memset(randStr, '\0', input + 1);

    char charPool[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int i, num;

    srand(time(0));

    for (i = 0; i < input; i++) {
        num = rand() % 27;
        randStr[i] = charPool[num];
    }

    printf("%s\n", randStr);

    return 0;
}