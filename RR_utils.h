typedef struct Node
{
    int pid;
    int priority;
    int start_time;
    int finish_time;
    struct Node * next ; 
    int memory_size;
    bool first_run;
}Node;

typedef struct {
    Node* head;
} RR_Queue;

bool Advance_process_RR(RR_Queue* q) 
{
    if (!q || q->head == NULL) return false;

    q->head = q->head->next;
    return true;
}

bool Add_process_RR(RR_Queue* q,Node* node)
{
    if (!q || !node) return false;

    if (q->head == NULL) {
        q->head = node;
        node->next = node; 
        return true;
    }

    Node* tail = q->head;
    while (tail->next != q->head) {
        tail = tail->next;
    }

    tail->next = node;
    node->next = q->head; 
    return true;
}


bool Remove_Process_RR(RR_Queue* q, int pid) {
    if (!q || q->head == NULL) return false;

    Node* current = q->head;
    Node* previous = NULL;
    
    // Case 1: Only one node in the queue
    if (current->next == current) {
        if (current->pid == pid) {
            q->head = NULL;
            free(current);
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
                Node* tail = q->head;
                while (tail->next != current) {
                    tail = tail->next;
                }
                tail->next = q->head; // Maintain the circular link
            } else if (previous) {
                previous->next = current->next;
            }

            // Update the active pointer if needed

            free(current);
            return true;
        }

        previous = current;
        current = current->next;
    } while (current != q->head);

    return false; // PID not found
}

Node* Dequeue_Process_RR(RR_Queue* q, int pid) {
    if (!q || q->head == NULL) return false;

    Node* current = q->head;
    Node* previous = NULL;
    
    // Case 1: Only one node in the queue
    if (current->next == current) {
        if (current->pid == pid) {
            q->head = NULL;
            
            return current;
        }
        return NULL; // PID not found
    }

    // Case 2: Multiple nodes in the queue
    do {
        if (current->pid == pid) {
            if (current == q->head) {
                // Update the head pointer
                q->head = current->next;

                // Find the tail and update its next pointer
                Node* tail = q->head;
                while (tail->next != current) {
                    tail = tail->next;
                }
                tail->next = q->head; // Maintain the circular link
            } else if (previous) {
                previous->next = current->next;
            }

            // Update the active pointer if needed


            return current;
        }

        previous = current;
        current = current->next;
    } while (current != q->head);

    return NULL; // PID not found
}


void Print_RR_Queue(RR_Queue* q) {
    if (!q || q->head == NULL) {
        printf("The Round Robin Queue is empty.\n");
        return;
    }

    printf("Round Robin Queue Elements:\n");

    Node* current = q->head;
    do {
        printf("Process PID: %d, Next PID: %d\n", 
               current->pid, 
               current->next->pid);
        current = current->next;
    } while (current != q->head); // Loop until we circle back to the head
}

bool RR_isEmpty(RR_Queue* q)
{
    if (!q || q->head == NULL) {
        return true;
    }

    return false;
}

