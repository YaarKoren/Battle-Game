# ===== Simulator/Makefile =====
IDS        := 207177197_301251571
TARGET_DIR := bin
TARGET     := $(TARGET_DIR)/simulator_$(IDS)

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I. -I../common -I../UserCommon
LDFLAGS  :=
LDLIBS   := -ldl -lpthread        # keep -ldl for dlopen/dlsym, add -lpthread if you use threads

# Sources: everything in this folder + any UserCommon .cpp you need
SRC_DIRS := .
SRCS     := $(wildcard $(addsuffix /*.cpp,$(SRC_DIRS))) \
            $(wildcard ../UserCommon/*.cpp)

OBJ_DIR  := obj
OBJS     := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) | $(TARGET_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# Compile and mirror subdirectory layout into obj/
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)ב
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET_DIR)
