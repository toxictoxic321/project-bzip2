# ==========================================
# BZip2 Implementation Makefile
# ==========================================

CC = gcc
CFLAGS = -Wall -O2 -I./include
TARGET = bzip2_impl

# List all source files (including Stage 2 & 3 additions)
SOURCES = src/main.c src/rle.c src/bwt.c src/mtf.c src/rle2.c src/huffman.c src/config.c
OBJECTS = $(SOURCES:.c=.o)

# ------------------------------------------
# Platform Detection
# ------------------------------------------
ifeq ($(OS),Windows_NT)
    # Windows native build settings
    EXEC = $(TARGET).exe
    RM = del /Q /F
    CLEAN_OBJS = src\*.o
else
    # Linux native build settings
    EXEC = $(TARGET)
    RM = rm -f
    CLEAN_OBJS = $(OBJECTS)
endif

CC_WIN = x86_64-w64-mingw32-gcc

# ------------------------------------------
# Targets
# ------------------------------------------
all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

windows:
	$(CC_WIN) $(CFLAGS) $(SOURCES) -o $(TARGET).exe

clean:
	$(RM) $(CLEAN_OBJS) $(EXEC) $(TARGET).exe