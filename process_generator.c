#include "headers.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
void clearResources(int);

// Extended process structure to include memsize
typedef struct {
    int id;
    int arrival_time;
    int run_time;
    int priority;
    int memsize;
} process;

int main(int argc, char *argv[]) {
    /* Read args to determine quantum and scheduling algorithm */

    key_t key_id;
    int to_sched_msgq_id, send_val;

    key_id = ftok("procfile", 65);
    to_sched_msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (to_sched_msgq_id == -1) {
        perror("Error creating message queue");
        exit(-1);
    }

    // Read the input file
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    process *ptr = (process *)malloc(10 * sizeof(process));
    if (ptr == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    int iterator = 0;
    char line[256];
    char temp[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comment lines
        if (line[0] == '#') {
            continue;
        }

        // Parse process details, including memsize
        sscanf(line, "%d\t%d\t%d\t%d\t%d", &ptr[iterator].id, &ptr[iterator].arrival_time, &ptr[iterator].run_time, &ptr[iterator].priority, &ptr[iterator].memsize);

        iterator++;
        if (iterator % 10 == 0) {
            ptr = (process *)realloc(ptr, (iterator + 10) * sizeof(process));
            if (ptr == NULL) {
                perror("Memory reallocation failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);

    // Start the clock process
    int pid = fork();
    if (pid == 0) {
        char *args[] = {"./clk.out", NULL};
        execv("./clk.out", args);
        perror("Clock process execution failed");
        exit(EXIT_FAILURE);
    }

    // Start the scheduler process
    pid = fork();
    if (pid == 0) {
        char *args[5];
        args[0] = "./scheduler.out";
        args[1] = argv[3]; // Scheduling algorithm
        args[2] = (atoi(argv[3]) == 3 || atoi(argv[3]) == 4) ? argv[5] : "0"; // Quantum if applicable
        
        char process_count[16];
        snprintf(process_count, sizeof(process_count), "%d", iterator);
        args[3] = process_count;
        args[4] = NULL;

        execv("./scheduler.out", args);
        perror("Scheduler process execution failed");
        exit(EXIT_FAILURE);
    }

    // Send processes to scheduler
    initClk();
    int next_process = 0;
    while (next_process < iterator) {
        int current_time = getClk();
        if (current_time < ptr[next_process].arrival_time) {
            usleep((ptr[next_process].arrival_time - current_time) * 1000);
            continue;
        }

        char message_text[64];
        //int result = snprintf(message_text, sizeof(message_text), "%d %d %d %d %d", ptr[next_process].id, ptr[next_process].run_time, ptr[next_process].priority, ptr[next_process].arrival_time, ptr[next_process].memsize);//Ahmad
        int result = snprintf(message_text, sizeof(message_text), "%d %d %d %d %d", ptr[next_process].id, ptr[next_process].run_time, ptr[next_process].priority,  ptr[next_process].memsize,ptr[next_process].arrival_time);

        struct msgbuff {
            long mtype;
            char mtext[64];
        } message;

        message.mtype = 7;
        strcpy(message.mtext, message_text);

        send_val = msgsnd(to_sched_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT);
        if (send_val == -1) {
            perror("Failed to send message to scheduler");
        }

        next_process++;
    }

    // Cleanup
    free(ptr);
    waitpid(-1, NULL, 0);
    clearResources(0);

    return 0;
}

void clearResources(int signum) {
    key_t key_id = ftok("procfile", 65);
    int to_sched_msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (to_sched_msgq_id != -1) {
        msgctl(to_sched_msgq_id, IPC_RMID, NULL);
    }

    destroyClk(true);
    exit(0);
}
