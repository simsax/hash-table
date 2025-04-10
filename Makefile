TARGET_EXE = ht
INCDIRS = ./src
CODEDIRS = ./src
BUILD_DIR = ./build
CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -pedantic-errors -MP -MMD
CFLAGS += -Wbad-function-cast
CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Wformat=2
CFLAGS += -Wlogical-op
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-include-dirs
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wnested-externs
CFLAGS += -Wpointer-arith
CFLAGS += -Wredundant-decls
CFLAGS += -Wsequence-point
CFLAGS += -Wshadow
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wswitch
CFLAGS += -Wundef
CFLAGS += -Wunreachable-code
CFLAGS += -Wunused-but-set-parameter
CFLAGS += -Wwrite-strings
CFLAGS += -Wnull-dereference
CFLAGS += -Wdouble-promotion
CFLAGS += -fanalyzer

# when developing, turn this on
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-unused-parameter

LDFLAGS = -lm

all: debug
debug: CFLAGS += -O0 -g3 # -fsanitize=address,undefined -fsanitize-trap
debug: LDFLAGS += -fsanitize=address
debug: $(BUILD_DIR)/$(TARGET_EXE)
release: CFLAGS += -O3 -DNDEBUG
release: $(BUILD_DIR)/$(TARGET_EXE)

run:
	$(BUILD_DIR)/$(TARGET_EXE)

CFLAGS += $(foreach D,$(INCDIRS),-I$(D))

SRCS = $(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET_EXE): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

# all targets that don't represent files go here
.PHONY: all clean run

-include $(DEPS)
