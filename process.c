#include "headers.h"
#include <errno.h>
#include <string.h>

/* Modify this file as needed*/
int main(int agrc, char *argv[])
{
    int to_bus_msgq_id, rec_val;
    key_t key_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    char* data = argv[1];
    int id, runtime, priority, remainingtime;
    sscanf(data,"%d %d %d",&id,&runtime,&priority);

    initClk();
    remainingtime = runtime;
    int currenttime = getClk() - 1;
    //TODO The process needs to get the remaining time from somewhere
    while (true) {
        if(getClk() >= currenttime + 1) {
            usleep(75000);
            msgbuff message;
            rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), id, IPC_NOWAIT);
            if (rec_val == -1) {
                if (errno != ENOMSG) { // If error is not "no message"
                    perror("msgrcv failed");
                    break;
                }
                // No message, skip iteration
                continue;
            }
            remainingtime--;
            currenttime = getClk();
            if(remainingtime == 0) {
                break;
            }
        }
    }

    msgbuff message;
    message.mtype = 99;
    char smthn [70];
    snprintf(smthn, sizeof(smthn), "%d",id);
    strcpy(message.mtext, smthn);
    int send_val = msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), 0);
    return 0;
}
