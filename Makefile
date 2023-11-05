CC = gcc
# The use of certain ncurses library functions make it difficult
# to avoid unused variables. Hence, we skip such warnings
CFLAGS = -Wall -Werror -Wno-unused-but-set-variable
TARGET = editor
SRCDIR = src
INCDIR = include
OBJDIR = build

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^ -lncurses

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o $(TARGET)