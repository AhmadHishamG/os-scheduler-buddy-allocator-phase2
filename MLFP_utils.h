typedef struct MLFP_Node {
    int pid;
    int priority;
    struct MLFP_Node* next;
    int memory_size;
    bool first_run;
} MLFP_Node;

typedef struct {
    struct MLFP_Node* head;
    int count;
}MLFP_Queue;

void add_procces_MLFP(MLFP_Queue* queue, MLFP_Node* new_node) {
    if(queue->head == NULL) {
        queue->head = new_node;
        queue->head->next = new_node;
        queue->count++;
        return;
    }

    if(queue->head->next == queue->head) {
        queue->head->next = new_node;
        new_node->next = queue->head;
        queue->count++;
        return;
    }

    MLFP_Node* temp = queue->head->next;               
    while(temp->next != queue->head) {
        temp = temp->next;
    }

    new_node->next = queue->head;
    temp->next = new_node;
    queue->count++;
}

void Advance_process_MLFP(MLFP_Queue* queue) {
    if(queue->head == NULL) {
        return;
    }
    queue->head = queue->head->next;
}

bool remove_process_MLFP(MLFP_Queue* q, int pid, int delete) {
    if (!q || q->head == NULL) return false;

    MLFP_Node* current = q->head;
    MLFP_Node* previous = NULL;
    
    // Case 1: Only one node in the queue
    if (current->next == current) {
        if (current->pid == pid) {
            q->head = NULL;
            if(delete) {
                free(current);
            }
            q->count--;
            return true;
        }
        return false; // PID not found
    }

    // Case 2: Multiple nodes in the queue
    do {
        if (current->pid == pid) {
            if (current == q->head) {
                // Update the head pointer
                q->head = current->next;

                // Find the tail and update its next pointer
                MLFP_Node* tail = q->head;
                while (tail->next != current) {
                    tail = tail->next;
                }
                tail->next = q->head; // Maintain the circular link
            } else if (previous) {
                previous->next = current->next;
            }

            if(delete) {
                free(current);
            }
            q->count--;
            return true;
        }

        previous = current;
        current = current->next;
    } while (current != q->head);

    return false; // PID not found
}

void Print_MLFP_Queue(MLFP_Queue* q) {
    if (!q || q->head == NULL) {
        printf("The Round Robin Queue is empty.\n");
        return;
    }

    printf("Round Robin Queue Elements:\n");

    MLFP_Node* current = q->head;
    do {
        printf("Process PID: %d, Next PID: %d\n", 
               current->pid, 
               current->next->pid);
        current = current->next;
    } while (current != q->head);
}