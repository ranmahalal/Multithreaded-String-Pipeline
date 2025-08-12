#include "plugin_common.h"
#include <stdlib.h>
#include <string.h>

const char* plugin_transform(const char* input) {
    if (!input) {
        return NULL;
    }
    
    char* output = malloc(strlen(input) + 1);
    if (!output) {
        return NULL;
    }
    // transform each character to uppercase
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] >= 'a' && input[i] <= 'z') {
            output[i] = input[i] - 32;
        } else {
            //character is not alphabetic
            output[i] = input[i];
        }
    }
    output[strlen(input)] = '\0';
    return output;
}

const char* plugin_init(int queue_size) {
    return common_plugin_init(plugin_transform, "uppercaser", queue_size);
}

