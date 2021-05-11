# Deadlock Puzzles
### Clarkson University
### CS444 - Operating Systems
### 
# Description:
## Dining Philosophers:
A well known synchronization problem where N philosophers sit around a table, each with a plate of noodles, and N chopsticks spread out evenly. Each philosopher goes through the routine of thinking, grabbing chopsticks, eating, then finally putting said chopsticks down and returning to thinking. Deadlock is easily reached if you're not careful while coding this. Consider the situation where every philosopher decides to eat at the same time - first they all grab the chopstick to the right of them, then reach for the one on their left, but it isn't there. Each philosopher is in a state of deadlock, since in order to eat, they all need two chopsticks. Yet since each philosopher is holding one chopstick and waiting on the one to their right, no one can eat, and therefore no one can put a chopstick back down. 

There's multiple ways to fix this, the route i took involved using the pthread library to utilize mutex locks. With mutex locks, threads can pass a token back and forth in a cordinated manor. I chose to make this token grant the ability of picking up/putting down both adjacent chopsticks at once. This way, only one philosopher at a time is allowed to pickup/put down chopsticks, hence, breaking the never ending cycle of waiting.

## Baboon Crossing:
A slightly less well know synchronization problem where N baboons are waiting on either side of canyon. Tied to each side and draped over the gap, is a single rope. Each baboon wishes to get to the other side of the canyon, but the rope is only so strong. In order for every baboon to survive, every baboon crossing the rope has to be going in the same direction, and there's only a predetermined amount of baboons allowed on the rope at a time. 

Again, while coding this it's very easy to accidentally have the baboons break the rope. In order to have all the baboons survive, I implemented the pthread library and used a mutex lock. This lock acted as a token that was passed between each side of the canyon, when they held said token, that meant that they were clear to cross and that the other side agreed to wait until every baboon crossed.

## Contents:
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
