#include "SJF_utils.h"
#include "PHPF_utils.h"
#include "RR_utils.h"
#include "PCB_utils.h"
#include "MLFP_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MEMORY_SIZE 1024 // Total memory size in bytes
int firstentry = 0;
typedef struct BuddyBlock {
    int start;
    int size;
    int is_free;
    int pid;
    struct BuddyBlock *next;
    int requested_size;    // Size actually requested (e.g., 200 bytes)
} BuddyBlock;

BuddyBlock *memory_list = NULL; // Linked list of memory blocks

// Function to initialize the buddy system
void initialize_memory() {
    memory_list = (BuddyBlock *)malloc(sizeof(BuddyBlock));
    memory_list->start = 0;
    memory_list->size = MEMORY_SIZE;
    memory_list->is_free = 1;
    memory_list->next = NULL;
    memory_list->pid =0;
}

// Function to split a block into two buddies
void split_block(BuddyBlock *block) {
    int half_size = block->size / 2;
    BuddyBlock *buddy = (BuddyBlock *)malloc(sizeof(BuddyBlock));

    buddy->start = block->start + half_size;
    buddy->size = half_size;
    buddy->is_free = 1;
    buddy->next = block->next;
    buddy->pid =0;

    block->size = half_size;
    block->next = buddy;
    //block->start = buddy->start;
}
void print_RR_queue(RR_Queue *queue) {
    Node *current = queue->head;
    printf("Running Queue: ");
    while (current) {
        printf("PID=%d -> ", current->pid);
        current = current->next;
    }
    printf("NULL\n");
}


// Function to allocate memory
BuddyBlock *allocate_memory(int size,int pid) {
    BuddyBlock *current = memory_list;
    //int PrevBlockSize = 0;
   // printf("test current %p\n",&current);//Ahmad
    // Find the first suitable free block
    while (current) {
        if (current->is_free && current->size >= size) {
            while ((current->size > size) ) {
               if (current->size / 2 < size) {
                    break;
                }
                split_block(current);
            }
            //memory_list = (BuddyBlock *)malloc(sizeof(current));
            current->is_free = 0;
            current->pid = pid;
            current->requested_size = size;
            //current->size = size;
            return current;
        }
        current = current->next;
    }
    return NULL; // No suitable block found
}

// Function to merge two buddy blocks
void merge_buddies() {
    BuddyBlock *current = memory_list;
    while (current && current->next) {
        if (current->is_free && current->next->is_free && current->size == current->next->size && current->start + current->size == current->next->start) {
            current->size *= 2;
            BuddyBlock *temp = current->next;
            current->next = current->next->next;
            free(temp);
        } else {
            current = current->next;
        }
    }
}

// Function to free memory
void free_memory(int pid,int *Bfree_start,int *Bfree_size) {
    BuddyBlock *current = memory_list;
    
    while (current) {
        //printf("yyyyyyYY  , fREE_START \n");
        if (current->pid == pid) {
            current->is_free = 1;
            current->pid = 0;
            *Bfree_start = current->start;
            *Bfree_size = current->requested_size;;
            int end_address = current->start + current->size - 1;
            log_memory_operation("freed",getClk(),current->requested_size,*Bfree_start,end_address,pid);
            merge_buddies();
            return;
        }
        current = current->next;
    }
      *Bfree_start = 0;
    *Bfree_size = 0;
    printf("Error: No block found for pid=%d\n", pid);
}

// Function to log memory operations
void log_memory_operation(const char *operation, int time, int size, int start, int end, int pid) {
    FILE *file = fopen("memory.log", "a");
    if(firstentry == 0)
    {
        fprintf(file,"#At time x allocated y bytes for process z from i to j\n");
        firstentry =1;
    }
    if (file) {
        if (strcmp(operation, "allocated") == 0) {
            fprintf(file, "At time %d allocated %d bytes for process %d from %d to %d\n", time, size, pid, start, end);
            fflush(file);
        } else if (strcmp(operation, "freed") == 0) {
            fprintf(file, "At time %d freed %d bytes from process %d from %d to %d\n", time, size, pid, start, end);
            fflush(file);
        }
        fclose(file);
    }
}





// Process structure
struct Process {
    int id;
    int arrival_time;
    int runtime;
    int memsize;
};

