#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    //take not more than one input if not error message
    
    int iter =0;
    
    if (argc != 2){
        fprintf(stderr,"Usage: %s <iterations>\n",argv[0]);
        exit(1);
    }

    iter = atoi(argv[1]);
    
    
    
    pid_t pid = getpid();
    pid_t ppid = getppid();

    for(int i=0; i<iter; i++){
    printf("worker PID:%d PPID:%d iterations:%d before sleeping\n\n",pid, ppid, i+1);

    sleep(1);

    printf("worker PID:%d PPID:%d iterations:%d after sleeping\n",pid, ppid, i+1);
    }
    
    return EXIT_SUCCESS;
} 