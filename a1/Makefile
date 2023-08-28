SHELL = /bin/bash
FLAGS = -Wall -g

all: seqbot

%.o: %.c 
	gcc ${FLAGS} -c $<

seqbot: seqbot_main.o seqbot_helpers.o
	gcc ${FLAGS} -o $@ $^

clean:
	rm seqbot *.o
