#include "plugin_common.h"
#include <stdlib.h>
#include <string.h>

const char* plugin_transform(const char* input) {
    if (!input) {
        return NULL;
    }
    size_t len = strlen(input);
    
    // In case of an empty string or single character
    if (len <= 1) {
        char* output = malloc(len + 1);
        if (!output) {
            return NULL;
        }
        strcpy(output, input);
        return output;
    }
    
    char* output = malloc(len + 1);
    if (!output) {
        return NULL; 
    }
    output[0] = input[len - 1];  // last character moves to front

    for (int i = 1; i < len; i++) {
        output[i] = input[i - 1];  // move each character right 
    }
        output[len] = '\0';
    
    return output;
}

const char* plugin_init(int queue_size) {
    return common_plugin_init(plugin_transform, "rotator", queue_size);
}