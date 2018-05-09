test: interp interp-test.cpp
	g++ --std=c++0x interp-test.cpp interp.o -o test

interp: interp.cpp interp.hpp Optional.hpp
	g++ -c --std=c++0x interp.cpp -o interp.o
