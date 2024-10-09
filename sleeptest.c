#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argument_count, char *arguments[]) {
    int sleep_duration = 60; // Default sleep duration is 60 seconds

    // An argument was given by user as the duration of sleep
    if (argument_count > 1) {
        sleep_duration = atoi(arguments[1]); // Convert the argument to an integer
        if (sleep_duration <= 0) {
            fprintf(stderr, "Invalid duration. Please provide a positive integer.\n");
            return 1;
        }
    }

    printf("Command entered: Sleeping for %d seconds\n", sleep_duration);
    sleep(sleep_duration); // Sleep for the specified duration
    printf("Finished sleeping for %d seconds.\n", sleep_duration);

    return 0;
}
