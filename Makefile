CXX=g++ 
CXXFLAGS= -pthread -std=c++11 -g

rideshare : main.o io.o structures.o thread_functions.o
	$(CXX) $(CXXFLAGS) -o rideshare $^ -lpthread -lrt

main.o : structures.h thread_functions.h io.h main.cpp

structures.o: structures.cpp structures.h

io.o : io.cpp io.h 

thread_functions.o : thread_functions.cpp thread_functions.h

clean :
	rm *.o