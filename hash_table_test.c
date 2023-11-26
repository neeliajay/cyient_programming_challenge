/**
 * @file hash_table_test.c
 * @brief Hash Table for Data storage with string keys and values.
 *        Player-Country is considered as a Key-Value pair.
 *        O(1) run time for value look up, Hash Function, and Resizing of hash table.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @brief Initial capacity of the hashtable. */
#define INITIAL_CAPACITY 10
/** @brief Load factor used while resizing. */
#define LOAD_FACTOR_THRESHOLD 0.75

/** @brief Structure representing a key-value pair. */
typedef struct KeyValue {
    char* key;  /**< Key of the pair. */
    char* value; /**< Value associated with the key. */
    struct KeyValue* next; /**< Pointer to the next key-value pair in the same container. */
} KeyValue;

/** @brief Structure representing the Hash Table. */
typedef struct {
    KeyValue** table; /**< Double pointer to facilitate linked list based key-value pairs when hash collisions occur. */
    int size; /**< Number of key-value pairs in the hashtable. */
    int capacity; /**< Current capacity of the hashtable. */
} HashTable;

/*  FUNCTION DECLARATIONS   */
void initHashTable(HashTable* ht, int capacity);
void insertKeyValPair(HashTable* ht, const char* key, const char* value);
void removeKeyValPair(HashTable* ht, const char* key);
const char* lookup_hashTable(const HashTable* ht, const char* key);
void freeHashTable(HashTable* ht);
void resizeHashTable(HashTable* ht);
KeyValue* createKeyValPair(const char* key, const char* value);
unsigned int hashFunction(const char* key, int capacity);

/*  FUNCTION DEFINITIONS */

/** 
 * @brief Main function demonstrating the usage of the hash table.
 * @details Creates a hash table, inserts key-value pairs, looks up values, removes a key-value pair,
 *          and frees allocated memory.
 * @return 0 on successful execution.
 */
int main(void) {
    //Create and initialize a Hash Table with a minimal capacity
    HashTable myHashTable;
    initHashTable(&myHashTable, INITIAL_CAPACITY);

    //Insert Key-value pairs
    //Taking Key-Value as Player-Country
    //During Insert - resizing of HashTable is handled
    insertKeyValPair(&myHashTable, "maradona", "Argentina");
    insertKeyValPair(&myHashTable, "pele", "Brazil");
    insertKeyValPair(&myHashTable, "zidane", "France");

    //look up values
    const char* zidane_cntry = lookup_hashTable(&myHashTable, "zidane");
    const char* pele_cntry = lookup_hashTable(&myHashTable, "pele");
    const char* maradona_cntry = lookup_hashTable(&myHashTable, "maradona");

    //Display results
    printf("Zidane Country : %s\n", zidane_cntry);
    printf("Pele Country: %s\n", pele_cntry);
    printf("Maradona Country: %s\n", maradona_cntry);


    //remove a key-value pair
    //During remove - resizing of HashTable is handled
    removeKeyValPair(&myHashTable, "maradona");
    const char* removedmaradona = lookup_hashTable(&myHashTable, "maradona");
    if (removedmaradona == NULL) {
        printf("Maradona Country Entry was successfully removed.\n");
    }

    //Free allocated memory
    freeHashTable(&myHashTable);

    return 0;
}

/** 
 * @brief Initializes a hash table with a given capacity.
 * @param ht Pointer to the hash table to be initialized.
 * @param capacity Initial capacity of the hash table.
 */
