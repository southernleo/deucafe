#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Global static variables
#define CUSTOMER_NUMBER 25
#define CUSTOMER_ARRIVAL_TIME_MIN 1
#define CUSTOMER_ARRIVAL_TIME_MAX 3
#define REGISTER_NUMBER 5
#define COFFEE_TIME_MIN 2
#define COFFEE_TIME_MAX 5

// Mutex and semaphore for synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t empty;
sem_t full;

// Keeps track of the available registers
int available_registers[REGISTER_NUMBER];

// Generates a random number between min and max
int get_random_number(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

// The customer thread
void* customer_thread(void* arg) {
    // Get the customer ID from the argument
    int customer_id = *((int*) arg);

    // 
    /* Try to find an open register-->
    The register_id variable is used to store the ID of an available register. 
    The code acquires a lock on the mutex and then enters a loop to find an available register.
    If no register is available, the code releases the mutex and waits on the semaphore before trying again.  
    If an available register is found, the code breaks out of the loop, releases the mutex, and exits the function.
      */
    int register_id = -1;
    pthread_mutex_lock(&mutex);
    while (1) {
        for (int i = 0; i < REGISTER_NUMBER; i++) {
            if (available_registers[i]) {
                register_id = i;
                available_registers[i] = 0;
                break;
            }
        }
        if (register_id >= 0) {
            break;
        }
        pthread_mutex_unlock(&mutex);
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&mutex);

    // Customer goes to the register
    printf("CUSTOMER %d GOES TO REGISTER %d.\n", customer_id, register_id);

    // Wait for the coffee to be ready
    int coffee_time = get_random_number(COFFEE_TIME_MIN, COFFEE_TIME_MAX);
    sleep(coffee_time);

    /*Customer gets the coffee and leaves This code  releasing a register that was previously acquired. 
     It acquires a lock on the mutex, marks the register as available, releases the mutex, 
     and then increments the semaphore to indicate that a new register is available.
    This allows other threads that are waiting for a register to be available to continue execution*/
    pthread_mutex_lock(&mutex);
    available_registers[register_id] = 1;
    pthread_mutex_unlock(&mutex);
    sem_post(&full);
    printf("CUSTOMER %d FINISHED BUYING FROM REGISTER %d AFTER %d SECONDS.\n", customer_id, register_id, coffee_time);
    // Return from the customer thread
    return NULL;
}
int main() {
    // Initialize the random seed
    srand(time(NULL));

    // Initialize the mutex and semaphores
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, 0);
    sem_init(&full, 0, REGISTER_NUMBER);

    // Initialize the available registers
    for (int i = 0; i < REGISTER_NUMBER; i++) {
        available_registers[i] = 1;
    }
    // Create the customer threads
    pthread_t threads[CUSTOMER_NUMBER];
    for (int i = 0; i < CUSTOMER_NUMBER;i++) {
        int* customer_id = malloc(sizeof(int));
        *customer_id = i;
         int arrival_time = get_random_number(CUSTOMER_ARRIVAL_TIME_MIN, CUSTOMER_ARRIVAL_TIME_MAX);
         sleep(arrival_time); // Wait for a random amount of time between the range of customerarrivaltimemax and customerarrivaltimemin
         printf("CUSTOMER %d IS CREATED AFTER %d SECONDS.\n", i, arrival_time);
        pthread_create(&threads[i], NULL, customer_thread, (void*) customer_id);
    }
    // Wait for the customer threads to finish
    for (int i = 0; i < CUSTOMER_NUMBER; i++) {
        pthread_join(threads[i], NULL);
    }
    // Destroy the mutex and semaphores
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    // Return from main
    return 0;
}
