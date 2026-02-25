CC     ?= gcc
CFLAGS += -Wall -Wextra

SRC_DIR := src
CFLAGS  += -I$(SRC_DIR)
BIN_DIR := build

SRCS := main.c config.c ax25/addr.c pse/fib.c pse/iface.c pse/pse.c
OBJS := $(SRCS:%.c=$(BIN_DIR)/%.o)

TARGET := $(BIN_DIR)/vpsed

.PHONY: all clean

all: $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)/ax25
	mkdir -p $(BIN_DIR)/pse

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -rf $(BIN_DIR)
