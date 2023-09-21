#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "data_share.h"

typedef struct{
    long startNano;
    int startSec;
    int occupied;
    int pid;
}PCB;

PCB procesTable[20];


//system clock increment 
void incremntClock(int increm,  Clocksys *clock){
    clock->nanoSec += increm;
    if(clock->nanoSec >=1000000000){
        clock->nanoSec -= 1000000000;
        clock->sec += 1;
    }
}


int main(int argc, char **argv){
    /*parse options and recieve command line arguments.
    getopt() use to pass the command line options
    loops runs untill getopt() returns -1
    switch statement value of options to determine options passed
    if unknown option passed exit from the program*/
 
    int opt_val = 0;
    int n=0;
    int s=0;
    int t=0;
    int increment = 2000000000/3;  // increment clock my 1 millisecond
    int stillChildrentoLaunch = 1;
    
	while((opt_val = getopt(argc, argv, "hn:s:t:")) != -1){ // getopt() function take arguments -n, -s ,-t
		switch (opt_val){
	
		case 'h':
            printf("******* Help Menu *******\n\n");
			printf("oss [-h] [-n proc] [-s simul] [-t iter]\n\n");
            printf("proc - number of total children to launch.\n");
            printf("simul- how many children to run simulataneously.\n");
            printf("iter -- time that a child process will be launched\n");
            printf("\n\n");
			break; 
		case 'n':
            n = atoi(optarg);
			break;

		case 's':
			s = atoi(optarg);
			break;
    
        case 't':
			t = atoi(optarg);
			break;

		default:
            fprintf(stderr,"Invalid option entered\n");           
			exit(1);

		}
    }

    
    //check for non negative arguments
    if(n <= 0 || s <= 0 || t <= 0){
        fprintf(stderr,"Invalid arguments, Please enter non negative arguments\n");           
		exit(1);
    }
    

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

    //initialize sec and nanosec to zero in shared memory
    clock->sec = 0;
    clock->nanoSec = 0;

    //Initilalize process table
    for(int i=0; i<20; i++){
        procesTable[i].occupied = 0;
        procesTable[i].pid =0;
        procesTable[i].startNano= 0;
        procesTable[i].startSec =0;
        
    }

    printf("Now at tge oss.c\n");
    int i=0;
    while(i < 10){
        incremntClock(increment,clock);
        printf("Sec %d and NanoSecond %ld\n",clock->sec, clock->nanoSec);
        i++;
    }

    
    

    // get random number between 1 and t for seconds and nano seconds to pass to worker
      srand(time(0));
    
    int sec_worker = (rand()%(t-1)) + 1;
    long nanoSec_worker = (rand()%1000000000) +1;


    // go into a loop and start doing a fork() and then an exec() call to launch worker process.
    //should only do this upto simul number of times. 

    
   int simul_counter =0;
   
   
    for(int tot_chldrn=0; tot_chldrn<n;){
        
       if(simul_counter < s){
            
            printf("simul is %d and children %d\n\n",simul_counter+1, tot_chldrn+1);

           
            pid_t childPid = fork(); // This is where the child process splits from the parent
            

            if(childPid == 0){ //child process call worker "t" times
                char args_sec[2];
                char args_nanosec[3];
                sprintf(args_sec,"%d",sec_worker );
                sprintf(args_nanosec,"%ld",nanoSec_worker );
                
                 execl("./worker","worker",args_sec,args_nanosec,NULL);

                perror("exec failed\n");
                exit(1);
               

                
            }else if(childPid > 0){
                
             // wait(0);
            

              simul_counter += 1;
              tot_chldrn += 1;
  
                
            }else{
                fprintf(stderr, "Exec failed, terminating");
                exit(1);
            }

        }else{
           
            //wait for child to finish
            simul_counter = 1;
            printf("\n***child to finish***\n");
            //printf("simul is %d and children %d\n\n",simul_counter+1, tot_chldrn+1);
            wait(0);
            simul_counter -= 1;  
            
        }  
     
    
    }
  
        //wait for all remaining child process to finish
        while(simul_counter > 0){
            
            wait(0);
            simul_counter -=1;
            
        }
    

    return 0;
}