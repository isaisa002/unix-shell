#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	
    int sleep_time = 40;  // Default sleep time 40 seconds

    // If an argument is passed, use it as the sleep time
    if (argc == 2) {
        sleep_time = atoi(argv[1]);
    }

    printf("Sleeping for %d seconds...\n", sleep_time);
    sleep(sleep_time);
    printf("Woke up after %d seconds\n", sleep_time);

    return 0;
}