/*********************
Name: James Castle
CS 7343 Programming Assignment 2 - Project 2
Purpose: Implementaation of the Sleeping Teaching Assistant (Project 2)
**********************/

#import <random>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <semaphore>

int randomRangeGen(int endRange, int startRange = 0, unsigned int seed = 42) {
    // General implementation borrowed from:
    // https://www.digitalocean.com/community/tutorials/random-number-generator-c-plus-plus
    int random;

    // Random pathway
    if (seed == 42) {
        random = startRange + (rand() % ((endRange - startRange) + 1));
    }
        // Set seed pathway
    else {
        // Modified with ChatGPT to take in a seed.
        // Initialize the random number generator with the provided seed
        std::mt19937 gen(seed);
        // Retrieve a random number between startRange and EndRange
        random = startRange + (gen() % ((endRange - startRange) + 1));
    }
    return random;
}

int main(){
    std::vector<int> hallway = {0, 0, 0};
    int minAssignmentCompletionTime = 10; // min time taken to complete the assignment for each student
    int maxAssignmentCompletionTime = 100; // max time taken to complete the assignment for each student
    int studytime = 10; // time quantum for studying before seeking help.
    // Hallway has 3 chairs for students to wait (array size 3)
        // Check if hallway is empty (student and TA check)

    // TA has 3 states (Sleeping, Awake, Helping)

    // Use threads, mutex locks, and sempaphores

    // Create a separate thread for the TA

        // Check to see if students are waiting for help in the hallway

    // Create a thread for each student
        // Generate the student's study time for the assignment

        // Student threads will either be programming (for a quantum) or seeking help from the TA

        // Do while loop to start first round of programming

            // if finish the assignment, set completion time, number of attempted/missed/successful trips to TA, and completion flag true

            // If not finished at the end of the quantum:
                // Check if there's a line outside the TA's office

                // If so, check if there'a an available chair

                    // If available, get in the first available chair

                    // If not, go back to studying and note an attempt and a miss.

                // Else if there is not a line, check if the TA is sleeping
                    // If yes, awaken with a Semaphore

                    // If not, get help (don't think this is possible)



    return 0;
}
