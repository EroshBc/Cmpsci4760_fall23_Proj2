#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

//create  processtable for shared memory
//sec  to store sceconds an nanoSec - to store nanseconds 
typedef struct {
    time_t sec;
    long nanoSec;
}PCB;


int main(int argc, char **argv){
    
    //generate unique key - ftok
    key_t key_clock = ftok("key_clock", 4760);

    //returns an identifier in shmid- shmget
    int shm_clock = shmget(key_clock, sizeof(PCB),0666|IPC_CREAT);
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
    

    
    //initailize sec amd nanoSec and set to zero in shared memory
    clockData->sec = 0;
    clockData->nanoSec = 0;

    


    
    //deattach the shared memeory

    //free the memory 
    
    return 0;


}