CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Isrc
LDFLAGS ?=
TARGET := urgencetriage
SOURCES := src/main.c src/patient.c src/file.c src/stats.c src/fichier.c src/outils.c
OBJDIR := build/obj
OBJECTS := $(SOURCES:src/%.c=$(OBJDIR)/%.o)

.PHONY: all run clean valgrind

all: $(TARGET)

$(TARGET): $(OBJECTS) | data
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

$(OBJDIR)/%.o: src/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

data:
	mkdir -p data

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clean:
	rm -f $(OBJECTS) src/*.o $(TARGET)
	rmdir $(OBJDIR) build 2>/dev/null || true