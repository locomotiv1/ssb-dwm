CC = gcc

CFLAGS = -Wall -Wextra -O2

LIBS = -lX11 -lasound

SRC = main.c

TARGET = ssb-dwm

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

.PHONY: all clean install
