#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

// function pointer types for plugin interface
typedef const char* (*plugin_init_func_t)(int);
typedef const char* (*plugin_fini_func_t)(void);
typedef const char* (*plugin_place_work_func_t)(const char*);
typedef void (*plugin_attach_func_t)(const char* (*)(const char*));
typedef const char* (*plugin_wait_finished_func_t)(void);
typedef const char* (*plugin_get_name_func_t)(void);

// Structure to hold plugin information
typedef struct {
    plugin_init_func_t init;
    plugin_fini_func_t fini;
    plugin_place_work_func_t place_work;
    plugin_attach_func_t attach;
    plugin_wait_finished_func_t wait_finished;
    plugin_get_name_func_t get_name;
    char* name;   
     void* handle
} plugin_handle_t;

// function to print usage help
void print_usage(void) {
    printf("Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>\n");
    printf("\nArguments:\n");
    printf("  queue_size    Maximum number of items in each plugin's queue\n");
    printf("  plugin1..N    Names of plugins to load (without .so extension)\n");
    printf("\nAvailable plugins:\n");
    printf("  logger        - Logs all strings that pass through\n");
    printf("  typewriter    - Simulates typewriter effect with delays\n");
    printf("  uppercaser    - Converts strings to uppercase\n");
    printf("  rotator       - Move every character to the right. Last character moves to the beginning.\n");
    printf("  flipper       - Reverses the order of characters\n");
    printf("  expander      - Expands each character with spaces\n");
    printf("\nExample:\n");
    printf("  ./analyzer 20 uppercaser rotator logger\n");
    printf("  echo 'hello' | ./analyzer 20 uppercaser rotator logger\n");
    printf("  echo '<END>' | ./analyzer 20 uppercaser rotator logger\n");
}

int main(int argc, char* argv[]) {
    //Step 1 - parse command line arguments
    if (argc < 3) {
        fprintf(stderr, "Must give at least 3 arguments\n");
        print_usage();
        return 1;
    }
    
    int queue_size = atoi(argv[1]);
    if (queue_size <= 0) {
        fprintf(stderr, "Queue size must be a positive integer\n");
        print_usage();
        return 1;
    }
    
    int num_plugins = argc - 2;
    char** plugin_names = &argv[2];
 
    
    // Step 2: load Plugin Shared Objects
    plugin_handle_t* plugins = malloc(num_plugins * sizeof(plugin_handle_t));
    if (!plugins) {
        fprintf(stderr, "memory allocation failed for plugins array\n");
        print_usage();
        return 1;
    }
    
    for (int i = 0; i < num_plugins; i++) {
        // construct filename by appending .so
        char filename[256]; 
        //Load using dlopen
        sprintf(filename, "./output/%s.so", plugin_names[i]);
        plugins[i].handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
        // If that fails, try running from output dir)
        if (!plugins[i].handle) {
            snprintf(filename, sizeof(filename), "./%s.so", plugin_names[i]);
            plugins[i].handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
    }
        if (!plugins[i].handle) {
            fprintf(stderr, "error loading plugin '%s': %s\n", filename, dlerror());
            print_usage();
            // cleanup for loaded plugins
            for (int j = 0; j < i; j++) {
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
        
        // reslove the exported functions using dlsym
        plugins[i].init = (plugin_init_func_t)dlsym(plugins[i].handle, "plugin_init");
        if (!plugins[i].init) {
            fprintf(stderr, "Cannot find 'plugin_init' in '%s': %s\n", filename, dlerror());
            print_usage();
            for (int j = 0; j <= i; j++) {
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
        
        plugins[i].fini = (plugin_fini_func_t)dlsym(plugins[i].handle, "plugin_fini");
        if (!plugins[i].fini) {
            fprintf(stderr, "Cannot find 'plugin_fini' in '%s': %s\n", filename, dlerror());
            print_usage;
            for (int j = 0; j <= i; j++) {
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
        
        plugins[i].place_work = (plugin_place_work_func_t)dlsym(plugins[i].handle, "plugin_place_work");
        if (!plugins[i].place_work) {
            fprintf(stderr, "Cannot find 'plugin_place_work' in '%s': %s\n", filename, dlerror());
            print_usage;
            for (int j = 0; j <= i; j++) {
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
        
        plugins[i].attach = (plugin_attach_func_t)dlsym(plugins[i].handle, "plugin_attach");
        if (!plugins[i].attach) {
            fprintf(stderr, "Cannot find 'plugin_attach' in '%s': %s\n", filename, dlerror());
            print_usage;
            for (int j = 0; j <= i; j++) {
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
        
        plugins[i].wait_finished = (plugin_wait_finished_func_t)dlsym(plugins[i].handle, "plugin_wait_finished");
        if (!plugins[i].wait_finished) {
            fprintf(stderr, "Cannot find 'plugin_wait_finished' in '%s': %s\n", filename, dlerror());
            print_usage;
            for (int j = 0; j <= i; j++) {
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
        
        plugins[i].get_name = (plugin_get_name_func_t)dlsym(plugins[i].handle, "plugin_get_name");
        if (!plugins[i].get_name) {
            fprintf(stderr, "Cannot find 'plugin_get_name' in '%s': %s\n", filename, dlerror());
            print_usage();
            for (int j = 0; j <= i; j++) {
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
        
        plugins[i].name = plugin_names[i];
    }
    
    // Step 3: Initialize Plugins
    for (int i = 0; i < num_plugins; i++) {
        const char* error = plugins[i].init(queue_size);
        if (error) {
            fprintf(stderr, "Error initializing plugin '%s': %s\n", plugins[i].name, error);            
            for (int j = 0; j < i; j++) {
                plugins[j].place_work("<END>");
            }
            
            for (int j = 0; j < i; j++) {
                plugins[j].wait_finished();
            }
            
            for (int j = 0; j < i; j++) {
                plugins[j].fini();
            }
            
            for (int j = 0; j <= i; j++) {
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 2;
        }

    }
    
    // Step 4: Attach Plugins Together
    for (int i = 0; i < num_plugins - 1; i++) {
        plugins[i].attach(plugins[i + 1].place_work);
    }
    
    // Step 5: Read Input from STDIN
    char line[1026]; //make sure 1026 is correct
    while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        // Send to first plugin
        const char* error = plugins[0].place_work(line);
        if (error) {
            fprintf(stderr, "error placing work in first plugin: %s\n", error);
            break;
        }   
        if (strcmp(line, "<END>") == 0) {
            break;
        }
    }
    
    // Step 6: Wait for Plugins to Finish
    for (int i = 0; i < num_plugins; i++) {
        const char* error = plugins[i].wait_finished();
        if (error) {
            fprintf(stderr, "error waiting for plugin '%s': %s\n", plugins[i].name, error);
        }
    }
    
    // Step 7: Cleanup
    for (int i = 0; i < num_plugins; i++) {
        const char* error = plugins[i].fini();
        if (error) {
            fprintf(stderr, "error cleaning up plugin '%s': %s\n", plugins[i].name, error);
        }
        dlclose(plugins[i].handle);
    }
    
    free(plugins);
    
    // Step 8: Finalize
    printf("Pipeline shutdown complete\n");
    return 0;
}