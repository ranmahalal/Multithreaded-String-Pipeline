# Multithreaded-StringPipeline

A flexible, plugin-based string processing system in C demonstrating multithreading, dynamic linking, and producer-consumer patterns. Input strings are processed through a configurable pipeline of plugins, where each plugin performs a transformation (uppercase conversion, character rotation, string reversal, etc.) in its own thread. Communication between plugins uses bounded, thread-safe queues.

---

## Features

- **Multithreaded Architecture**: Each plugin runs in its own thread for concurrent processing.
- **Dynamic Plugin Loading**: Plugins are loaded at runtime as shared objects (`.so` files).
- **Thread-Safe Communication**: Bounded producer-consumer queues with proper synchronization.
- **Configurable Pipeline**: Specify processing order and queue sizes via command line.
- **Graceful Shutdown**: Clean termination when input is received.

---

## Available Plugins

- `logger`: Logs all strings that pass through.
- `typewriter`: Simulates a typewriter effect with delays.
- `uppercaser`: Converts strings to uppercase.
- `rotator`: Rotates characters one position right.
- `flipper`: Reverses the order of characters.
- `expander`: Inserts spaces between each character.

---

## How It Works

1. The program reads input lines from `stdin` and pushes them onto the first queue.
2. Each plugin is dynamically loaded and runs a worker thread that:
   - Pulls strings from its input queue
   - Transforms the string
   - Pushes the result onto the next queue
3. Queues are bounded and synchronized using mutexes and condition variables to prevent data races and provide back-pressure.
4. On shutdown, the host signals all workers and drains queues for a clean exit.

---