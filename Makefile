CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = agent
SRC = main.cpp

compile:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: compile
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: compile run clean