.PHONY: clean

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -g
LDLIBS := -lraylib

OBJS := main.o maze.o
TARGET := maze

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS)

main.o: main.cpp maze.hpp game_settings.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp 

maze.o: maze.cpp maze.hpp game_settings.hpp
	$(CXX) $(CXXFLAGS) -c maze.cpp $(LDLIBS)


clean:
	rm -rf $(TARGET) $(OBJS)