// Function to handle process arrival and memory allocation
void handle_process(struct Process *process) {
    BuddyBlock *allocated_block = allocate_memory(process->memsize,process->id);
    if (allocated_block) {
        // Calculate end address
        int end_address = allocated_block->start + allocated_block->size - 1;

        // Log memory allocation
        log_memory_operation("allocated", process->arrival_time, process->memsize, allocated_block->start, end_address, process->id);

        // Simulate process runtime
        printf("Process %d is running\n", process->id);

        // Simulate completion and free memory
      //  free_memory(allocated_block->start, allocated_block->size);

        // Log memory deallocation
        log_memory_operation("freed", process->arrival_time + process->runtime, process->memsize, allocated_block->start, end_address, process->id);
    } else {
        printf("Process %d: Memory allocation failed\n", process->id);
    }
}


int algorithm, quantum, process_count;
int first_clk = 0, last_clk;

void final_performance() {
    const char *filePath2 = "scheduler.perf";
    FILE *file2;

    file2 = fopen(filePath2, "w");
    float performance[3];
    get_performance(performance);
    float cpu_utilization = ((performance[0]) / ((float)(last_clk - first_clk))) * 100;
    float avg_wta = performance[1] / (float)process_count;
    float avg_wait = performance[2] / (float)process_count;
    fprintf(file2, "CPU utilization = %.2f\nAvg WTA = %.2f\nAvg Waiting = %.2f\n", cpu_utilization, avg_wta, avg_wait);

    fflush(file2);
    fclose(file2);

    log_files_close();
}





