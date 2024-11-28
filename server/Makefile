CXX = g++
CXXFLAGS = -std=c++11 -Wall

SRCS = server.cpp
HEADERS = authentication.h directory.h file.h resources.h
OBJS = $(SRCS:.cpp=.o)
TARGET = server

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

dist-clean: clean
	rm -f *~