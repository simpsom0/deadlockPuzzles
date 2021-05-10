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
#define MAX_SIDE_BABOON 10
#define MAX_ROPE_BABOON 5
#define NUM_THREADS 3
#define TIME_TO_CROSS 5


// models a rope hanging over a canyon
typedef struct rope_s
{
    int going_left;
    int going_right;
    int num_baboons;
    int num_going_left;
    int num_going_right;
} rope_s;


// models a side of the canyon
typedef struct side_s
{
    int num_baboons;
    int type;
    int side;
    FILE *fp;
} side_s;


// global variables
time_t start_t, end_t;
pthread_mutex_t lock;
pthread_cond_t hold;
int threads_left = NUM_THREADS;
rope_s rope;


// init declaration of functions
void init_locks();
void free_locks();
void init_rope();
void init_side(side_s*);
void* check_deadlock(void*);
void* deadlock_left_canyon(void*);
void* deadlock_right_canyon(void*);
void* left_canyon(void*);
void* right_canyon(void*);


// initializes locks
void init_locks()
{
    if (pthread_mutex_init(&lock, NULL)!=0)
        perror("mutex_init() failed");

    if (pthread_cond_init(&hold, NULL)!=0)
        perror("cond_init() failed");
}


// frees locks
void free_locks()
{

    if (pthread_mutex_destroy(&lock) !=0)
        perror("mutex_destroy() failed");

    if (pthread_cond_destroy(&hold) != 0)
        perror("cond_destroy() failed");
}


// initializes rope struct
void init_rope()
{
    rope.going_left = 0;
    rope.going_right = 0;
    rope.num_baboons = 0;
    rope.num_going_left = 0;
    rope.num_going_right = 0;
}


// can init to random size if necessary
void init_side(side_s *side)
{
    int b;
    int errornum;
    char dir[27];
    char path;

    side->num_baboons = MAX_SIDE_BABOON;

    if(side->type == 0) // deadlock
        strcpy(dir, "output/deadlock");
    else // solution
        strcpy(dir, "output/solution");

    if(side->side == 0) // left
        strcat(dir, "/l_side.txt\0");
    else // right
        strcat(dir, "/r_side.txt\0");

    side->fp = fopen(dir, "w");
    if (side->fp == NULL)
    {
        errornum = errno;
        printf("Failed to open file. errno:%d\n", errornum);
        perror("Error: ");
        exit(1);
    }
}


// monitoring function that exits the program on any error/deadlock/integrity violation
void* check_deadlock(void* args)
{
    FILE *fp;
    int *which_type = (int *) args;
    char dir[28];
    
    if(*which_type == 0) // deadlock
        strcpy(dir, "output/deadlock/monitor.txt\0");
    else // solution
        strcpy(dir, "output/solution/monitor.txt\0");

    fp = fopen(dir, "w");
    
    while(threads_left > 1)
    {
        if(rope.going_left == 1 && rope.going_right == 1)
        {
            printf("\nERROR: data integrity corrupted.\nBoth sides are crossing at once.\n");
            fprintf(fp, "\nERROR: data integrity corrupted.\nBoth sides are crossing at once.\n");
            printf("exiting...");
            fprintf(fp, "exiting...");
            exit(-1);
        }
        if(rope.num_baboons > MAX_ROPE_BABOON)
        {
            printf("\nERROR: data integrity corrupted.\nToo many baboons on the rope at once.");
            fprintf(fp, "\nERROR: data integrity corrupted.\nToo many baboons on the rope at once.");
            printf("exiting...");
            fprintf(fp, "exiting...");
            exit(-1);
        }
        if(rope.num_going_left > 0 && rope.num_going_right > 0)
        {
            printf("\nERROR: data integrity corrupted.\nBaboons from both sides are crossing.");
            fprintf(fp, "\nERROR: data integrity corrupted.\nBaboons from both sides are crossing.");
            printf("exiting...");
            fprintf(fp, "exiting...");
            exit(-1);
        }
        if(rope.num_going_left + rope.num_going_right > rope.num_baboons)
        {
            printf("\nERROR: data integrity corrupted.\nIncongruent number of baboons");
            fprintf(fp, "\nERROR: data integrity corrupted.\nIncongruent number of baboons");
            printf("exiting...");
            fprintf(fp, "exiting...");
            exit(-1);
        }

        sleep(1);
    }
    threads_left--;
    fclose(fp);
    return NULL;
}

