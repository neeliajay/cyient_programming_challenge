# cyient_programming_challenge
Programming challenges as per Cyient Interview process

1. Implement a Hash Table  -> hash_table_test.c

   Problem Statement : implement a hash table data storage. 

   Requirements :  The data should be stored using string keys and needs to store string values
                   O(1) run time for value look up
                   Resizing of the hash table when adding/removing key-value pairs to/from the hash table

3. Shared Queue  ->  shared_queue_test.c

   Problem Statement :  implement a shared queue that is intended to be accessed by multiple readers and a single writer.

   Requirements :  The queue receives and releases string objects.
                   create 5 threads that consume the strings from the queue, and a single writer that adds the strings to the queue.
                   The writer should add 5 messages a second, and the messages should be distributed relatively evenly between the consumers

5. Implement Client-Server Data Exchange -> client_test.c , server_test.c

   Problem Statement : Implement a server and a client.

   Requirements :  The server shall support responding to a single “ping” message with a “pong” response.
                   The request-response data can be strings.
                   The protocol used can be either Layer 4 (e.g., TCP), but must be connection oriented.
                   The client, once started, must connect to the server only once,
                   and start sending “ping” message, printing the fact that the message is sent,
                   and reporting when a “pong” response is received.
                   Clients shall continue sending “ping” messages each second (whether responses are received or not).
                   The server must support multiple clients simultaneously.
                  If the server encountered an error, it should display an error and continue working.
                  If the client encounters an error, it should display an error and stop.
                  The client shall take in program parameter indicating where to connect,
                   and the server shall take in a program parameter specifying how to listen to incoming client connections.
                  •	How does the server handle multiple clients simultaneously  - using select and fd_set
                  •	Suitability of your choice of protocol for the task - TCP
                  •	Network error handling on both the client and the server - Done by graceful termination


