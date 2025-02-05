# Compiler
CC = g++

# Compiler flags
ROUTING_KIT_ARGS = -IRoutingKit/include -LRoutingKit/lib -Wl,-rpath,$(CURDIR)/RoutingKit/lib

CFLAGS = $(ROUTING_KIT_ARGS) -w -march=native -std=c++23 -O3 -Wall
INCLUDE_LIBRARIES = -lgsl -lroutingkit

# Dependency flags (generate dependency files during compilation)
DEPFLAGS = -MMD -MP

# Directories
SRC_DIRS = src
OBJ_DIR = obj
BIN_DIR = bin
DOC_DIR = docs
DATA_DIR = data

# Executables names without prefix/suffix (just the target name)
EXEC_NAMES = test_dimension find_best_clustering_coefficients find_matching_dimensions run_optimal_vs_dimension

.PHONY: all directories clean $(EXEC_NAMES) docs

# Generate full path to executables
EXECUTABLES = $(addprefix $(BIN_DIR)/,$(EXEC_NAMES))

# List of .cpp files
CPP_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))

# Object files corresponding to the .cpp files
BASE_CPP_OBJ_FILES := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(CPP_FILES)))

# Combine object files
BASE_OBJ_FILES = $(BASE_CPP_OBJ_FILES)

# Dependency files for each object file
DEP_FILES = $(wildcard $(OBJ_DIR)/*.d)

# Default target
all: directories $(EXECUTABLES)

# Create necessary directories
directories:
	mkdir -p $(BIN_DIR) $(OBJ_DIR) $(DOC_DIR) $(DATA_DIR)

# Compile source files into object files
$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@ $(INCLUDE_LIBRARIES)

# Include dependency files if they exist
-include $(DEP_FILES)

# Rule to link each executable
define MAKE_EXECUTABLE
$(BIN_DIR)/$(1): $(BASE_OBJ_FILES) $(OBJ_DIR)/$(1).o
	$(CC) $(CFLAGS) -o $$@ $$^ $(INCLUDE_LIBRARIES)
endef

# Generate rules for executables
$(foreach exec,$(EXEC_NAMES),$(eval $(call MAKE_EXECUTABLE,$(exec))))

$(EXEC_NAMES): %: $(BIN_DIR)/%

# Generate documentation
docs:
	doxygen Doxyfile

# Adjust pattern rules for finding source files not just in SRC_DIRS
vpath %.cpp $(SRC_DIRS)

# Clean
clean:
	rm -f $(BASE_OBJ_FILES) $(DEP_FILES) $(EXECUTABLES)
	rm -rf $(DOC_DIR)/*