void initHashTable(HashTable* ht, int capacity) {
    ht->table = (KeyValue**)malloc(sizeof(KeyValue*) * capacity);
    if (ht->table == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    ht->size = 0;
    ht->capacity = capacity;

    // Initialize each container with NULL
    for (int i = 0; i < capacity; ++i) {
        ht->table[i] = NULL;
    }
}

/** 
 * @brief Inserts a key-value pair into the hash table, handles resizing if necessary.
 * @param ht Pointer to the hash table.
 * @param key Key of the pair.
 * @param value Value associated with the key.
 */
void insertKeyValPair(HashTable* ht, const char* key, const char* value) {
    // Check if resizing is needed
    if ((double)ht->size / ht->capacity > LOAD_FACTOR_THRESHOLD) {
        resizeHashTable(ht);
    }

    // Calculate hash index
    unsigned int index = hashFunction(key, ht->capacity);

    // Create a new key-value pair
    KeyValue* newPair = createKeyValPair(key, value);

    // Add the new pair to the beginning of the linked list at the hash index
    newPair->next = ht->table[index];
    ht->table[index] = newPair;

    // Increment the size of the hashtable
    ht->size++;
}

/** 
 * @brief Removes a key-value pair from the hash table, handles resizing if necessary.
 * @param ht Pointer to the hash table.
 * @param key Key of the pair to be removed.
 */
void removeKeyValPair(HashTable* ht, const char* key) {
    // Calculate hash index
    unsigned int index = hashFunction(key, ht->capacity);
    KeyValue* current = ht->table[index];
    KeyValue* prev = NULL;

    // Traverse the linked list in the container
    while (current != NULL) {
        // Check if the current key matches the target key
        if (strcmp(current->key, key) == 0) {
            // Remove the pair from the linked list
            if (prev == NULL) {
                // If the pair is at the beginning of the list
                ht->table[index] = current->next;
            } else {
                prev->next = current->next;
            }

            // Free memory for the removed pair
            free(current->key);
            free(current->value);
            free(current);

            // Decrement the size of the hashtable
            ht->size--;

            // Check if resizing is needed
            if ((double)ht->size / ht->capacity < LOAD_FACTOR_THRESHOLD) {
                resizeHashTable(ht);
            }
            
            return;
        }

        prev = current;
        current = current->next;
    }
}

/** 
 * @brief Looks up the value associated with a given key in the hash table.
 * @param ht Pointer to the hash table.
 * @param key Key to look up.
 * @return Value associated with the key, or NULL if not found.
 */
const char* lookup_hashTable(const HashTable* ht, const char* key) {
    // Calculate hash index
    unsigned int index = hashFunction(key, ht->capacity);
    KeyValue* current = ht->table[index];

    // Traverse the linked list in the container
    while (current != NULL) {
        // Check if the current key matches the target key
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }

    // Key not found
    return NULL;
}

/** 
 * @brief Frees the memory allocated for the hash table and its key-value pairs.
 * @param ht Pointer to the hash table to be freed.
 */
void freeHashTable(HashTable* ht) {
    // Iterate through each container in the hashtable
    for (int i = 0; i < ht->capacity; ++i) {
        KeyValue* current = ht->table[i];
        // Traverse the linked list in the current container
        while (current != NULL) {
            KeyValue* next = current->next;
            // Free memory for the key, value, and the key-value pair
            free(current->key);
            free(current->value);
            free(current);
            current = next;
        }
    }
    // Free the array of containers
    free(ht->table);
}

/** 
 * @brief Creates a new key-value pair with the given key and value.
 * @param key Key of the pair.
 * @param value Value associated with the key.
 * @return Pointer to the newly created key-value pair.
 */
KeyValue* createKeyValPair(const char* key, const char* value) {
    // Allocate memory for a new key-value pair
    KeyValue* newPair = (KeyValue*)malloc(sizeof(KeyValue));
    if (newPair == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    // Duplicate the key and value using strdup to manage memory
    newPair->key = (char *)strdup(key);
    newPair->value = (char *)strdup(value);
    // Initialize the next pointer to NULL
    newPair->next = NULL;

    return newPair;
}

/** 
 * @brief Resizes the hash table by doubling its size and rehashing existing key-value pairs.
 * @param ht Pointer to the hash table to be resized.
 */
void resizeHashTable(HashTable* ht) {
    // Calculate the new capacity (double the current capacity)
    int newCapacity = ht->capacity * 2;
    // Allocate memory for the new array of containers
    KeyValue** newTable = (KeyValue**)malloc(sizeof(KeyValue*) * newCapacity);
    if (newTable == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    // Initialize each container in the new array with NULL
    for (int i = 0; i < newCapacity; ++i) {
        newTable[i] = NULL;
    }

    // Iterate through each container in the current hashtable
    for (int i = 0; i < ht->capacity; ++i) {
        KeyValue* current = ht->table[i];
        // Traverse the linked list in the current container
        while (current != NULL) {
            KeyValue* next = current->next;
            // Calculate the new hash index for the key in the new hashtable
            unsigned int newIndex = hashFunction(current->key, newCapacity);
            // Add the current key-value pair to the beginning of the linked list in the new container
            current->next = newTable[newIndex];
            newTable[newIndex] = current;
            current = next;
        }
    }

    // Free the memory allocated for the old array of containers
    free(ht->table);
    // Update the hash table with the new array of containers and capacity
    ht->table = newTable;
    ht->capacity = newCapacity;
}

/** 
 * @brief Simple hash function.
 * @details Calculates the hash value as the sum of ASCII values of characters in the key, 
 *          rounded to the hashtable capacity.
 * @param key Key for which the hash value is calculated.
 * @param capacity Current capacity of the hash table.
 * @return Calculated hash value.
 */
unsigned int hashFunction(const char* key, int capacity) {
    unsigned int hash = 0;

    // Calculate the hash value by adding the ASCII values of characters in the key
    for (int i = 0; key[i] != '\0'; ++i) {
        hash += key[i];
    }

    // Round the calculated hash value to fit within the hashtable capacity
    return hash % capacity;
}




