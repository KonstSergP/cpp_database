
CXX = g++

CXXFLAGS = \
	-std=c++20 \
	-Wall      \
	-Wextra    \
	-Werror \
	-g3 \
	-O0 \
	-fsanitize=undefined,address

LDFLAGS = \
		-fsanitize=undefined,address

BRED    = \033[1;31m
BGREEN  = \033[1;32m
BYELLOW = \033[1;33m
GREEN   = \033[1;35m
BCYAN   = \033[1;36m
RESET   = \033[0m

#-------
# Files
#-------

# Add "include" folder to header search path:
#CXXFLAGS += -I $(abspath include)

INCLUDES = $(shell find ./ -name \*.hpp)

SOURCES := $(shell find ./src -name \*.cpp)

VPATH := $(dir $(SOURCES))

OBJDIR := ./build
OBJECTS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(notdir $(SOURCES)))

EXECUTABLE := $(OBJDIR)/main

# Build process

$(OBJDIR)/%: %.cpp $(OBJECTS)
	@printf "$(BYELLOW)Building object file $(BCYAN)$@$(RESET)\n"
	@mkdir -p build
	$(CXX) -c $< $(CXXFLAGS) -o $@.o
	@printf "$(BYELLOW)Linking executable $(BCYAN)$@$(RESET)\n"
	@$(CXX) $(LDFLAGS) $(OBJECTS) $@.o -o $@

default: $(EXECUTABLE)

$(OBJDIR)/%.o: %.cpp $(INCLUDES) Makefile
	@printf "$(BYELLOW)Building object file $(BCYAN)$@$(RESET)\n"
	@mkdir -p build
	$(CXX) -c $< $(CXXFLAGS) -o $@



run: $(EXECUTABLE)
	@printf "$(BYELLOW)Running file $(BCYAN)$<$(RESET)\n"
	@./$(EXECUTABLE)

debug: $(EXECUTABLE)
	@printf "$(BYELLOW)Debugging file $(BCYAN)$<$(RESET)\n"
	@gdb $(EXECUTABLE)

clean:
	@printf "$(BYELLOW)Cleaning build directory $(RESET)\n"
	rm -rf build


.PHONY: run clean default
.PRECIOUS: $(OBJDIR)/%.o % $(OBJDIR)/%
