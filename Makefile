#gcc -o hid_test ./detect_voltage_example/hid_test.c -I/usr/local -L/usr/local -lusb
CC = gcc
CFLAGS = -Wall -Wextra 
LDFLAGS = -I/usr/local -L/usr/local
LIBS = -lusb
PROJET = gpicase-battery
$(PROJET): $(PROJET).c
	$(CC) $(CFLAGS) -o $(PROJET) $(PROJET).c $(LDFLAGS)  $(LIBS)
clean:
	rm $(PROJET)
