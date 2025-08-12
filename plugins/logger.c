#include "plugin_common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


const char* plugin_transform(const char* input) {
    if (!input) {
        return NULL;
    }
    printf("[logger] %s\n", input);
    
    char* output = malloc(strlen(input) + 1);
    if (!output) {
        return NULL;
    }
    strcpy(output, input);
    return output;
}

const char* plugin_init(int queue_size) {
    return common_plugin_init(plugin_transform, "logger", queue_size);
}