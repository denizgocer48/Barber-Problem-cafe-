#include <unistd.h>          // Provides API for POSIX(or UNIX) OS for system calls
#include <stdio.h>           // Standard I/O Routines
#include <stdlib.h>          // For exit() and rand()
#include <pthread.h>         // Threading APIs
#include <semaphore.h>       // Semaphore APIs

#define CUSTOMER_NUMBER 25
#define CUSTOMER_ARRIVAL_TIME_MIN 1
#define CUSTOMER_ARRIVAL_TIME_MAX 3 
#define REGISTER_NUMBER 5
#define COFFEE_TIME_MIN 2
#define COFFEE_TIME_MAX 5


sem_t customer; // Semaphore
sem_t registerr; //Semaphore
sem_t mutex; // Semaphore for providing mutially exclusive access
int waitingCustomer = 0; // Counter for waiting customers
int customerRegister[CUSTOMER_NUMBER]; // To exchange pid between customer and register
int registerId = 0; // Index for next legitimate register
int customerId = -1; // Index to choose customer
int id = -1; // Counter of customers
void registerThread(void *tmp); // Thread Function
void customerThread(void *tmp); // Thread Function
void bringCustomer(); // Function of generating customers's arrival time
void giveCoffee(); // Function of generating coffee taking time
static int coffee_time, customer_arrival_time;

int main()
{   
    pthread_t registers[REGISTER_NUMBER], customers[CUSTOMER_NUMBER]; // Thread declaration
    int i = 0;

    // Semaphore initialization
    sem_init(&customer, 0, 0);
    sem_init(&registerr, 0, 0);
    sem_init(&mutex, 0, 1);

    for(i = 0; i < REGISTER_NUMBER; i++) // Creation of register threads
    {   
        pthread_create(&registers[i], NULL, (void *)registerThread, (void*)&i); // Create register thread
    }

    // Customer thread initialization
    for(i = 0; i < CUSTOMER_NUMBER; i++) // Creation of Customer Threads
    {   
        pthread_create(&customers[i], NULL, (void *)customerThread, (void*)&i);  // Create customer thread
    }   

    for(i = 0; i < CUSTOMER_NUMBER; i++) {
        pthread_join(customers[i], NULL);
    }
        
    exit(EXIT_SUCCESS);  // Exit abandoning infinite loop of register thread
}

void customerThread(void *tmp)  // Customer Process
{   
    bringCustomer(); // Create customers in random interval
    sleep(customer_arrival_time);
    waitingCustomer++;
    int r = 0;
    sem_wait(&mutex);  // Lock mutex to protect register changes
    id++;
    printf("CUSTOMER %d IS CREATED AFTER %d SECONDS.\n", id, customer_arrival_time); // Print the customer's creating process
    if(waitingCustomer > 0) // Continue the process if there are customers waiting
    {
        --waitingCustomer; // Reduce the number of waiting customers
        printf("CUSTOMER %d GOES TO REGISTER %d.\n", id, r); // Print customer's going to register
        registerId = (++registerId) % REGISTER_NUMBER;
        customerRegister[id] = registerId; // Keep in which register the customer is
        sem_post(&mutex);                   // Release mutex
        sem_post(&registerr);               // Run register
        sem_wait(&customer);                // Join queue of waiting customers
        sem_wait(&mutex);                   // Lock mutex to protect register changes
        r = customerRegister[id];
        waitingCustomer++;                  // Increase the number of waiting customers
        sem_post(&mutex);                   // Release the mutex
    }

    pthread_exit(0);
}

void registerThread(void *tmp) // Register Process
{   
    int index = *(int *)(tmp);
    int cr = 0; // In which register the customer is
    while(1) // Infinite loop   
    {
        giveCoffee();
        sleep(coffee_time); // Wait while taking coffee
        sem_wait(&registerr);
        sem_wait(&mutex);
        customerId++;
        cr = customerRegister[customerId]; // Get selected register's ID
        customerRegister[customerId] = pthread_self(); // Leave own ID for customer
        sem_post(&mutex);
        sem_post(&customer); // Call selected customer
        printf("CUSTOMER %d FINISHED BUYING FROM REGISTER %d AFTER %d SECONDS.\n", customerId, cr, coffee_time); // Print the customer's coffee pickup process
    }
}

void bringCustomer() { // Generate customer's arrival time
    int random = rand(); // Generate a random number
    customer_arrival_time =  random % (CUSTOMER_ARRIVAL_TIME_MAX - CUSTOMER_ARRIVAL_TIME_MIN + 1) + CUSTOMER_ARRIVAL_TIME_MIN; // Generate a random number of seconds passed defined by CUSTOMER_ARRIVAL_TIME_MIN and CUSTOMER_ARRIVAL_TIME_MAX variables.
}

void giveCoffee() { // Generate of coffee taking time
    int random2 = rand(); // Generate a random number
    coffee_time = random2 % (COFFEE_TIME_MAX - COFFEE_TIME_MIN + 1) + COFFEE_TIME_MIN; // Generate a random amount of time, defined by COFFEE_TIME_MIN and COFFEE_TIME_MAX variables.
}