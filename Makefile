CC = clang
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g

TARGET = billing_engine
SOURCES = main.c customer.c storage.c billing.c server.c client.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
