NAME = nemu
INC_DIR += ./include
BUILD_DIR ?= ./build
OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)

include Makefile.git

.DEFAULT_GOAL = app

# Compilation flags
CC = gcc
LD = gcc
INCLUDES  = $(addprefix -I, $(INC_DIR))
CFLAGS   += -O2 -MMD -Wall -ggdb $(INCLUDES)

# Files to be compiled
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app run submit clean
app: $(BINARY)

ARGS ?= -l $(BUILD_DIR)/nemu-log.txt

# Command to execute NEMU
NEMU_EXEC := $(BINARY) $(ARGS)

$(BINARY): $(OBJS)
	$(call git_commit, "compile")
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ -lSDL2 -lreadline

run: $(BINARY)
	$(call git_commit, "run")
	$(NEMU_EXEC)

gdb: $(BINARY)
	$(call git_commit, "gdb")
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean: 
	rm -rf $(BUILD_DIR)

.PHONY: count
count:
	@echo "Comparing line counts between current branch and 2017 branch within nemu/ directory..."
	@lines_current=$$(find . -name "*.c" -o -name "*.h" | xargs cat | grep -v "^\s*$$" | wc -l); \
	git checkout 2017 > /dev/null 2>&1; \
	lines_2017=$$(find . -name "*.c" -o -name "*.h" | xargs cat | grep -v "^\s*$$" | wc -l); \
	git checkout - > /dev/null 2>&1; \
	echo "Current branch line count (excluding blank lines): $$lines_current"; \
	echo "2017 branch line count (excluding blank lines): $$lines_2017"; \
	diff=$$((lines_current-lines_2017)); \
	echo "Difference (current - 2017): $$diff"



