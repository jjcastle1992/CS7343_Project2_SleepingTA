/*********************
Name: James Castle
CS 7343 Programming Assignment 2 - Project 2
Purpose: Implementaation of the Sleeping Teaching Assistant (Project 2)
**********************/

#import <random>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <semaphore>

struct TeachingAssistant {
    int numStudents;
    int taState;  // 0 = sleeping, 1 = waking, 2 = helping
    int studentsStillStudying;
};

struct Student {
    int studentId;
    int timeToCompleteHw; // Starting time to complete assignment (randomized)
    int timeRemaining; // time left to complete assignment
    int taVistsAttempted;   // Total number of attempted TA visits (attempted to get help/queue for help)
    int taVisitsSucceeded;  // Times actually got help from TA (visited the TA and/or got a seat in the hall)
    int taBusyTimes;  // Times the seats in the hall were full
    bool homeworkFinished;
};

// ************GLOBAL VARIABLES********* TERRIBLE PRACTICE
std::binary_semaphore s(1); // our semaphore acting as a mutex lock
std::vector<Student*> students;
std::vector<Student*> hallway;
Student *walkinStudent = nullptr;
int studytime = 10; // time quantum for studying before seeking help.

enum taStatus {Sleeping, Waking, Helping};

void taOfficeHours (TeachingAssistant *currentTa){
    // While students are still finishing their assignments, continue to run (work, wait, sleep)
    while(currentTa->studentsStillStudying != 0){

//        **** TO IMPLEMENT****** SLEEP AND WAKEUP INSTEAD OF STRAIGHT TO QUEUE
        // See if there is a walk-in students (no one in chairs so no one in queue, but student waiting to be served)
        if(walkinStudent){
            if(currentTa->taState == Sleeping){
                std::cout << "TA Waiting to be woken up by Walk-in" << std::endl;
            }
            else{
                Student *currentStudent = walkinStudent;
                std::cout << "TA working with Student:" << currentStudent->studentId << std::endl;
                currentStudent = nullptr;
            }
        }

        // Checking for students in hall chairs
        if((!hallway.empty()) && (currentTa->taState != Sleeping)){
            // Critical Section
            Student *currentStudent = hallway[0]; // Call in the next student
            hallway.erase(hallway.begin()); // Dequeue the student off the queue
            std::cout << "TA working with Student:" << currentStudent->studentId << std::endl;
        }

        // No students (going to sleep)
        else{
            std::cout << "TA getting sleepy..." << std::endl;
            currentTa->taState = Sleeping;
        }
    }
    // Exit once all students have finished their assignments
    std::cout << "ALl students done! TA Going home!" << std::endl;
}

bool visitTa (Student *currentStudent, TeachingAssistant *currentTa) {
    bool successfulVisit = false;
    // Called by all students who havent finished after studying for a quantum
    if((!walkinStudent) && (hallway.empty())){
        // Walk in, acquire semaphore, and wake up TA if needed
        std::cout << "TA being woken up by Student: " << currentStudent->studentId << std::endl;
        currentTa->taState = Waking;
        // Acquire Semaphore

        // Set current Student
        walkinStudent = currentStudent;
        successfulVisit = true;
    }

    // Acquire/Wait (walk-in, grab a chair, or go back to studying)

    else if (hallway.size() != 3){  // Grab a chair if one is available
        hallway.push_back(currentStudent);
        successfulVisit = true;
    }

    // Release/Signal (Tell the next student to go in)
        // Remainder
    return successfulVisit;
}

void study (Student* currentStudent, TeachingAssistant *currentTa){
    // Called by all students
    while(currentStudent->timeRemaining > 0){  // // study for a quantum (or less if remaining time < quantum)
        for (int i = 0; i < studytime; i++){
            if(currentStudent->timeRemaining <= 0){
                break;
            }
            currentStudent->timeRemaining--;
        }
        // If not done, check if TA is available
        if(currentStudent->timeRemaining >0){
            bool taVisited = visitTa(currentStudent, currentTa);
            currentStudent->taVistsAttempted++;
            if(taVisited){
                currentStudent->taVisitsSucceeded++;
            }
            else{
                currentStudent->taBusyTimes++;
            }
        }
    }


    std::cout << "Student " << currentStudent->studentId << " Done Studying!";
    currentStudent->homeworkFinished = true;
    currentTa->studentsStillStudying--;
}

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
    unsigned int seed = 2; // to ensure same results

    // Student config variables
    int numberStudents = 4; // Number of students working on programming assignments
    int minAssignmentCompletionTime = 10; // min time taken to complete the assignment for each student
    int maxAssignmentCompletionTime = 100; // max time taken to complete the assignment for each student


    // Make our TA and our students
    auto *ourTa = new TeachingAssistant;
    ourTa->studentsStillStudying = numberStudents;
    ourTa->numStudents = numberStudents;
    ourTa->taState = Sleeping;

    // Create Students
    for (int i = 0; i < numberStudents; i++){
        // Initialize student
        auto *currentStudent = new Student;
        currentStudent->studentId = i;
        currentStudent->timeToCompleteHw = randomRangeGen(maxAssignmentCompletionTime, minAssignmentCompletionTime, seed + i);
        currentStudent->timeRemaining = currentStudent->timeToCompleteHw;
        currentStudent->taVistsAttempted = 0;
        currentStudent->taVisitsSucceeded = 0;
        currentStudent->taBusyTimes = 0;
        currentStudent->homeworkFinished = false;

        // add to vector of students
        students.push_back(currentStudent);
    }

    std::cout << "TA and students made" << std::endl;

    // Start TA office hours
    while(ourTa->studentsStillStudying > 0){
        taOfficeHours(ourTa);
        for(int stu = 0; stu < students.size(); stu++){
            study(students[stu], ourTa);
        }
    }

    std::cout << "done" << std::endl;

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
    delete ourTa;

    return 0;
}
