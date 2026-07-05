CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = ims
SRCS     = main.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all run clean
