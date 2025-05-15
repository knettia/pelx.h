CC       := gcc
CFLAGS   := -std=c99 -Wall -Wextra \
            -I.. \
            -I../dep \
            -DPELX_with_implementation

TARGET   := mushrooms.bin
SRCS     := mushrooms.c
OBJS     := $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
