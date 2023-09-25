#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "data_share.h"
#include <stdbool.h>

typedef struct{
    long startNano;
    int startSec;
    int occupied;
    int pid;
}PCB;

PCB processTable[20];


//system clock increment 
void incremntClock(int increm,  Clocksys *clock){
    clock->nanoSec += increm;
    if(clock->nanoSec >=1000000000){
        clock->nanoSec -= 1000000000;
        clock->sec += 1;
    }
}

//funtions to check 30seconds times to output the process table

bool timeToOutput(Clocksys *clock){
    int prevSec = clock->sec;
    int timeTo = 0;
    printf("Sec %d and NanoSecond %ld\n",clock->sec, clock->nanoSec);
    
    printf("previous Sec %d\n",prevSec);
    
    if((clock->sec - prevSec) >= 1 ){ 
        prevSec = clock->sec;
        return (timeTo %2 ==0);
    }
    return false; 
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


void checkIfChildHasTerminated(Clocksys *clock) {
    int pid, status;
    
    do{
        pid = waitpid(-1, &status, WNOHANG);
       // printf("PID at checkIfChildHasTerminated() %d\n",pid);
        if (pid == 0) {
          //  printf("at the checkIfChildHasTerminated() ");
            for (int i = 0; i < 20; i++) {
                if (processTable[i].pid == pid) {
                    processTable[i].occupied = 0;
                    processTable[i].startNano = 0;
                    processTable[i].startSec = 0 ;
                    break;
                }
            }
        }

    }while (pid == 0);   
}

void updatePCBOfTerminatedChild(int pid) {
    int i;
    for (i = 0; i < 20; i++) {
        if (processTable[i].occupied && processTable[i].pid == pid) {
            processTable[i].occupied = 0;
            processTable[i].pid = 0;
            processTable[i].startSec = 0;
            processTable[i].startNano = 0;
            break;
        }
    }
    
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
    int increment = 100000;  // increment clock my 1 millisecond
    int stillChildrenToLaunch = 1;
    int childHasTerminated = 0;
    
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

    //create process table
    for(int i=0; i<20; i++){
        processTable->occupied = 0;
        processTable->pid = 0;
        processTable->startSec = 0;
        processTable->startNano =0;
    }

    
    int previousNano = 0;
   // int x =10;

    while(stillChildrenToLaunch){

      //  printf("\nNow incrementing clock\n");
        incremntClock(increment,  clock);
       // printf("now Sec: %d and nanSec:%ld",clock->sec, clock->nanoSec);

        /*output the process table every half second of simulated clock time
            -check simulated lock for 30 seconds
            -output process table */

        if((clock->nanoSec - previousNano) % 500000 == 0){
            printf("OSS PID:%d SysClockS:%d SysclockNano : %ld\n",getpid(),clock->sec,clock->nanoSec);

            printProcessTable();
            int previousNano = clock->nanoSec;
        }

        checkIfChildHasTerminated(clock);

        //update PCB of terminated child
        if(childHasTerminated){
            updatePCBOfTerminatedChild(getpid());

        }

      //  x--;
      //  if(x<2){

         //   stillChildrenToLaunch = 0;
      //  }
    }


    // get random number between 1 and t for seconds and nano seconds to pass to worker
        
    srand(time(0));
    int sec_worker = (rand()%(t-1)) + 1;
    long nanoSec_worker = (rand()%1000000000) +1;
    printf("\npassing t Sec: %d and nanoSec%ld\n\n",sec_worker,nanoSec_worker);


    pid_t childPid = fork(); // This is where the child process splits from the parent
                
    if(childPid == 0){ //child process call worker "t" times
                char args_sec[2];
                char args_nanosec[3];
                    
                sprintf(args_sec,"%d",sec_worker);
                sprintf(args_nanosec,"%ld",nanoSec_worker);
                execl("./worker","worker",args_sec,args_nanosec,NULL);
                
                     
                perror("exec failed\n");
                exit(1);

    }else if(childPid > 0){
        //waitpid
        int status;
        printf("now at oss in parent at oss\n");
        int wait_pid = waitpid(-1, &status, WNOHANG);
        printf("waitPid %d",wait_pid);
                    
        if(wait_pid == 0){
        
            addToProcessTable(getpid(), clock->sec, clock->nanoSec);
                            
            // simul_counter += 1;
            // tot_chldrn += 1;
            printProcessTable();
    }
    

    }else{
        stillChildrenToLaunch == 0 ;
    }

     //Detach shared memory segment from process address space
    if (shmdt(clock) == -1) {
            perror("shmdt");
            exit(EXIT_FAILURE);
    }

    // Destroy shared memory segment
    if (shmctl(shm_clock, IPC_RMID, NULL) == -1) {
    
        perror("shmctl");
        exit(EXIT_FAILURE);
    } 
    
    return 0;
}   


/*
    
        
   

        

            
                    
                
                    
                }else{
                
                    fprintf(stderr, "Exec failed, terminating");
                    exit(1);
                }

            }else{
            
                //wait for child to finish
               // simul_counter = 1;
                printf("\n***child to finish***\n");
                //printf("simul is %d and children %d\n\n",simul_counter+1, tot_chldrn+1);
                wait(0);
              //  simul_counter -= 1;  
                
            }  

        
        
        
  
        //wait for all remaining child process to finish
        while(simul_counter > 0){
            
            wait(0);
          /  simul_counter -=1;
            stillChildrentoLaunch =0;
        } 
       

*/

    
 /*     
        // go into a loop and start doing a fork() and then an exec() call to launch worker process.
        //should only do this upto simul number of times. 

    
        int simul_counter =0;
   
   
        for(int tot_chldrn=0; tot_chldrn<n;){
            
        if(simul_counter < s){
            int previousNano = 0;     
           // printf("simul is %d and children %d\n\n",simul_counter+1, tot_chldrn+1);
            
            
            
        
        

            
*/
   
         
        
    
    