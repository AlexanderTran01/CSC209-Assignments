all : simple die large server

server : server.o
	gcc -Wall -g -o $@ $^

simple : simple.o cgi.o
	gcc -Wall -g -o $@ $^

die : die.o cgi.o
	gcc -Wall -g -o $@ $^

large : large.o cgi.o
	gcc -Wall -g -o $@ $^

%.o : %.c 
	gcc -Wall -g -c $<
	
# "make tests" will run the following tests on the following programs
tests : server simple die large
# Tests how server handles a simple CGI program takes a single GET request with one name=value pair
	-./server < msg1
# Tests how server handles a simple CGI program takes a single GET request with multiple name=value pair
	-./server < msg2
# Tests how server handles a large CGI program (one that outputs a relatively large amount of data) takes a single GET request
	-./server < msg3
	-./server < msg8
# Tests how server handles multiple CGI programs (some exitting with a signal)
	-./server < realmessages
	-./server < msg7
# Tests how server handles an empty query string
	-./server < msg4
# Tests how server handles bad GET line formatting
	-./server < msg5
	-./server < msg11
# Tests how server handles a filepath not existing in the directory
	-./server < msg6
# Tests how server handles input with any GET requests
	-./server < msg9
# Tests how server handles no input
	-./server < msg10

# Dependencies for header files
simple.o : cgi.h
large.o : cgi.h
cgi.o : cgi.h

clean : 
	rm *.o simple die large server
