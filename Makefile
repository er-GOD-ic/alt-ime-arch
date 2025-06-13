CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -Iinclude 

SRCDIR = src
BUILDDIR = build
TARGET = $(BUILDDIR)/alt-ime-arch

SOURCES = $(SRCDIR)/main.cpp
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
