#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    //take not more than two input if not error message
    printf("Now in worker\n");
    
    //sec and nano secs form oss files and 
    int sec_from_oss = 0;
    int nanosec_from_oss = 0;
    

    sec_from_oss = atoi(argv[1]);
    nanosec_from_oss = atoi(argv[2]);
    printf("%d   %d\n",sec_from_oss, nanosec_from_oss);

    if (argc != 3){
        fprintf(stderr,"Usage: %s seconds and nanoseconds\n",argv[0]);
        exit(1);
    }

    
  
   /* 
    pid_t pid = getpid();
    pid_t ppid = getppid();

    for(int i=0; i<iter; i++){
    printf("worker PID:%d PPID:%d iterations:%d before sleeping\n\n",pid, ppid, i+1);

    sleep(1);

    printf("worker PID:%d PPID:%d iterations:%d after sleeping\n",pid, ppid, i+1);
    }
    */
    
    return EXIT_SUCCESS;
} 