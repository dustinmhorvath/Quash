quash: quash.o
	g++ -g -std=c++11 quash.o -o quash

quash.o: quash.cpp
	g++ -g -std=c++11 -c quash.cpp

clean:
	rm test *.o
