
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
BIN = schemeful

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJS)
