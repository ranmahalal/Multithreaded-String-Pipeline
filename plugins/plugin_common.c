#include "plugin_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//global context for the plugin
static plugin_context_t* g_plugin_context = NULL;

void* plugin_consumer_thread(void* arg) {
    plugin_context_t* context = (plugin_context_t*)arg;
    
    if(!context || !context->queue) {
        return NULL;
    }

    while(!context->finished) {
        char* input = consumer_producer_get(context->queue);
        
        if(!input) {
            continue; 
        }
        
        // <END> string - we finished
        if(strcmp(input, "<END>") == 0) {
            if(context->next_place_work) {
                context->next_place_work(input);
            }
            free(input);
            context->finished = 1;
            consumer_producer_signal_finished(context->queue);
            break;
        }
        
        //process the input string
        char* processed = (char*)context->process_function(input);
        free(input);
        
        if(processed && context->next_place_work) {
            context->next_place_work(processed);
        }
        
        if(processed) {
            free(processed);
        }
    }
    
    return NULL; 
}
/*
void log_error(plugin_context_t* context, const char* message) {
    if(!context || !message) {
        return;
    }
    fprintf(stderr, "[ERROR][%s] - %s\n", context->name, message);
}

void log_info(plugin_context_t* context, const char* message) {
    if(!context || !message) {
        return;
    }
    printf("[INFO][%s] - %s\n", context->name, message);
} */

const char* plugin_get_name(void) {
    if(g_plugin_context == NULL) {
        return NULL;
    }
    return g_plugin_context->name;
}

const char* common_plugin_init(const char* (*process_function)(const char*),
const char* name, int queue_size){
    if(g_plugin_context != NULL) {
        return "plugin is already initialized";
    }
    if(!process_function) {
        return "process function can not be NULL";
    }
    if(!name) {
        return "plugin name can not be NULL";
    }
    if(queue_size <= 0) {
        return "queue size must be positive";
    }
    
    g_plugin_context = malloc(sizeof(plugin_context_t));
    if(!g_plugin_context) {
        return "allocation failed for plugin context";
    }
    // initialize fields
    g_plugin_context->name = name;
    g_plugin_context->process_function = process_function;
    g_plugin_context->next_place_work = NULL;
    g_plugin_context->initialized = 0;
    g_plugin_context->finished = 0;
    
    //allocate memory for the queue
    g_plugin_context->queue = malloc(sizeof(consumer_producer_t));
    if(!g_plugin_context->queue) {
        free(g_plugin_context);
        g_plugin_context = NULL;
        return "allocation failed for queue";
    }
    
    // init the consumer producer queue
    const char* queue_init_result = consumer_producer_init(g_plugin_context->queue, queue_size);
    if(queue_init_result != NULL) {
        free(g_plugin_context->queue);
        free(g_plugin_context);
        g_plugin_context = NULL;
        return queue_init_result;  // return the queue init error from consumer_producer_init
    }
    
    // create the consumer thread
    if(pthread_create(&g_plugin_context->consumer_thread, NULL, 
                      plugin_consumer_thread, g_plugin_context) != 0) {
        consumer_producer_destroy(g_plugin_context->queue);
        free(g_plugin_context->queue);
        free(g_plugin_context);
        g_plugin_context = NULL;
        return "fail to create consumer thread";
    }
    
    g_plugin_context->initialized = 1;
    
    return NULL;
}

const char* plugin_place_work(const char* str) {
    if(g_plugin_context == NULL) {
        return "plugin not initialized";
    }
    if(!g_plugin_context->initialized) {
        return "plugin not initialized";
    }
    if(!str) {
        return "string is can not be NULL";
    }
    if(g_plugin_context->finished) {
        return "plugin is finished";
    }
    // put the work into the queue using consumer_producer_put
    const char* put_result = consumer_producer_put(g_plugin_context->queue, str);
    
    return put_result;
}

void plugin_attach(const char* (*next_place_work)(const char*)){
    if(g_plugin_context == NULL) {
        return;
    }
    if(!g_plugin_context->initialized) {
        return;
    }   
    g_plugin_context->next_place_work = next_place_work;
}

const char* plugin_wait_finished(void){
    if(g_plugin_context == NULL) {
        return "plugin not initialized";
    }
    if(!g_plugin_context->initialized) {
        return "plugin not properly initialized";
    }
    
    if(consumer_producer_wait_finished(g_plugin_context->queue) != 0) {
        return "failed to wait for the queue to finish";
    }
    
    return NULL;
}

const char* plugin_fini(void) {
    if(g_plugin_context == NULL) {
        return NULL;
    }
    
    // terminatee consumer thread
    if(g_plugin_context->initialized) {
        pthread_join(g_plugin_context->consumer_thread, NULL);
    }
    
    if(g_plugin_context->queue) {
        consumer_producer_destroy(g_plugin_context->queue);
        free(g_plugin_context->queue);
        g_plugin_context->queue = NULL;
    }
    
    free(g_plugin_context);
    g_plugin_context = NULL;
    
    return NULL; 
}






