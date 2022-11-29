CXX = g++
CXXFLAGS = -Wall -std=c++20 -pedantic
OBJECTS = main.o

all: signcipher

signcipher: $(OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^

clean:
	$(RM) signcipher *.o core *~

