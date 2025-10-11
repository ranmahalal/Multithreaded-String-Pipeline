set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[BUILD]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create output directory if it doesn't exist
print_status "creating the output directory..."
mkdir -p output

# Build main application
print_status "Building the main application..."
gcc-13 -o output/analyzer main.c -ldl -lpthread || {
    print_error "Failed to build main application"
    exit 1
}

# Building plugins
print_status "Building plugins..."
for plugin_name in logger uppercaser rotator flipper expander typewriter; do
    print_status "Building plugin: $plugin_name"
    gcc-13 -fPIC -shared -o output/${plugin_name}.so \
        plugins/${plugin_name}.c \
        plugins/plugin_common.c \
        plugins/sync/monitor.c \
        plugins/sync/consumer_producer.c \
        -ldl -lpthread || {
        print_error "Failed to build $plugin_name"
        exit 1
    }
done

print_status "Build completed successfully!"
