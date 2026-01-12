
typedef struct {
    Priority_Node* head;
} PHPF_Queue;

bool Add_Process_PHPF(PHPF_Queue* q , Priority_Node* node)
{
    if (!q || !node) return false;

    if (q->head == NULL || q->head->priority > node->priority) {
        // Insert at the head
        node->next = q->head;
        q->head = node;
        return true;
    }

    // Find the appropriate position
    Priority_Node* previous = NULL;
    Priority_Node* temp = q->head;
    
    while (temp != NULL && temp->priority <= node->priority) {
        previous = temp;
        temp = temp->next;
    }

    // Insert in the list
    previous->next = node;
    node->next = temp;
    return true;
}
bool Remove_Process_PHPF(PHPF_Queue* q)
{
    if (!q || q->head == NULL) return false;

    Priority_Node* temp = q->head;
    q->head = temp->next;
    free(temp);
    return true;
}