CXX=g++ -Wall -std=c++0x -I../xbyak/ -I../include


release: main.cpp
	$(CXX) -O2 -o dot_product main.cpp
releaseavx: main.cpp
	$(CXX) -O2 -march=skylake-avx512 -o dot_product main.cpp

debug: main.cpp
	$(CXX) -g -o dot_product main.cpp

