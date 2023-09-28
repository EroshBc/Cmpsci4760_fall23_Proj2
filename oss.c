#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "data_share.h"
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct{
    long startNano;
    int startSec;
    int occupied;
    int pid;
    int terminated;
}PCB;
PCB processTable[20];

//system clock increment 
void incremntClock(int increm,  Clocksys *clock){
    clock->nanoSec += increm;
    if(clock->nanoSec >=1000000000/2){
        clock->nanoSec -= 1000000000/2;
        clock->sec += 1;
    }
}

// Function to print the process table
void printProcessTable() {
    printf("Process Table:\n");
    printf("Entry\tOccupied\tPID\tStartS\tStartN\n");
    for (int i = 0; i < 20; i++) {
        printf("%d\t%d\t\t%d\t%d\t%ld\n", i, processTable[i].occupied,
               processTable[i].pid, processTable[i].startSec, processTable[i].startNano);
    }
    printf("\n");
}

// Function to add a new process to the process table
void addToProcessTable(pid_t pid, int startSeconds, int startNano) {
    for (int i = 0; i < 20; i++) {
        if (!processTable[i].occupied) {
            processTable[i].occupied = 1;
            processTable[i].pid = pid;
            processTable[i].startSec = startSeconds;
            processTable[i].startNano = startNano;
            break;
        }
    }
}

void updatePCBOfTerminatedChild(int terminated_pid) {
    // Update process table entry for terminated child
    int i;
    for (i = 0; i < 20; i++) {
        if (processTable[i].occupied && processTable[i].pid == terminated_pid) {
            //Update the the PCB for terminated fields
            processTable[i].occupied = 0;
            processTable[i].pid = 0;
            processTable[i].startSec = 0;
            processTable[i].startNano = 0;
            break;
        }
    }
    
}


int main(){
    int stillChildrenToLaunch = 1;
    int previousNano = 0;
    int increment = 1000000000/2;
    
    /*create simualted clock in shared memory */
    

    // ***** shared memory *******
    key_t key = ftok("oss.c", 12316);
    if(key == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int shm_id = shmget(key,sizeof(Clocksys),0666|IPC_CREAT);
    if(shm_id == -1){
        perror("shm_id");
        exit(EXIT_FAILURE);
    }

    //Attach to the shared memory
    Clocksys *clock = (Clocksys *) shmat(shm_id, NULL, 0);

    if((void *) clock == (void *) -1){
        perror("shmat for clock\n");
        exit(EXIT_FAILURE);
    }

    clock->sec = 0;
    clock->nanoSec = 0;


    // ****** end of shared memory crete *******
    
    //create process table
    for(int i=0; i<20; i++){
        processTable->occupied = 0;
        processTable->pid = 0;
        processTable->startSec = 1;
        processTable->startNano =200;
    } 

    
        
    

    while(stillChildrenToLaunch){
        
        incremntClock(increment, clock);
        printf("OSS PID:%d SysClockS:%d SysclockNano : %ld\n",getpid(),clock->sec,clock->nanoSec);

        /*output the process table every half second of simulated clock time
            -check simulated lock for 30 seconds
            -output process table*/ 
        
        if((clock->nanoSec - previousNano == 1) % 500000 == 0){

            printProcessTable();
            int previousNano = clock->nanoSec;
        }

        int pid, status;
        pid = waitpid(-1, &status, WNOHANG);

        if(pid > 0){
            updatePCBOfTerminatedChild(pid);
        }else{
        stillChildrenToLaunch = 0;
        }
    }

    // get random number between 1 and t for seconds and nano seconds to pass to worker
       
    int t = 3;

    srand(time(0));
    int sec_worker = (rand()%(t-1)) + 1;
    long nanoSec_worker = (rand()%1000000000) +1;
    printf("\npassing t Sec: %d and nanoSec%ld\n\n",sec_worker,nanoSec_worker);
    
    pid_t childPid = fork();

    if(childPid == 0){ //child process call worker "t" times
                char args_sec[2];
                char args_nanosec[3];
                    
                sprintf(args_sec,"%d",sec_worker);
                sprintf(args_nanosec,"%ld",nanoSec_worker);
                execl("./worker","worker",args_sec,args_nanosec,NULL);
                    
                perror("exec failed\n");
                exit(1);

    }else if(childPid > 0){
        int status;
        printf("Now at oss\n");
        int wait_pid = waitpid(-1, &status, WNOHANG);
        printf("waitPid %d\n\n",wait_pid);

        if(wait_pid == 0){
            addToProcessTable(getpid(), clock->sec, clock->nanoSec);
            printProcessTable();
        

        }else{
            stillChildrenToLaunch ==0;
        }

    }
    
    //detached shared memory
    if (shmdt(clock) == -1) {
            perror("shmdt");
            exit(EXIT_FAILURE);
    }

    // Destroy shared memory segment
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
    
        perror("shmctl");
        exit(EXIT_FAILURE);
    } 

    return 0;
}