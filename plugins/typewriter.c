#include "plugin_common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


const char* plugin_transform(const char* input) {
    if (!input) {
        return NULL;
    }
    printf("[typewriter] ");
    //force the output
    fflush(stdout); 
    
    for (int i = 0; input[i] != '\0'; i++) {
        printf("%c", input[i]);
        fflush(stdout);
        usleep(100000);
    }
    printf("\n");
    
    char* output = malloc(strlen(input) + 1);
    if (!output) {
        return NULL;
    }
    strcpy(output, input);
    return output;
}

const char* plugin_init(int queue_size) {
    return common_plugin_init(plugin_transform, "typewriter", queue_size);
}