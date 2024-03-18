CC = gcc
CFLAGS = -Wall -g

TARGET = spchk

OBJS = mainfile.o dictionary.o file-processing.o

DEPS = dictionary.h file-processing.h

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS)