void run_MLFP(int to_sched_msgq_id) {
    PCBEntry pcbtable[process_count+1];
    int activeProcess = 0;

    MLFP_Queue running_queue[11];
    for (int i = 0; i < 11; i++) {
        running_queue[i].head = NULL;
        running_queue[i].count = 0;
    }

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    if (to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    initialize_memory(); // Initialize the buddy memory system
    int time_progress = getClk();
    int finishedProcs = 0, quantum_counter = 0, currentPriority = 0, iteration = 0;

    while (finishedProcs < process_count) {
        if (getClk() == time_progress + 1) {
            printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());

            usleep(50000);
            time_progress++;
            msgbuff message;

            // Process arrival handling
            while (msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
                int id, runtime, priority, memsize;
                sscanf(message.mtext, "%d %d %d %d", &id, &runtime, &priority, &memsize);

                if (first_clk == 0) {
                    first_clk = getClk();
                }

                struct Process process = {id, getClk(), runtime, memsize};

                // Attempt to allocate memory using the buddy system
                BuddyBlock *allocated_block = allocate_memory(process.memsize,process.id);
                if (allocated_block) {
                    log_memory_operation("allocated", process.arrival_time, process.memsize, allocated_block->start, allocated_block->start + allocated_block->size - 1, process.id);
                    printf("Process %d allocated %d bytes at address %d\n", process.id, process.memsize, allocated_block->start);

                    addPCBentry(pcbtable, id, getClk(), runtime, priority);

                    PCBEntry *pcb = findPCBentry(pcbtable, id, process_count);
                    if (pcb) {
                        pcb->start_address = allocated_block->start;
                        pcb->allocated_size = allocated_block->size;
                    }

                    int pid = fork();
                    if (pid == 0) {
                        printf("Process %d with priority %d has arrived at time %d.\n", id, priority, getClk());
                        char *argsProcess[3] = {"./process.out", message.mtext, NULL};
                        if (execv(argsProcess[0], argsProcess) == -1) {
                            perror("execv failed");
                            exit(EXIT_FAILURE);
                        }
                    } else if (pid > 0) {
                        MLFP_Node *newNode = (MLFP_Node *)malloc(sizeof(MLFP_Node));
                        newNode->pid = id;
                        newNode->priority = priority;
                        newNode->next = NULL;
                        add_procces_MLFP(&running_queue[priority], newNode);
                        if (currentPriority > priority) {
                            currentPriority = priority;
                            quantum_counter = 0;
                            activeProcess = id;
                        }
                    } else {
                        perror("Fork failed");
                    }
                } else {
                    printf("Process %d: Memory allocation failed\n", process.id);
                }
            }

            // Check for available processes in current priority queue
            int count = 0;
            while (running_queue[currentPriority].count == 0) {
                if (count >= 11) {
                    break;
                }
                currentPriority = (currentPriority + 1) % 11;
                count++;
            }
            if (count == 11) {
                continue;
            }
            activeProcess = running_queue[currentPriority].head->pid;

            // Handle quantum expiration
            if (quantum_counter == quantum) {
                Advance_process_MLFP(&running_queue[currentPriority]);
                quantum_counter = 0;
                if (iteration == running_queue[currentPriority].count) {
                    iteration = 0;
                    int oldPriority = currentPriority;
                    int minChange = 11;
                    currentPriority = (currentPriority + 1) % 11;
                    printf("going to next priority queue %d\n", currentPriority);
                    while (running_queue[oldPriority].count > 0) {
                        MLFP_Node *temp = running_queue[oldPriority].head;
                        remove_process_MLFP(&running_queue[oldPriority], running_queue[oldPriority].head->pid, 0);
                        if (oldPriority == 10) {
                            add_procces_MLFP(&running_queue[temp->priority], temp);
                            if (minChange > temp->priority) {
                                minChange = temp->priority;
                            }
                            currentPriority = minChange;
                        } else {
                            add_procces_MLFP(&running_queue[currentPriority], temp);
                            iteration++;
                        }
                    }
                } else {
                    printf("going to next process %d\n", running_queue[currentPriority].head->pid);
                }
            }

            // Increment quantum counter
            if (running_queue[currentPriority].head) {
                quantum_counter++;
            }

            // Send messages to processes in current queue
            if (running_queue[currentPriority].head) {
                printf("running process %d\n", running_queue[currentPriority].head->pid);
                int id = running_queue[currentPriority].head->pid;
                message.mtype = id;

                advancePCBtable(pcbtable, id, activeProcess, process_count);
                activeProcess = id;
                if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                    perror("msgsnd failed");
                }
            }

            // Handle process completion and memory deallocation
            usleep(100000);
            int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
           /* if (rec_val == -1) {
            perror("msgrcv failed"); // This prints the error
            printf("Error: msgrcv returned -1, errno=%d\n", errno); // Prints error code
            } else {
            printf("Message received: mtype=%ld, text=%s\n", message.mtype, message.mtext);
            }*/

           // printf(" received value %d",rec_val);
            if (rec_val != -1) {
                printf("removing process %d\n", running_queue[currentPriority].head->pid);
                int freed_pid = running_queue[currentPriority].head->pid;
               // printf("ecwecewc");
                //PCBEntry *pcb = findPCBentry(pcbtable, freed_pid, process_count);//Ahmad
                
                    //int end_address = pcb->start_address + pcb->allocated_size - 1;//Ahmad
                    int end_address = 0;
                    int freed_start =0;
                    int freed_size =0;
                    free_memory(freed_pid, &freed_start, &freed_size);
                //    printf("ewfwefewf");
                   // log_memory_operation("freed", getClk(), pcb->allocated_size, pcb->start_address, end_address, freed_pid);
                

                remove_process_MLFP(&running_queue[currentPriority], freed_pid, 1);
                finishedProcs++;
                quantum_counter = 0;
            }
        }
    }
    printf("finished all processes\n");
}


