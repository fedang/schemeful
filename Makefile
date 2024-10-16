CFLAGS = -ggdb -Wall
# CFLAGS += -DANY_SEXP_NO_BOXING -DANY_LOG_VALUE_GENERIC_TYPE=any_sexp_t

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
