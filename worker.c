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

int main(int argc, char *argv[]){


    //******* shared memory ******* //
    key_t key = ftok("oss.c", 12316);
    if(key == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //IPC not used as worker reading from oss only

    int shm_id = shmget(key,sizeof(Clocksys),0666);
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
    // ***** end shared memory creating *****

    
    if (argc != 3){
        fprintf(stderr,"Usage: %s seconds and nanoSeconds\n",argv[0]);
        exit(1);
    }
    
    //get the time from excl
    int sec_from_oss = atoi(argv[1]);
    long nanosec_from_oss = atol(argv[2]);

    //calculate the the termainate to be seconds and nanoseconds
    int termSec = sec_from_oss + clock->sec;

    long termNano = nanosec_from_oss + clock->nanoSec;
    if(termNano>1000000000){
        termNano -= 1000000000;
        termSec += 1;
    }
    int pid = getpid();
    int ppid = getppid();

    //starting point 
    printf("worker PID:%d PPID:%d  \n",pid, ppid);
    printf("SysClockS:%d  SysclockNan:%ld  TermTimeS: %d TermTimeNano: %ld\n",clock->sec,clock->nanoSec,termSec, termNano);
    printf("--Just starting\n\n");

    
    
    while(true){
        int previous_Sec = clock->sec;

        //calculate the the termainate to be seconds and nanoseconds
        int termSec = sec_from_oss + clock->sec;

        long termNano = nanosec_from_oss + clock->nanoSec;
        if(termNano>1000000000){
            termNano -= 1000000000;
            termSec += 1;
        }
        int count = 0;
        if((clock->sec - previous_Sec) == 1  ){
            count += 1;
            printf("worker PID:%d PPID:%d  \n",pid, ppid);
            printf("SysClockS:%d  SysclockNan:%ld  TermTimeS: %d TermTimeNano: %ld\n",clock->sec,clock->nanoSec,termSec, termNano);
            printf("--%d seconds have passed from starting\n\n",count);
            previous_Sec = clock->sec;
        }
        if(clock->sec > termSec || clock->sec == termSec && clock->nanoSec >= termNano ){
            printf("target time completed\n");
            printf("SysClockS:%d  SysclockNan:%ld  TermTimeS: %d TermTimeNano: %ld\n",clock->sec,clock->nanoSec,termSec, termNano);
            printf("--Terminating\n\n");
            exit(EXIT_SUCCESS);
        }

    }
    
     //detached shared memory
    if (shmdt(clock) == -1) {
            perror("shmdt");
            exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}