void run_RR(int to_sched_msgq_id) {
    PCBEntry pcbtable[process_count + 1];
    int activeProcess = 0;

    RR_Queue running_queue;
    running_queue.head = NULL;

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    if (to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    initialize_memory(); // Initialize the Buddy Memory System
    int time_progress = getClk();
    int finishedProcs = 0, quantum_counter = 0;

    while (finishedProcs < process_count) {
        if (getClk() == time_progress + 1) {
            printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
            usleep(50000);
            time_progress = getClk();
            last_clk = getClk() + 1;
            msgbuff message;

            while (msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
                //int id, runtime, priority,arrival , memsize;//Ahmad 2
                int id, runtime, priority, memsize;
                //sscanf(message.mtext, "%d %d %d %d %d", &id, &runtime, &priority,&arrival ,&memsize);//Ahmad
                int ret = sscanf(message.mtext, "%d %d %d %d", &id, &runtime, &priority ,&memsize);//Ahmad2
               // printf("[ ID: %d,RUNTIME: %d, Priority: %d,arrival: %d,memsize: %d\n", id, runtime, priority,arrival ,memsize);//Ahmad
                
                //int ret = sscanf(message.mtext, "%d %d %d %d", &id, &runtime, &priority, &memsize);
                //if (ret != 4) { //Ahmad
                if (ret != 4) {
                     fprintf(stderr, "Error: Malformed message received: '%s'. Expected 4 values, but got %d.\n", message.mtext, ret);
                    continue; // Skip this message and move to the next one
                    }

                if (first_clk == 0) {
                    first_clk = getClk();
                }

                struct Process process = {id, getClk(), runtime, memsize};

                BuddyBlock *allocated_block = allocate_memory(process.memsize,process.id);
                if (allocated_block) {
                    log_memory_operation("allocated", process.arrival_time, process.memsize, allocated_block->start, allocated_block->start + allocated_block->size - 1, process.id);
                    printf("Process %d allocated %d bytes at address %d\n", process.id, process.memsize, allocated_block->start);

                    PCBEntry *pcb = findPCBentry(pcbtable, id, process_count);
                    if (pcb) {
                        pcb->start_address = allocated_block->start;
                        pcb->allocated_size = allocated_block->size;
                    }

                    int pid = fork();
                    if (pid == 0) {
                        printf("Process %d with priority %d has arrived at time %d.\n", id, priority, getClk());
                        char *argsProcess[3] = {"./process.out", message.mtext, NULL};
                        if (execv(argsProcess[0], argsProcess) == -1) {
                            perror("execv failed");
                            exit(EXIT_FAILURE);
                        }
                    } else if (pid > 0) {
                        Node *newNode = (Node *)malloc(sizeof(Node));
                        newNode->pid = id;
                        newNode->next = NULL;
                        Add_process_RR(&running_queue, newNode);

                        addPCBentry(pcbtable, id, getClk(), runtime, priority);
                    } else {
                        perror("Fork failed");
                    }
                } else {
                    printf("Process %d: Memory allocation failed\n", process.id);
                }
            }

            if (quantum_counter == quantum) {
                if (running_queue.head != NULL) {
                    printf("Quantum has ended on %d at time %d", running_queue.head->pid, getClk());
                    Advance_process_RR(&running_queue);
                    printf(", switching to next process %d.\n", running_queue.head->pid);
                    quantum_counter = 0;
                }
            }

            if (running_queue.head) {
                quantum_counter++;
            }

            if (running_queue.head) {
                int id = running_queue.head->pid;
                message.mtype = id;

                advancePCBtable(pcbtable, id, activeProcess, process_count);
                activeProcess = id;
                if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                    perror("msgsnd failed");
                }
            }

            usleep(100000);
            int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
            if (rec_val != -1) {
                int freed_pid = running_queue.head->pid;
                int freed_start = 0;
                int freed_size = 0;
                
               // print_RR_queue(&running_queue);
                Remove_Process_RR(&running_queue, freed_pid);
               // print_RR_queue(&running_queue); 

                finishedProcs++;
                quantum_counter = 0;

                // Free memory after process completes
                //PCBEntry *pcb = findPCBentry(pcbtable, freed_pid, process_count);
                //if (pcb) {
                   // printf("IDACO pid %d\n  ",freed_pid);
                    free_memory(freed_pid, &freed_start, &freed_size);
                  //  printf("Freeing results: start=%d, requested_size=%d, end=%d\n", 
                   // freed_start, freed_size, freed_start + freed_size - 1);

                    //printf("Free memory results: start=%d, size=%d\n", freed_start, freed_size);
                    //printf("idaco start address %d, allocated size %d",pcb->start_address,pcb->allocated_size);
                    //log_memory_operation("freed", getClk(), pcb->allocated_size, pcb->start_address, pcb->start_address + pcb->allocated_size - 1, pcb->pid);//Ahmad
                    //log_memory_operation("freed", getClk(), freed_size, freed_start,  freed_start + freed_size - 1, freed_pid);
                //}
            }
        }
    }
    final_performance();
    while (wait(NULL) > 0);

    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}



