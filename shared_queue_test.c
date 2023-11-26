/**
 * @file
 * @brief Multi-threaded message queue example.
 *
 * This file contains a multi-threaded program demonstrating the use of a
 * shared queue with a single writer and multiple readers. The writer produces
 * messages, and readers consume and process these messages. The program uses
 * pthreads for thread management and synchronization.
 *
 * @author Ajay Neeli
 * @date November 25, 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define MAX_MESSAGES 100
#define NUM_READERS 5

/**
 * @brief Structure for the shared queue.
 */
typedef struct {
    char* messages[MAX_MESSAGES]; /**< Array to store messages. */
    int front, rear; /**< Front and rear indices of the queue. */
    pthread_mutex_t mutex; /**< Mutex for synchronization. */
    pthread_cond_t cond; /**< Condition variable for signaling. */
} SharedQueue;

// Function prototypes
void initQueue(SharedQueue* q);
void enqueue(SharedQueue* q, const char* message);
char* dequeue(SharedQueue* q);
void* writer(void* arg);
void* reader(void* arg);

// Global variables
SharedQueue messageQueue;

//Function Definitions

/**
 * @brief Main function.
 *
 * The main function initializes the shared queue, creates a writer thread,
 * and multiple reader threads. It then joins the threads and cleans up
 * resources after their completion.
 *
 * @return Exit status.
 */
int main() {
    // Initialize the shared queue
    initQueue(&messageQueue);

    // Create writer thread
    pthread_t writerThread;
    if (pthread_create(&writerThread, NULL, writer, NULL) != 0) {
        perror("Error in pthread_create (writer)");
        exit(EXIT_FAILURE);
    }

    // Create reader threads
    pthread_t readerThreads[NUM_READERS];
    int readerIDs[NUM_READERS];
    for (int i = 0; i < NUM_READERS; ++i) {
        readerIDs[i] = i + 1;
        // Create reader threads
        if (pthread_create(&readerThreads[i], NULL, reader, &readerIDs[i]) != 0) {
            perror("Error in pthread_create (reader)");
            exit(EXIT_FAILURE);
        }
    }

    // Join writer thread
    if (pthread_join(writerThread, NULL) != 0) {
        perror("Error in pthread_join (writer)");
        exit(EXIT_FAILURE);
    }

    // Join reader threads
    for (int i = 0; i < NUM_READERS; ++i) {
        if (pthread_join(readerThreads[i], NULL) != 0) {
            perror("Error in pthread_join (reader)");
            exit(EXIT_FAILURE);
        }
    }

    // Destroy mutex and condition variable
    if (pthread_mutex_destroy(&messageQueue.mutex) != 0) {
        perror("Error in pthread_mutex_destroy");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_destroy(&messageQueue.cond) != 0) {
        perror("Error in pthread_cond_destroy");
        exit(EXIT_FAILURE);
    }

    return 0;
}

/**
 * @brief Writer function (produces messages).
 *
 * This function represents the writer thread. It continuously produces
 * messages and enqueues them into the shared queue. After enqueuing a message,
 * it signals one waiting reader to process the message.
 *
 * @param arg Argument (not used in this case).
 * @return void pointer.
 */
void* writer(void* arg) {

    while(1)
    {
        for (int i = 0; i < 5; ++i) {
            char message[20];
            // Create a message
            sprintf(message, "Message %d", i + 1);

            // Produce a message
            pthread_mutex_lock(&messageQueue.mutex);
            enqueue(&messageQueue, message);
            // Signal one waiting reader
            pthread_cond_signal(&messageQueue.cond); // Signal one waiting reader
            pthread_mutex_unlock(&messageQueue.mutex);

        }

        sleep(1); // Simulate adding 5 messages per second

    }

    pthread_exit(NULL);
}

/**
 * @brief Reader function (consumes messages).
 *
 * This function represents a reader thread. It continuously waits for
 * messages in the shared queue. Upon receiving a signal from the writer,
 * it dequeues a message, processes it, and then performs some simulated work.
 * The reader avoids busy waiting and yields to the scheduler to prevent
 * excessive CPU usage.
 *
 * @param arg Argument containing the reader ID.
 * @return void pointer.
 */
void* reader(void* arg) {
    int readerID = *(int*)arg;

    while (1) {
        pthread_mutex_lock(&messageQueue.mutex);

        // Wait for a message to be available
        while (messageQueue.front == messageQueue.rear) {
            pthread_cond_wait(&messageQueue.cond, &messageQueue.mutex);
        }

        // Consume a message
        char* message = dequeue(&messageQueue);
        printf("Reader %d consumed: %s\n", readerID, message);
        free(message);

        pthread_mutex_unlock(&messageQueue.mutex);

        // Simulate some unique work with the consumed message
        // ...
        for(int i=0; i<5000000; i++);

        // Avoid spinning on CPU by yielding to the scheduler
        sched_yield();
    }
}

/**
 * @brief Initializes the shared queue.
 *
 * This function initializes the components of the shared queue, including
 * the message array, front and rear indices, mutex for synchronization,
 * and condition variable for signaling.
 *
 * @param q Pointer to the SharedQueue structure.
 */
void initQueue(SharedQueue* q) {
    q->front = q->rear = 0;
    // Initialize mutex for synchronization
    if (pthread_mutex_init(&q->mutex, NULL) != 0) {
        perror("Error in pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
    // Initialize condition variable for signaling
    if (pthread_cond_init(&q->cond, NULL) != 0) {
        perror("Error in pthread_cond_init");
        exit(EXIT_FAILURE);
    }
}


/**
 * @brief Enqueues a message into the shared queue.
 *
 * This function adds a new message to the shared queue. It acquires the
 * mutex to ensure exclusive access during the enqueue operation.
 *
 * @param q Pointer to the SharedQueue structure.
 * @param message The message to be enqueued.
 */
void enqueue(SharedQueue* q, const char* message) {
    // Check if the queue is full
    if ((q->rear + 1) % MAX_MESSAGES == q->front) {
        fprintf(stderr, "Error: Queue is full\n");
        exit(EXIT_FAILURE);
    }
    // Copy the message into the queue
    q->messages[q->rear] = strdup(message);
    // Move rear to the next position
    q->rear = (q->rear + 1) % MAX_MESSAGES;
}

/**
 * @brief Dequeues a message from the shared queue.
 *
 * This function removes and returns a message from the shared queue.
 * It acquires the mutex to ensure exclusive access during the dequeue operation.
 *
 * @param q Pointer to the SharedQueue structure.
 * @return The dequeued message.
 */
char* dequeue(SharedQueue* q) {
    // Check if the queue is empty
    if (q->front == q->rear) {
        fprintf(stderr, "Error: Queue is empty\n");
        exit(EXIT_FAILURE);
    }
    // Retrieve the message from the queue
    char* message = q->messages[q->front];
    // Move front to the next position
    q->front = (q->front + 1) % MAX_MESSAGES;
    return message;
}
