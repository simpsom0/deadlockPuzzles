// 
// Clarkson University
// CS444 - Final Project
// diningPhilosophers.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

extern int errno;

// global parameters
#define NUM_PHILOSOPHERS 5 // max 10
#define MAX_THINKING_TIME 10
#define MAX_EATING_TIME 10
#define MAX_CHOPSTICK_WAIT 10
#define DINNER_TIME 5


// global variables
pthread_mutex_t lock[NUM_PHILOSOPHERS];
pthread_cond_t hold;
time_t start_t, end_t;
int chopsticks[NUM_PHILOSOPHERS];
int philosophers_left = NUM_PHILOSOPHERS;

// init declaration of functions
void init_locks();
void free_locks();
void init_chopsticks();
void think(int, FILE *);
void eat(int, FILE *);
void *deadlock_philosophers_life(void *);
void deadlock_wait_chopstick(int, int, FILE *);
void deadlock_free_chopstick(int, int, FILE *);
void *philosophers_life(void *);
void wait_chopstick(int, int, FILE *);
void free_chopstick(int, int, FILE *);


// used to help file i/o with multithreading
typedef struct thread_s 
{
    int index;
    char index_c;
    FILE *fp;
} thread_s;


/*
    Below are five functions used by both the
    deadlock version of Dining Philosophers and the
    solution version of Dining Philosophers
*/


// initializes locks
void init_locks(){
    for(int i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        if (pthread_mutex_init(&lock[i], NULL)!=0)
        perror("mutex_init() failed");
    }

    if (pthread_cond_init(&hold, NULL)!=0)
        perror("cond_init() failed");
}


// frees locks
void free_locks(){
    for(int i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        if (pthread_mutex_destroy(&lock[i]) !=0)
            perror("mutex_destroy() failed");
    }

    if (pthread_cond_destroy(&hold) != 0)
        perror("cond_destroy() failed");
}


// initializes chopsticks array
void init_chopsticks()
{
    for(int i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        chopsticks[i] = 1;
    }
}


// models philosophers "thinking"
void think(int i, FILE *fp)
{
    int t = rand() % MAX_THINKING_TIME;
    printf("P:%d is thinking for %d seconds...\n", i, t);
    fprintf(fp, "P:%d is thinking for %d seconds...\n", i, t);
    sleep(t);
}


// models philosophers "eating"
void eat(int i, FILE *fp)
{
    int t = rand() % MAX_EATING_TIME;
    printf("P:%d is eating for %d seconds...\n", i, t);
    fprintf(fp, "P:%d is eating for %d seconds...\n", i, t);
    sleep(t);
}


/*
    Below are the three main functions used by the
    deadlock version of Dining Philosophers
*/


// models dining philosophers problem
void* deadlock_philosophers_life(void* args)
{
    // chopstick indices
    thread_s *philosopher = (thread_s *) args;
    int i = philosopher->index;
    FILE *fp = philosopher->fp;
    char i_char = philosopher->index_c;
    int j;
    int first = i;
    int second = (i - 1) % NUM_PHILOSOPHERS;

    // handles wrapping
    if(second == -1)
        second = NUM_PHILOSOPHERS - 1;

    for(j = 0; j < DINNER_TIME; j++)
    {
        think(i, fp);

        deadlock_wait_chopstick(i, first, fp);
        deadlock_wait_chopstick(i, second, fp);

        eat(i, fp);

        deadlock_free_chopstick(i, first, fp);
        deadlock_free_chopstick(i, second, fp);
    }

    printf("P:%d is done eating\n", i);
    philosophers_left--;
    fclose(fp);
    return NULL;
}


// waits for chopsticks[C] to become available
void deadlock_wait_chopstick(int p, int c, FILE *fp)
{
    int itr = 0;
    int wait_time = 2;

    while(chopsticks[c] == 0)
    {
        printf("P:%d is waiting for C:%d...\n", p, c);
        fprintf(fp, "P:%d is waiting for C:%d...\n", p, c);
        sleep(wait_time);
        itr += 1;

        if(itr > MAX_CHOPSTICK_WAIT)
        {
            printf("DEADLOCK!!!\n");
            fprintf(fp, "DEADLOCK!!!\n");
            printf("exiting...\n");
            fprintf(fp, "exiting...\n");
            exit(-1);
        }
    }

    chopsticks[c] = 0;
    printf("P:%d grabbed C:%d\n", p, c);
    fprintf(fp, "P:%d grabbed C:%d\n", p, c);
}


// frees chopsticks[C]
void deadlock_free_chopstick(int p, int c, FILE *fp)
{
    if(chopsticks[c] == 1)
    {
        printf("ERROR: P:%d cannot put C:%d down, it's already available.\n", p, c);
        fprintf(fp, "ERROR: P:%d cannot put C:%d down, it's already available.\n", p, c);
    }
    else
    {
        chopsticks[c] = 1;
        printf("P:%d put C:%d down\n", p, c);
        fprintf(fp, "P:%d put C:%d down\n", p, c);
    }
}


/*
    Below are the three main functions used by the
    solution version of Dining Philosophers
*/


