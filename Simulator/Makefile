# ===== Simulator/Makefile =====
IDS        := 207177197_301251571
TARGET     := simulator_$(IDS)  # create the executable directly in the Simulator folder

CXX      ?= g++-11
CPPFLAGS := -I. -I../common -I../UserCommon   # headers go in CPPFLAGS
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic -O2 -fPIC
LDFLAGS  := -rdynamic
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
