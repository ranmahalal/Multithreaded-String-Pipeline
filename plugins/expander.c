#include "plugin_common.h"
#include <stdlib.h>
#include <string.h>

const char* plugin_transform(const char* input) {
    if (!input) {
        return NULL;
    }   
    size_t input_len = strlen(input);
    
    // In case of an empty string
    if (input_len == 0) {
        char* output = malloc(1);
        if (!output) {
            return NULL;
        }
        output[0] = '\0';
        return output;
    }
    
    size_t output_len = input_len + (input_len - 1);
    char* output = malloc(output_len + 1);
    if (!output) {
        return NULL;
    }
    
    int index = 0;
    for (int i = 0; i < input_len; i++) {
        output[index] = input[i];
        index++;
        
        // if its not the last character, add a space
        if (i < input_len - 1) {
            output[index] = ' ';
            index++;
        }
    }

    output[output_len] = '\0';
    return output;
}

const char* plugin_init(int queue_size) {
    return common_plugin_init(plugin_transform, "expander", queue_size);
}