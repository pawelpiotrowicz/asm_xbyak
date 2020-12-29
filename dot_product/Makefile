CXX=g++ -Wall -std=c++0x -I../xbyak/


release: main.cpp
	$(CXX) -O2 -o prog main.cpp
releaseavx: main.cpp
	$(CXX) -O2 -march=skylake-avx512 -o prog main.cpp

debug: main.cpp
	$(CXX) -g -o prog main.cpp

