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

using namespace std;

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
std::binary_semaphore taSemaphore(1); // our semaphore for TA thread critical section
std::binary_semaphore studentSemaphore(1); // Mutex lock for student in office
std::counting_semaphore<4> hallwaySemaphore(4); // Semaphore for office + 3 hallway chair access

std::vector<Student*> students;
std::vector<Student*> hallway;
int hallwayIndex = -1; // 0 - 4 where 0 is the office, 1 to 3 are the chairs outside.
int studytime = 10; // time quantum for studying before seeking help.
enum taStatus {Sleeping, Waking, Helping};
mutex outputMutex; // Mutex for synchronizing output

void atomPrint(std::string *message) {
    // Modified to atomically print
    lock_guard<mutex> lock(outputMutex); // Lock the mutex
    cout << *message << endl; // Print the message
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

void taOfficeHours (TeachingAssistant *currentTa){
    // While students are still finishing their assignments, continue to run (work, wait, sleep)
    while(currentTa->studentsStillStudying != 0){

//        **** TO IMPLEMENT****** SLEEP AND WAKEUP INSTEAD OF STRAIGHT TO QUEUE
        // See if there is a walk-in students (no one in chairs so no one in queue, but student waiting to be served)
        if((hallwayIndex >= 0) && (currentTa->taState == Sleeping)) {
            string message = "There is a walk-in, but I am asleep";
            atomPrint(&message);
        }
        // Checking for students in hall chairs
        if((!hallway.empty()) && (currentTa->taState != Sleeping)){
            currentTa->taState = Helping;
            // Critical Section
            Student *currentStudent = hallway[0]; // Call in the next student
            string message = "\nStudent " + to_string(currentStudent->studentId) + " in Office.";
            atomPrint(&message);
            hallway.erase(hallway.begin()); // Dequeue the student off the queue
            message = "TA working with Student:" + to_string(currentStudent->studentId) +
                    " (Study Time Remaining: " + to_string(currentStudent->timeRemaining) + ")";
            atomPrint(&message);
            hallwaySemaphore.release();
        }

        // No students (going to sleep)
        else{
            if(currentTa->taState != Sleeping){
                currentTa->taState = Sleeping;
                string message = "\nTA Falling asleep...";
                atomPrint(&message);
            }
            else{
//                string message = "TA is sleeping...";
//                atomPrint(&message);
            }
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
    // Exit once all students have finished their assignments
    string message = "ALl students done! TA Going home!";
    atomPrint(&message);
}

bool visitTa (Student *currentStudent, TeachingAssistant *currentTa) {
    bool successfulVisit = false;
    // Called by all students who havent finished after studying for a quantum

    if(hallway.empty()){
        hallwaySemaphore.acquire();
        studentSemaphore.acquire();
        hallway.push_back(currentStudent);
        // Walk in and wake up TA if needed
        if(currentTa->taState == Sleeping){
            string message = "\nTA being woken up by Student: " + to_string(currentStudent->studentId);
            atomPrint(&message);
            currentTa->taState = Waking;
        }

        successfulVisit = true;
        studentSemaphore.release(); // Done with the office
    }

    // Acquire/Wait (walk-in, grab a chair, or go back to studying)

    else if (hallway.size() < 4){  // Grab a chair if one is available
        hallwaySemaphore.acquire();
        hallway.push_back(currentStudent);
        successfulVisit = true;
        string message = "Student " + to_string(currentStudent->studentId) + " queued.";
        atomPrint(&message);
    }

    // Release/Signal (Tell the next student to go in)
        // Remainder
    return successfulVisit;
}

void study (Student* currentStudent, TeachingAssistant *currentTa){
    // Called by all students
    while(currentStudent->timeRemaining > 0){  // // study for a quantum (or less if remaining time < quantum)
//        string message = "Student " + to_string(currentStudent->studentId) + " Study Time remaining: " +
//                         to_string(currentStudent->timeRemaining);
//        atomPrint(&message);

        for (int i = 0; i < studytime; i++){
            if(currentStudent->timeRemaining <= 0){
                break;
                currentStudent->homeworkFinished = true;
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
        if(currentStudent->homeworkFinished){
            break;
        }
        else{
            // Sleep
            int sleepTime = randomRangeGen(10, 1, 42 + currentStudent->timeRemaining);  // sleep for 1 to 10 seconds
//            message = "Student " + to_string(currentStudent->studentId) + " Sleeping for " + to_string(sleepTime) +
//                      " seconds";
//            atomPrint(&message);
            this_thread::sleep_for(chrono::seconds(sleepTime));
        }
    }

    string message = "Student " + to_string(currentStudent->studentId) + " Done Studying!";
    atomPrint(&message);
    currentStudent->homeworkFinished = true;
    currentTa->studentsStillStudying--;
}

int main(){
    unsigned int seed = 42; // to ensure same results

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
    std::vector<std::thread> studentThreads;

    while(ourTa->studentsStillStudying > 0){
        std::thread oh(&taOfficeHours, ourTa); // Thread for TA's office hours

        for(int stu = 0; stu < students.size(); stu++){
            studentThreads.emplace_back(&study, students[stu], ourTa);
        }
        for (auto& thread : studentThreads) {
            thread.join();
        }

        // Join TA office hour thread
        oh.join();
    }

    std::cout << "done" << std::endl;

    delete ourTa;

    return 0;
}
