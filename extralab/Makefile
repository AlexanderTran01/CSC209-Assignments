all : test_wordlist test_constraints

test_wordlist : test_wordlist.o wordlist.o 
	gcc -Wall -g -o $@ $^

test_constraints : test_constraints.o constraints.o
	gcc -Wall -g -o $@ $^

%.o : %.c 
	gcc -Wall -g -c $<

# "make constraint_tests" will run the four tests on test_constraints
constraint_tests : test_constraints
	./run_con_tests.sh testfiles/con1.input testfiles/con1.expected 
	./run_con_tests.sh testfiles/con2.input testfiles/con2.expected 
	./run_con_tests.sh testfiles/con3.input testfiles/con3.expected 
	./run_con_tests.sh testfiles/con4.input testfiles/con4.expected 

# "make wordlist_tests" will run the tests one test_wordlist
wordlist_tests : test_wordlist
	./run_word_tests.sh testfiles/small_words5.txt
	./run_word_tests.sh words5.txt 


# Dependencies for header files
# In practice there are tools to automatically generate these dependencies
wordlist.o : wordlist.h
constraints.o : constraints.h
test_constraints.o : constraints.h
test_wordlist.o : wordlist.h

clean : 
	rm *.o *.out test_wordlist test_constraints