// the deadlock left canyon handler
void* deadlock_left_canyon(void* args)
{
    side_s *left = (side_s *) args;

    while(left->num_baboons > 0)
    {
        if(rope.going_left == 0 || rope.going_right == 1)
        {
            rope.going_right = 1;
            int amount = (rand() % left->num_baboons) + 1;
            //printf("amount: %d  baboons left:%d\n", amount, (left->num_baboons - amount));

            if(amount > MAX_ROPE_BABOON)
                amount = MAX_ROPE_BABOON;

            rope.num_going_right += amount;
            rope.num_baboons += amount;
            left->num_baboons -= amount;

            printf("\n%d Baboons started going right across the rope.\n", amount);
            fprintf(left->fp, "%d Baboons started going right across the rope.\n", amount);

            sleep(TIME_TO_CROSS);

            printf("%d Baboons made it across safely!!\n", amount);
            fprintf(left->fp, "%d Baboons made it across safely!!\n", amount);

            rope.going_right = 0;
            rope.num_going_right -= amount;
            rope.num_baboons -= amount;
        }
    }
    threads_left--;
    fclose(left->fp);
    return NULL;
}


void* deadlock_right_canyon(void* args)
{
    side_s *right = (side_s *) args;

    while(right->num_baboons > 0)
    {
        if(rope.going_right == 0 || rope.going_left == 1)
        {
            rope.going_left = 1;
            int amount = (rand() % right->num_baboons) + 1;
            //printf("amount: %d  baboons left:%d\n", amount, (right->num_baboons - amount));


            if(amount > MAX_ROPE_BABOON)
                amount = MAX_ROPE_BABOON;

            rope.num_going_left += amount;
            rope.num_baboons += amount;
            right->num_baboons -= amount;

            printf("\n%d Baboons started going left across the rope.\n", amount);
            fprintf(right->fp, "%d Baboons started going left across the rope.\n", amount);
            
            sleep(TIME_TO_CROSS);
            
            printf("%d Baboons made it across safely!!\n", amount);
            fprintf(right->fp, "%d Baboons made it across safely!!\n", amount);

            rope.going_left = 0;
            rope.num_going_left -= amount;
            rope.num_baboons -= amount;
            
        }
    }
    threads_left--;
    fclose(right->fp);
    return NULL;
}


void* left_canyon(void* args)
{
    side_s *left = (side_s *) args;

    while(left->num_baboons > 0)
    {
        if(rope.going_left == 0 || rope.going_right == 1)
        {
            pthread_mutex_lock(&lock);

            rope.going_right = 1;
            int amount = (rand() % left->num_baboons) + 1;
            //printf("amount: %d  baboons left:%d\n", amount, (left->num_baboons - amount));

            if(amount > MAX_ROPE_BABOON)
                amount = MAX_ROPE_BABOON;

            rope.num_going_right += amount;
            rope.num_baboons += amount;
            left->num_baboons -= amount;

            printf("\n%d Baboons started going right across the rope.\n", amount);
            fprintf(left->fp, "%d Baboons started going right across the rope.\n", amount);

            sleep(TIME_TO_CROSS);

            printf("%d Baboons made it across safely!!\n", amount);
            fprintf(left->fp, "%d Baboons made it across safely!!\n", amount);

            rope.going_right = 0;
            rope.num_going_right -= amount;
            rope.num_baboons -= amount;
            
            printf("amount: %d  baboons left: %d\n", amount, left->num_baboons);

            pthread_mutex_unlock(&lock);
            sleep(1);
        }
    }
    threads_left--;
    fclose(left->fp);
    return NULL;
}


