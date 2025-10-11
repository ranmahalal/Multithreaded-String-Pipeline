#include "consumer_producer.h"
#include <stdlib.h>
#include <string.h>

const char* consumer_producer_init(consumer_producer_t* queue, int capacity) {
    if(!queue || capacity <= 0) {
        return "queue or capacity is not valid";
    }
    if(pthread_mutex_init(&queue->mutex, NULL) != 0) {
        return "failed to init mutex";
    }
    // memory for the items array
    queue->items = malloc(capacity * sizeof(char*));
    if(!queue->items) {
        pthread_mutex_destroy(&queue->mutex);
        return "allocation failed";
    }
    for(int i = 0; i < capacity; i++) {
    queue->items[i] = NULL;
    }
    //setting queue fields
    queue->count = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->capacity = capacity;

    // Initialize monitors
    if(monitor_init(&queue->not_full_monitor) != 0) {
        free(queue->items);
        pthread_mutex_destroy(&queue->mutex);
        return "fail to init not_full monitor";
    }
    
    if(monitor_init(&queue->not_empty_monitor) != 0) {
        monitor_destroy(&queue->not_full_monitor);
        free(queue->items);
        pthread_mutex_destroy(&queue->mutex);
        return "fail to init not_empty monitor";
    }
    
    if(monitor_init(&queue->finished_monitor) != 0) {
        monitor_destroy(&queue->not_full_monitor);
        monitor_destroy(&queue->not_empty_monitor);
        free(queue->items);
        pthread_mutex_destroy(&queue->mutex);
        return "fail to init finished monitor";
    }
    //queue is not full initially
    monitor_signal(&queue->not_full_monitor);
    
    return NULL;
}

void consumer_producer_destroy(consumer_producer_t* queue){
    if(!queue){
        return;
    }
    if(!queue->items) {
        return;
    }
    //free strings in the queue
    for(int i = 0; i < (queue->capacity); i++) {
        free(queue->items[i]);
    }

    free(queue->items);
    monitor_destroy(&queue->not_full_monitor);
    monitor_destroy(&queue->not_empty_monitor);
    monitor_destroy(&queue->finished_monitor);
    pthread_mutex_destroy(&queue->mutex);
}


const char* consumer_producer_put(consumer_producer_t* queue, const char*item) {
    if(!queue){
        return "queue is not valid";
    }
    if(!item){
        return "item is not valid";
    }
    char* itemToAdd = malloc(strlen(item) + 1);
    if(!itemToAdd) {
        return "memory allocation failed";
    }
    strcpy(itemToAdd, item);
    // wait for queue to not be full
    if(monitor_wait(&queue->not_full_monitor) != 0) {
        free(itemToAdd);
        return "not_full_monitor failed";
    }
    
    pthread_mutex_lock(&queue->mutex);
    queue->items[queue->tail] = itemToAdd;
    queue->tail = (queue->tail + 1) % (queue->capacity);
    queue->count++;

    if(queue->count == 1){
        //in this case we added the first item
        monitor_signal(&queue->not_empty_monitor);
    }

    if(queue->count == queue->capacity){
        //mark the queue as full
        monitor_reset(&queue->not_full_monitor);
    }

    pthread_mutex_unlock(&queue->mutex);

    return NULL;
}

char* consumer_producer_get(consumer_producer_t* queue){
    if(!queue){
        return NULL;
    }
    // wait for queue to not be empty
    if(monitor_wait(&queue->not_empty_monitor) != 0){
        return NULL;
    }
    pthread_mutex_lock(&queue->mutex);
    char* item_to_remove = queue->items[queue->head];
    queue->items[queue->head] = NULL;
    queue->head = (queue->head + 1) % (queue->capacity);
    queue->count--;
 
    if(queue->count == 0){
        monitor_reset(&queue->not_empty_monitor);
    }
    if(queue->count == (queue->capacity - 1)){
        monitor_signal(&queue->not_full_monitor);
    }
    pthread_mutex_unlock(&queue->mutex);
    return item_to_remove; 
}

void consumer_producer_signal_finished(consumer_producer_t* queue){
    if(!queue){
        return;
    }
    monitor_signal(&queue->finished_monitor);
}

int consumer_producer_wait_finished(consumer_producer_t* queue){
    if(!queue){
        return -1;
    }
    return monitor_wait(&queue->finished_monitor);
}