void* philosophers_life(void *args)
{
    // chopstick indices
    thread_s *philosopher = (thread_s *) args;
    int i = philosopher->index;
    FILE *fp = philosopher->fp;
    char i_char = philosopher->index_c;
    int j;
    int first = i;
    int second = (i - 1) % NUM_PHILOSOPHERS;

    // handles wrapping
    if(second == -1)
        second = NUM_PHILOSOPHERS - 1;

    for(j = 0; j < DINNER_TIME; j++)
    {
        think(i, fp);

        pthread_mutex_lock(&lock[first]);
        pthread_mutex_lock(&lock[second]);

        wait_chopstick(i, first, fp);
        wait_chopstick(i, second, fp);

        eat(i, fp);

        free_chopstick(i, first, fp);
        free_chopstick(i, second, fp);

        pthread_mutex_unlock(&lock[first]);
        pthread_mutex_unlock(&lock[second]);
    }

    printf("P:%d is done eating\n", i);
    philosophers_left--;
    fclose(fp);
    return NULL;
}


// frees chopsticks[C]
void free_chopstick(int p, int c, FILE *fp)
{
    if(chopsticks[c] == 1)
    {
        printf("ERROR: P:%d cannot put C:%d down, it's already available.\n", p, c);
        fprintf(fp, "ERROR: P:%d cannot put C:%d down, it's already available.\n", p, c);
    }
    else
    {
        chopsticks[c] = 1;
        printf("P:%d put C:%d down\n", p, c);
        fprintf(fp, "P:%d put C:%d down\n", p, c);
    }
}


// waits for chopsticks[C] to become available
void wait_chopstick(int p, int c, FILE *fp)
{
    int itr = 0;
    int wait_time = 2;

    while(chopsticks[c] == 0)
    {
        printf("P:%d is waiting for C:%d...\n", p, c);
        fprintf(fp, "P:%d is waiting for C:%d...\n", p, c);
        sleep(wait_time);
        itr += 1;

        if(itr > MAX_CHOPSTICK_WAIT)
        {
            printf("DEADLOCK!!!\n");
            fprintf(fp, "DEADLOCK!!!\n");
            printf("exiting...\n");
            fprintf(fp, "exiting...\n");
            exit(-1);
        }
    }

    chopsticks[c] = 0;
    printf("P:%d grabbed C:%d\n", p, c);
    fprintf(fp, "P:%d grabbed C:%d\n", p, c);
}


int main(int argc, char **argv)
{
    int i;
    int errornum;
    char input[8];
    pthread_t threads[NUM_PHILOSOPHERS];
    thread_s args[NUM_PHILOSOPHERS];
    srand((unsigned int)time(NULL));
    time(&start_t);
    char path[33];
    char dir[28];

    init_locks();
    init_chopsticks();

    // handles which version to run
    printf("--Dining Philosophers problem--");
    while(strcmp(input, "deadlock") != 0 && strcmp(input,"solution") != 0)
    {
        printf("\nrun deadlock version or solution?\n(\"deadlock\"/\"solution\"): ");
        fgets(input, 9, stdin);
        printf("input: %s", input);

        if(strcmp(input, "deadlock") != 0 && strcmp(input,"solution") != 0)
        {
            printf("\nERROR: incorrect input. Try again.");
        }
        //fflush(stdin);
    }
    
    // handles where to save output
    if(strcmp(input, "deadlock") == 0)
    {
        strcpy(dir, "output/deadlock/philosopher");
    }
    else
    {
        strcpy(dir, "output/solution/philosopher");
    }

    for(i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        args[i].index = i;
        args[i].index_c = i + '0';
        
        strcpy(path, dir);
        strcat(path, &args[i].index_c);
        // issue on 3rd iteration, see "issue.png"
        //strcat(path, ".txt");
        path[28] = '.';
        path[29] = 't';
        path[30] = 'x';
        path[31] = 't';
        path[32] = '\0';

        args[i].fp = fopen(path, "w");

        if (args[i].fp == NULL)
        {
            errornum = errno;
            printf("Failed to open file. errno:%d\n", errornum);
            perror("Error: ");
            exit(1);
        }
        
        if(strcmp(input, "deadlock") == 0)
        {
            printf("\n(there is a small chance no deadlock will be encountered)");
            if(pthread_create(&(threads[i]), NULL, deadlock_philosophers_life, (void *)(&args[i])) != 0)
            {
                printf("thread creation failed\n");
                printf("exiting...\n");
                exit(-1);
            }
        }
        else
        {
            if(pthread_create(&(threads[i]), NULL, philosophers_life, (void *)(&args[i])) != 0)
            {
                printf("thread creation failed\n");
                printf("exiting...\n");
                exit(-1);
            }
        }
    }

    for(i = 0; i < NUM_PHILOSOPHERS; i++)
        pthread_detach(threads[i]);

    while(philosophers_left > 0)
        sleep(2);

    printf("everyone finished eating!!!\n");
    printf("if you're seeing this, no deadlock occurred!\n");

    free_locks();
    time(&end_t);
    printf("Secs elapsed:  %g\n", difftime(end_t, start_t));

    return 0;
}
