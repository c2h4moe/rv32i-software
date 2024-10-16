CC := riscv64-unknown-elf-gcc
LD := riscv64-unknown-elf-ld
AR := riscv64-unknown-elf-ar
OBJCOPY := riscv64-unknown-elf-objcopy
CFLAGS := -O1 -nostdlib -ffreestanding -fno-builtin -march=rv32i -mabi=ilp32 -I./runtime/
LDFLAGS :=  -m elf32lriscv -T linker.ld
APP_DIR := apps
OBJ_DIR := build
RUNTIME_DIR := runtime
RUNTIMES := $(wildcard $(RUNTIME_DIR)/*.c)
RUNTIME_OBJS := $(patsubst %.c, %.o, $(RUNTIMES))
APPS := $(wildcard $(APP_DIR)/*.c)
OBJS := $(patsubst $(APP_DIR)/%.c, $(OBJ_DIR)/%.o, $(APPS))
PROGRAMS := $(patsubst $(APP_DIR)/%.c, $(OBJ_DIR)/%, $(APPS))
TARGETS := $(patsubst $(APP_DIR)/%.c, %, $(APPS))
TARGET_DIR := target
LDLIB := -L./runtime/ -lrv32iemu

mode ?= sim

ifeq ($(mode),sim)
CFLAGS += -D SIM_MODE
endif

$(RUNTIME_OBJS) : %.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(RUNTIME_DIR)/librv32iemu.a: $(RUNTIME_OBJS)
	$(AR) -rc $@ $^

runtime/_start.o : runtime/_start.S
	$(CC) $(CFLAGS) -o $@ -c $^

$(OBJS): $(OBJ_DIR)/%.o: $(APP_DIR)/%.c $(RUNTIME_DIR)/librv32iemu.a
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(PROGRAMS): %: %.o runtime/_start.o
	mkdir -p $(TARGET_DIR)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIB)
	$(OBJCOPY) --strip-all \
	--dump-section .data=$(patsubst $(OBJ_DIR)/%,$(TARGET_DIR)/%, $@)_data.hex \
	--dump-section .text=$(patsubst $(OBJ_DIR)/%,$(TARGET_DIR)/%, $@)_inst.hex $@
	od -v -An -w4 -tx4 $(patsubst $(OBJ_DIR)/%,$(TARGET_DIR)/%, $@)_inst.hex > \
	$(patsubst $(OBJ_DIR)/%,$(TARGET_DIR)/%, $@)_rom.hex
	od -v -An -w4 -tx4 $(patsubst $(OBJ_DIR)/%,$(TARGET_DIR)/%, $@)_data.hex > \
	$(patsubst $(OBJ_DIR)/%,$(TARGET_DIR)/%, $@)_ram.hex
	rm $(patsubst $(OBJ_DIR)/%,$(TARGET_DIR)/%, $@)_inst.hex
	rm $(patsubst $(OBJ_DIR)/%,$(TARGET_DIR)/%, $@)_data.hex

$(TARGETS): %: $(OBJ_DIR)/%

all: $(PROGRAMS)

.PHONY: all runtime