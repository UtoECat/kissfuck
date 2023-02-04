# Output file
TARGET_EXEC ?= kissfuck

#directories
BUILD_DIR ?= ./build
BINARY_DIR ?= ./
SRC_DIRS ?= ./
INC_DIRS ?= ./

#what types of files we should to compile?
SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
INC_FLAGS := -I.
#forming compiler flags
UNIFLAGS ?= -Os -Wall -Wextra -flto -g
LINFLAGS ?= 
CMPFLAGS ?= $(INC_FLAGS) -MMD -MP $(UNIFLAGS) -std=gnu11

#libraries, that we need to link
LDLIBS := -lm
CMP    := gcc

$(BINARY_DIR)/$(TARGET_EXEC): $(OBJS)
	$(MKDIR_P) $(dir $@)
	$(CMP) $(OBJS) -o $@ $(LDLIBS) $(UNIFLAGS) $(LINFLAGS)

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CMP) $(CMPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
