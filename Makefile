
CC = gcc
LIBS = -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lm
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 $(LIBS) -Ofast
SRCS = main.c 
OBJS = $(SRCS:.c=.o)
EXEC = double-pendulum

.PHONY: all clean install uninstall

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
