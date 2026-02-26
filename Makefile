REQUIRED_LIBS = liburcu
PKG_CONFIG ?= pkg-config

CC     ?= gcc
CFLAGS += -Wall -Wextra

SRC_DIR := src
CFLAGS  += -I$(SRC_DIR) -DRCU_MB
LDFLAGS += -lurcu-mb -lpthread
BIN_DIR := build

SRCS := main.c config.c ax25/addr.c pse/fib.c pse/iface.c pse/pse.c
OBJS := $(SRCS:%.c=$(BIN_DIR)/%.o)

TARGET := $(BIN_DIR)/vpsed

.PHONY: all clean

all: check-libs $(TARGET)

check-libs:
	@$(shell $(PKG_CONFIG) --exists $(REQUIRED_LIBS) || \
	( echo "Missing one of the required libraries: $(REQUIRED_LIBS)"; exit 1 ))

$(BIN_DIR):
	mkdir -p $(BIN_DIR)/ax25
	mkdir -p $(BIN_DIR)/pse

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)
