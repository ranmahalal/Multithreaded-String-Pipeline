#include "plugin_common.h"
#include <stdlib.h>
#include <string.h>

const char* plugin_transform(const char* input) {
    if (!input) {
        return NULL;
    }
    
    size_t len = strlen(input);
    char* output = malloc(strlen(input) + 1);
    if (!output) {
        return NULL;
    }
    
    // copy characters from end to beginning
    for (int i = 0; i < len; i++) {
        output[i] = input[len - i - 1];
    }
    output[len] = '\0';
    return output;
}

const char* plugin_init(int queue_size) {
    return common_plugin_init(plugin_transform, "flipper", queue_size);
}