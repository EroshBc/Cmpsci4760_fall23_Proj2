#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "data_share.h"

int main(int argc, char *argv[]){

    printf("***** Now at worker***\n");
    
    // ********** shared memory *******

     //generate unique key - ftok
    key_t key_clock = ftok("oss.c", 4760);
    if(key_clock == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //returns an identifier in shmid- shmget
    int shm_clock = shmget(key_clock, sizeof(Clocksys),0666|IPC_CREAT);
    if(shm_clock == -1){
        perror("shmget for clock");
        return EXIT_FAILURE;
    }


    //Attch to the shared memory for the simulated system clock
    Clocksys *clock = (Clocksys*)shmat(shm_clock,NULL, 0);

    if((void *) clock == (void *) -1){
        perror("shmat for clock");
        return EXIT_FAILURE;
    }

    //get the time from excl
    int sec_from_oss = atoi(argv[1]);
    long nanosec_from_oss = atol(argv[2]);

    if (argc != 3){
        fprintf(stderr,"Usage: %s seconds and nanoSeconds\n",argv[0]);
        exit(1);
    }

    pid_t pid = getpid();
    pid_t ppid = getppid();

    //calculate the the termainate to be seconds and nanoseconds
    int termSec = sec_from_oss + clock->sec;

    long termNano = nanosec_from_oss + clock->nanoSec;
    if(termNano>1000000000){
        termNano -= 1000000000;
        termSec += 1;
    }


    //starting point 
    printf("worker PID:%d PPID:%d  \n",pid, ppid);
    printf("SysClockS:%d  SysclockNan:%ld  TermTimeS: %d TermTimeNano: %ld\n",clock->sec,clock->nanoSec,termSec, termNano);
    printf("--Just starting\n\n");
    
    sleep(1);
    //while(1){
        
        int previous_Seconds = clock->sec;
        
        //calculate ter
         termSec = sec_from_oss + clock->sec;
         termNano = nanosec_from_oss + clock->nanoSec;

        if(termNano>1000000000){
         termNano -= 1000000000;
         termSec += 1;
        }
       
        
        if(clock->sec > termSec || clock->sec > termSec && clock->nanoSec >= termNano ){
            printf("target time completed\n");
            printf("SysClockS:%d  SysclockNan:%ld  TermTimeS: %d TermTimeNano: %ld\n",clock->sec,clock->nanoSec,termSec, termNano);
            printf("--seconds have passed from the start\n\n");
            exit(EXIT_SUCCESS);
        }

        /*if(clock->sec > previous_Seconds ){
            printf("Second has changed\n");
            previous_Seconds = clock->sec;
        }*/
       
        
    //}
        
    
    printf("**** Now exitting from worker\n");
    
    return EXIT_SUCCESS;
} 