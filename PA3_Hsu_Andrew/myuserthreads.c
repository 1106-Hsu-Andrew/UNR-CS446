
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#include <signal.h>

#include <ucontext.h>

#include <stdatomic.h>

#include "modify_value.h"

#define REPS 100000
#define STACK_SIZE 64 * 1024

int counter = 0;
atomic_flag flag = ATOMIC_FLAG_INIT;
struct ucontext_t ctxMain, ctxTask1, ctxTask2;
struct ucontext_t* ctxCurr;

int task_1_finished = 0;
int task_2_finished = 0;


void task_1_func() {
    printf("Task 1 started.\n");

    for (int i=0; i<REPS; ++i) {
		

        // implement spin-guard lock()-ing //
        while(atomic_flag_test_and_set(&flag)){}

        while (rand() % 8 != 0) {}  // short, random delay to prevent loop unrolling

        if (i % 1000 == 0) { printf("Task 1 running.\n"); }  // debug print every 1000 reps

        modify_value_up(&counter);


        // implement spin-guard unlock()-ing //
        atomic_flag_clear(&flag);

    }
    
    printf("Task 1 finished.\n");
    task_1_finished = 1;
}

void task_2_func() {
    printf("Task 2 started.\n");

    for (int i=0; i<REPS; ++i) {
		

        // implement spin-guard lock()-ing //
        while(atomic_flag_test_and_set(&flag)){}

        while (rand() % 8 != 0) {}  // short, random delay to prevent loop unrolling

        if (i % 1000 == 0) { printf("Task 2 running.\n"); }  // debug print every 1000 reps

        modify_value_down(&counter);


        // implement spin-guard unlock()-ing //
        atomic_flag_clear(&flag);
    }
    
    printf("Task 2 finished.\n");
    task_2_finished = 1;
}


void time_slice_expired_handler(int signal) {
    printf("\t\tTIME SLICE EXPIRED\n");

    // implement simple scheduling between tasks //
    ucontext_t* ctxPrev = ctxCurr;
    if((ctxCurr == &ctxTask2 || ctxCurr == &ctxMain) && !task_1_finished){
        ctxCurr = &ctxTask1;
    }
    else if((ctxCurr == &ctxTask1 || ctxCurr == &ctxMain) && !task_2_finished){
        ctxCurr = &ctxTask2;
    }

    if(ctxPrev != ctxCurr){
        swapcontext(ctxPrev, ctxCurr);
    }
}


int main(int argc, char *argv[]){
    struct itimerval timer;

    srand(time(NULL));  // initialize random number generator

    // implement task1 and task2 ucontext setup //
    void* stack1 = malloc(STACK_SIZE);
    if(stack1 == NULL){
        printf("Stack 1 memory allocation failed.");
        return -1;
    }

    void* stack2 = malloc(STACK_SIZE);
    if(stack2 == NULL){
        printf("Stack 2 memory allocation failed.");
        return -1;
    }
    
    getcontext(&ctxMain);

    getcontext(&ctxTask1);
    ctxTask1.uc_stack.ss_sp = stack1;
    ctxTask1.uc_stack.ss_size = STACK_SIZE;
    ctxTask1.uc_link = &ctxMain;
    makecontext(&ctxTask1, task_1_func, 0);

    getcontext(&ctxTask2);
    ctxTask2.uc_stack.ss_sp = stack2;
    ctxTask2.uc_stack.ss_size = STACK_SIZE;
    ctxTask2.uc_link = &ctxMain;
    makecontext(&ctxTask2, task_2_func, 0);

    // implement interval timer setup //
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1000;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1000;
    
    setitimer(ITIMER_REAL, &timer, NULL);
    signal(SIGALRM, time_slice_expired_handler);

    ctxCurr = &ctxMain;

    printf("Main started.\n");
	
    while (!task_1_finished || !task_2_finished) {
        printf("\t\tTasks 1 && 2 not both Finished yet - Waiting...\n");
		
		// implement any other task scheduling operations you might need here //
		
        pause();  // pause the main Thread, to wait for delivery of the the next timer-based signal 
    }
    free(stack1);
    free(stack2);
	
    printf("Main: Finished. Final counter: %d\n", counter);
 
    return 0;  
}
