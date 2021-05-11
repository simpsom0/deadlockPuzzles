# Deadlock Puzzles
### Clarkson University
### CS444 - Operating Systems
### Final Project
## Description:
### Dining Philosophers:
A well known synchronization problem where N philosophers sit around a table, each with a plate of noodles, and N chopsticks spread out evenly. Each philosopher goes through the routine of thinking, grabbing chopsticks, eating, then finally putting said chopsticks down and returning to thinking. Deadlock is easily reached if you're not careful while coding this. Consider the situation where every philosopher decides to eat at the same time - first they all grab the chopstick to the right of them, then reach for the one on their left, but it isn't there. Each philosopher is in a state of deadlock, since in order to eat, they all need two chopsticks. Yet since each philosopher is holding one chopstick and waiting on the one to their right, no one can eat, and therefore no one can put a chopstick back down. 

There's multiple ways to fix this, the route i took involved using the pthread library to utilize mutex locks. With mutex locks, threads can pass a token back and forth in a cordinated manor. I chose to make this token grant the ability of picking up/putting down both adjacent chopsticks at once. This way, only one philosopher at a time is allowed to pickup/put down chopsticks, hence, breaking the never ending cycle of waiting.

### Baboon Crossing:

### Contents:
Included in this repo are two main folders. They have .c files, executables and an output folder.
#### .c file:
These contain the respective code base of each synchronization puzzle.
#### executable:
Compiled using:

    gcc -g -o executable filename.c

-g flag is optional, only used for debugging
how to run:

    ./executable
#### output folder:
There are two sub-folders within named "solution" and "deadlock". Once an executable is ran, depending on whether you chose to run the deadlock or solution version, the output of each thread will be printed to .txt files in order to view the results in a more organized way.
