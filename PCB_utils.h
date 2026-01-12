
typedef struct{
    int pid;
    int arrival_time;
    int run_time;
    int priority;
    int remaining_time;
    int state;
    int totalwait;
    int start_address;    // Address of allocated memory block
    int allocated_size;   // Size of allocated memory block
}PCBEntry;
const char *filePath = "scheduler.log";
FILE *file ;


float total_runtime=0.0;
float total_wta=0.0;
float total_waiting=0.0;
void get_performance(float arr[])
{
    arr[0]=total_runtime;
    arr[1]=total_wta;
    arr[2]=total_waiting;
}
void log_files_init()
{
    file = fopen(filePath, "w");

    fprintf(file,"#At time x process y state arr w total z remain y wait k\n");

}
void log_files_close()
{

    fclose(file);

}

void addPCBentry(PCBEntry* pcbtable, int id, int arrival_time, int run_time, int priority) {
    pcbtable[id].pid = id;
    pcbtable[id].arrival_time = arrival_time;
    pcbtable[id].run_time = run_time;
    pcbtable[id].priority = priority;
    pcbtable[id].remaining_time = run_time;
    pcbtable[id].state = 0; // 0: ready, 1: running 2: blocked
    pcbtable[id].totalwait = 0;
     pcbtable[id].start_address = -1; // Default to unallocated
    pcbtable[id].allocated_size = 0; // Default to zero
}

void DecrementPCBentrytime(PCBEntry* pcbtable, int id) {
    pcbtable[id].remaining_time--;
    if(pcbtable[id].remaining_time == 0) {
        pcbtable[id].state = 0;
        int TA=getClk()+1-pcbtable[id].arrival_time;
        float WTA=(float)TA/pcbtable[id].run_time;
        total_runtime+=pcbtable[id].run_time;
        total_waiting+=pcbtable[id].totalwait;
        total_wta+=WTA;
        fprintf(file,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk()+1, id, pcbtable[id].arrival_time, pcbtable[id].run_time, pcbtable[id].remaining_time, pcbtable[id].totalwait,TA,WTA);
        fflush(file);
    }
}

void advancePCBtable(PCBEntry* pcbtable, int activeProcess, int oldProcess, int process_count) {
    if(oldProcess == 0)
        oldProcess = activeProcess;

    if(oldProcess != activeProcess && pcbtable[oldProcess].state == 1) {
        fprintf(file,"At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), oldProcess, pcbtable[oldProcess].arrival_time, pcbtable[oldProcess].run_time, pcbtable[oldProcess].remaining_time, pcbtable[oldProcess].totalwait);
        fflush(file);
        pcbtable[oldProcess].state = 2;
    }

    if(pcbtable[activeProcess].state == 2) {
        fprintf(file,"At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), activeProcess, pcbtable[activeProcess].arrival_time, pcbtable[activeProcess].run_time, pcbtable[activeProcess].remaining_time, pcbtable[activeProcess].totalwait);
        fflush(file);
    }
    else if(pcbtable[activeProcess].state == 0) {
        fprintf(file,"At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), activeProcess, pcbtable[activeProcess].arrival_time, pcbtable[activeProcess].run_time, pcbtable[activeProcess].remaining_time, pcbtable[activeProcess].totalwait);
        fflush(file);
    }
    pcbtable[activeProcess].state = 1;

    DecrementPCBentrytime(pcbtable, activeProcess);

    for(int i = 1; i <= process_count; i++) {
        if(pcbtable[i].pid != activeProcess) {
            pcbtable[i].totalwait++;
        }
    }
   

}
PCBEntry *findPCBentry(PCBEntry *pcbtable, int pid, int process_count) {
    for (int i = 0; i < process_count; i++) {
        if (pcbtable[i].pid == pid) {
            return &pcbtable[i];
        }
    }
    return NULL; // Return NULL if not found
}