void run_PHPF(int to_sched_msgq_id) {
    PCBEntry pcbtable[process_count+1];
    int activeProcess = 0;

    PHPF_Queue running_queue;
    running_queue.head = NULL;

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    key_t semkey = ftok("semfile", 75);
    int sem_id = semget(semkey, 1, IPC_CREAT | 0666);

    if (to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    initialize_memory(); // Initialize Buddy Memory System
    int time_progress = getClk();
    int finishedProcs = 0;

    while (finishedProcs < process_count) {
        if (getClk() == time_progress + 1) {
            printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
            usleep(50000);
            time_progress = getClk();
            last_clk = getClk() + 1;

            msgbuff message;

            // Process Arrival
            while (msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
                int id, runtime, priority, memsize;
                sscanf(message.mtext, "%d %d %d %d", &id, &runtime, &priority, &memsize);

                if (first_clk == 0) {
                    first_clk = getClk();
                }

                // Allocate memory for the process
                struct Process process = {id, getClk(), runtime, memsize};
                BuddyBlock *allocated_block = allocate_memory(process.memsize,process.id);
                if (allocated_block) {
                    log_memory_operation("allocated", process.arrival_time, process.memsize, allocated_block->start, allocated_block->start + allocated_block->size - 1, process.id);
                    printf("Process %d allocated %d bytes at address %d\n", process.id, process.memsize, allocated_block->start);

                    addPCBentry(pcbtable, id, getClk(), runtime, priority);

                    PCBEntry *pcb = findPCBentry(pcbtable, id, process_count);
                    if (pcb) {
                        pcb->start_address = allocated_block->start;
                        pcb->allocated_size = allocated_block->size;
                    }

                    int pid = fork();
                    if (pid == 0) {
                        char *argsProcess[4] = {"./process.out", message.mtext, NULL};
                        if (execv(argsProcess[0], argsProcess) == -1) {
                            perror("execv failed");
                            exit(EXIT_FAILURE);
                        }
                    } else if (pid > 0) {
                        Priority_Node *newNode = (Priority_Node *)malloc(sizeof(Priority_Node));
                        newNode->priority = priority;
                        newNode->pid = id;
                        newNode->next = NULL;
                        Add_Process_PHPF(&running_queue, newNode);
                    } else {
                        perror("Fork failed");
                    }
                } else {
                    printf("Process %d: Memory allocation failed\n", process.id);
                }
            }

            // Run the highest-priority process
            if (running_queue.head) {
                int id = running_queue.head->pid;
                message.mtype = id;

                advancePCBtable(pcbtable, id, activeProcess, process_count);
                activeProcess = id;
                if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                    perror("msgsnd failed");
                }
            }

            // Handle Process Completion
            usleep(80000);
            int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
           /* if (rec_val == -1) {
                perror("msgrcv failed"); // This prints the error
                printf("Error: msgrcv returned -1, errno=%d\n", errno); // Prints error code
                } else {
                printf("Message received: mtype=%ld, text=%s\n", message.mtype, message.mtext);
                }

            printf("btngan\n");*/
            // printf("%d\n",rec_val);
             
           /**/ if (rec_val != -1) {
               // printf("edwefwef\n");
                
                int freed_pid = running_queue.head->pid;
                printf("%d",freed_pid);
                //PCBEntry *pcb = findPCBentry(pcbtable, freed_pid, process_count);//Ahmad
                
                 //Free memory 

                  // int end_address =pcb->start_address + pcb->allocated_size - 1;//Ahmad
                  int end_address =0;//Ahmad
                     int freed_start =0;//ahmad
                    int freed_size =0;
                    free_memory(freed_pid, &freed_start, &freed_size); //Ahmad
                    
                   // printf("Process %d freed %d bytes from address %d to %d\n", freed_pid, freed_size, freed_start, freed_start+freed_size-1);
                

                Remove_Process_PHPF(&running_queue);
                if (running_queue.head == NULL) {
                    activeProcess = 0;
                }
                finishedProcs++;
                //printf("fwefewf");
            }
            else {
               // printf("xxxxxxxx\n");
            }
        }
    }

    // Final Performance Logging
    final_performance();
    while (wait(NULL) > 0);

    // Clean up resources
    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}

