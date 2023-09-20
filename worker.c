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

    // ********** shared memory *******
     //generate unique key - ftok
    key_t key_clock = ftok("oss.c", 4760);
    if(key_clock == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    //returns an identifier in shmid- shmget
    int shm_clock = shmget(key_clock, sizeof(PCB),0666);
    if(shm_clock == -1){
        perror("shmget for clock");
        return EXIT_FAILURE;
    }
    //Attch to the shared memory for the simulated system clock
    PCB *clockData = (PCB *)shmat(shm_clock,NULL, 0);

    if((void *) clockData == (void *) -1){
        perror("shmat for clock");
        return EXIT_FAILURE;
    }


    //take not more than two input if not error message
    printf("Now in worker\n");


    printf("Reading from oss Sec %d and nanoSec %d\n",clockData->startSec, clockData->startNano);
    
    //sec and nano secs form oss files and 
    int sec_from_oss = 0;
    int nanosec_from_oss = 0;
    

    sec_from_oss = atoi(argv[1]);
    nanosec_from_oss = atoi(argv[2]);
    printf("pass from t \n");
    printf("%d   %d\n",sec_from_oss, nanosec_from_oss);

    if (argc != 3){
        fprintf(stderr,"Usage: %s seconds and nanoSeconds\n",argv[0]);
        exit(1);
    }

    int termSec = sec_from_oss + clockData->startSec;
    int termNano = nanosec_from_oss + clockData->startNano;
    if(termNano>1000000000){
        termNano -= 1000000000;
        termSec += 1;
    }
    
  
    
    pid_t pid = getpid();
    pid_t ppid = getppid();

   // for(int i=0; i<iter; i++){
    printf("worker PID:%d PPID:%d  TermTimeS: %d TermTimeNano: %d \n\n",pid, ppid,termSec, termNano);
    printf("                       SysClockS:%d  SysclockNano:%d",clockData->startSec, clockData->startNano);

    
  //  }
    
    
    return EXIT_SUCCESS;
} 