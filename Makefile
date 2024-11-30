CC := riscv64-unknown-elf-gcc
CXX := riscv64-unknown-elf-g++
LD := riscv64-unknown-elf-ld
AR := riscv64-unknown-elf-ar
OBJCOPY := riscv64-unknown-elf-objcopy

CFLAGS := -O2 -nostdlib -ffreestanding -fno-builtin -march=rv32i -mabi=ilp32 -I./runtime/
LDFLAGS :=  -m elf32lriscv -T linker.ld
APP_DIR := apps
OBJ_DIR := build
RUNTIME_DIR := runtime

RUNTIMES := $(wildcard $(RUNTIME_DIR)/*.c)
RUNTIME_OBJS := $(patsubst %.c, %.o, $(RUNTIMES))

APPS := $(wildcard $(APP_DIR)/*.c $(APP_DIR)/*.cpp)
OBJS := $(patsubst $(APP_DIR)/%.c, $(OBJ_DIR)/%.o, $(filter %.c, $(APPS))) \
        $(patsubst $(APP_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(filter %.cpp, $(APPS)))

PROGRAMS := $(patsubst $(APP_DIR)/%.c, $(OBJ_DIR)/%, $(filter %.c, $(APPS))) \
            $(patsubst $(APP_DIR)/%.cpp, $(OBJ_DIR)/%, $(filter %.cpp, $(APPS)))

TARGETS := $(patsubst $(APP_DIR)/%.c, %, $(filter %.c, $(APPS))) \
           $(patsubst $(APP_DIR)/%.cpp, %, $(filter %.cpp, $(APPS)))

TARGET_DIR := target
LDLIB := -L./runtime/ -lrv32iemu

mode ?= sim

ifeq ($(mode),sim)
CFLAGS += -D SIM_MODE
endif

simulator:
	g++ -O3 -o doodle_emulator doodle_emulator.cpp `sdl2-config --cflags --libs` -lSDL2_image

$(RUNTIME_OBJS) : %.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(RUNTIME_DIR)/librv32iemu.a: $(RUNTIME_OBJS)
	$(AR) -rc $@ $^

runtime/_start.o : runtime/_start.S
	$(CXX) $(CFLAGS) -o $@ -c $^

$(OBJS): $(OBJ_DIR)/%.o: $(APP_DIR)/%.cpp $(RUNTIME_DIR)/librv32iemu.a
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CFLAGS) -o $@ -c $<

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

$(TARGETS): %: $(OBJ_DIR)/% simulator

all: $(PROGRAMS)

.PHONY: all runtime