void run_SJF(int to_sched_msgq_id) {
     PCBEntry pcbtable[process_count+1];
    int activeProcess = 0;

    SJF_Queue running_queue;
    running_queue.head = NULL;

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    if(to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    initialize_memory(); // Initialize the buddy memory system
    int time_progress = getClk();
    int finishedProcs = 0;
    while (finishedProcs < process_count) {
    if (getClk() == time_progress + 1) {
        printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
        usleep(50000);
        time_progress = getClk();
        last_clk=getClk()+1;

        msgbuff message;

        while(msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
            int id, runtime, priority, memsize;
            sscanf(message.mtext, "%d %d %d %d ", &id, &runtime, &priority, &memsize);
            int ret = sscanf(message.mtext, "%d %d %d %d", &id, &runtime, &priority, &memsize);
            if (ret != 4) {
                fprintf(stderr, "Error: Malformed message received: '%s'. Expected 4 values, but got %d.\n", message.mtext, ret);
                continue; // Skip this message and move to the next one
            }

            if(first_clk==0)
            {
                first_clk=getClk();
            }
            struct Process process = {id, getClk(), runtime, memsize};

            BuddyBlock *allocated_block = allocate_memory(process.memsize,process.id);
            if (allocated_block) {
                log_memory_operation("allocated", process.arrival_time, process.memsize, allocated_block->start, allocated_block->start + allocated_block->size - 1, process.id);
                printf("Process %d allocated %d bytes at address %d\n", process.id, process.memsize, allocated_block->start);

                int pid = fork();
                if (pid == 0) {
                    char *argsProcess[4] = {"./process.out", message.mtext, NULL};
                    if (execv(argsProcess[0], argsProcess) == -1) {
                        perror("execv failed");
                        exit(EXIT_FAILURE);
                    }
                } else if (pid > 0) {
                    Priority_Node* newNode = (Priority_Node*)malloc(sizeof(Priority_Node));
                    newNode->priority = runtime;
                    newNode->pid = id;
                    newNode->next = NULL;
                    Add_Process_SJF(&running_queue, newNode);

                    addPCBentry(pcbtable, id, getClk(), runtime, priority);

                    pcbtable[id].start_address = allocated_block->start;
                    pcbtable[id].allocated_size = allocated_block->size;
                } else {
                    perror("Fork failed");
                }
            } else {
                printf("Process %d: Memory allocation failed\n", process.id);
            }
        }

        if (running_queue.head) {
            int id = running_queue.head->pid;
            message.mtype = id;
            
            advancePCBtable(pcbtable, id, activeProcess, process_count);
            activeProcess = id;
            if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                perror("msgsnd failed");
            }
        }

        usleep(80000);
        int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
        if(rec_val != -1) {
            int freed_pid = running_queue.head->pid;
            int freed_start =0;
            int freed_size =0;
            free_memory(freed_pid, &freed_start, &freed_size);
            Remove_SJF(&running_queue);
            if(running_queue.head == NULL) {
                activeProcess = 0;
            }
            finishedProcs++;

            PCBEntry *pcb = findPCBentry(pcbtable, freed_pid, process_count);
           
           
            /*int freed_start =0;
            int freed_size =0;
            free_memory(freed_pid, &freed_start, &freed_size);*/
            // log_memory_operation("freed", getClk(), pcb->allocated_size, pcb->start_address, pcb->start_address + pcb->allocated_size - 1, freed_pid);
            printf("Process %d freed %d bytes at address %d\n", freed_pid, pcb->allocated_size, pcb->start_address);
            
        }
        }
    }
    final_performance();
    while (wait(NULL) > 0);

    // Clean up resources
    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}


int main(int argc, char *argv[]) {
    log_files_init();


    int to_sched_msgq_id, send_val;
    key_t key_id = ftok("procfile", 65);
    to_sched_msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    algorithm = atoi(argv   [1]); //Ahmad
    //algorithm = atoi(argv   [3]); 

    quantum = 0;
    if (algorithm == 3 || algorithm == 4) {
        quantum = atoi(argv[2]); //Ahmad
       //quantum = atoi(argv[5]);
    }
    sscanf(argv[3], "%d", &process_count); //Ahmad
    //sscanf(argv[1], "%d", &process_count);

    printf("sub from scheduler with algorithm %d and quantum %d and procs %d\n", algorithm, quantum, process_count);

    switch (algorithm) {
        case 1:
            run_SJF(to_sched_msgq_id);
            break;
        case 2:
            run_PHPF(to_sched_msgq_id);
            break;
        case 3:
            run_RR(to_sched_msgq_id);
            break;
        case 4:
            run_MLFP(to_sched_msgq_id);
            break;
    }

    exit(0);
}