void* right_canyon(void* args)
{
    side_s *right = (side_s *) args;

    while(right->num_baboons > 0)
    {
        if(rope.going_right == 0 || rope.going_left == 1)
        {
            pthread_mutex_lock(&lock);

            rope.going_left = 1;
            int amount = (rand() % right->num_baboons) + 1;
            //printf("amount: %d  baboons left:%d\n", amount, (right->num_baboons - amount));


            if(amount > MAX_ROPE_BABOON)
                amount = MAX_ROPE_BABOON;

            rope.num_going_left += amount;
            rope.num_baboons += amount;
            right->num_baboons -= amount;

            printf("\n%d Baboons started going left across the rope.\n", amount);
            fprintf(right->fp, "%d Baboons started going left across the rope.\n", amount);
            
            sleep(TIME_TO_CROSS);
            
            printf("%d Baboons made it across safely!!\n", amount);
            fprintf(right->fp, "%d Baboons made it across safely!!\n", amount);

            rope.going_left = 0;
            rope.num_going_left -= amount;
            rope.num_baboons -= amount;

            printf("amount: %d  baboons left: %d\n", amount, right->num_baboons);

            pthread_mutex_unlock(&lock);
            sleep(1);
        }
    }
    threads_left--;
    fclose(right->fp);
    return NULL;
}


int main(int argc, char **argv)
{
    int i;
    int which_type;
    char path[28];
    char dir[23];
    char input[9];
    time(&start_t);
    srand((unsigned int)time(NULL));
    pthread_t threads[NUM_THREADS];
    side_s left;
    side_s right;

    // debugging
    //strcpy(input, "deadlock\0");

    left.side = 0;
    right.side = 1;

    init_rope();
    init_locks();

    // handles which version to run
    printf("--Baboon Crossing problem--");
    while(strcmp(input, "deadlock") != 0 && strcmp(input,"solution") != 0)
    {
        printf("\nrun deadlock version or solution?\n(\"deadlock\"/\"solution\"): ");
        fgets(input, 9, stdin);
        //printf("input: %s", input);

        if(strcmp(input, "deadlock") != 0 && strcmp(input,"solution") != 0)
        {
            printf("\nERROR: incorrect input. Try again.");
        }
        //fflush(stdin);
    }

    if(strcmp(input, "deadlock") == 0)
    {
        printf("\n(there is a small chance no deadlock will be encountered)");
        which_type = 0;
        left.type = 0;
        right.type = 0;
    }
    else
    {
        which_type = 1;
        left.type = 1;
        right.type = 1;
    }

    init_side(&left);
    init_side(&right);

    printf("\n");

    // initializes threads
    if(pthread_create(&(threads[1]), NULL, check_deadlock, (void *)(&which_type)) != 0)
    {
        printf("thread creation failed\n");
        printf("exiting...\n");
        exit(-1);
    }
    if(pthread_create(&(threads[0]), NULL, deadlock_right_canyon, (void *)(&right)) != 0)
    {
        printf("thread creation failed\n");
        printf("exiting...\n");
        exit(-1);
    }
    if(pthread_create(&(threads[1]), NULL, deadlock_left_canyon, (void *)(&left)) != 0)
    {
        printf("thread creation failed\n");
        printf("exiting...\n");
        exit(-1);
    }
    
    for(i = 0; i < NUM_THREADS; i++)
        pthread_detach(threads[i]);

    while(threads_left > 1)
        sleep(2);

    printf("everyone made it across safely!!!\n");
    printf("if you're seeing this, no deadlock occurred!\n");

    free_locks();
    time(&end_t);
    printf("Secs elapsed:  %g\n", difftime(end_t, start_t));
    return 0;
}
