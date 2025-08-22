# ===== GameManager/Makefile =====
IDS     := 207177197_301251571
SO_NAME := GameManager_$(IDS).so

CXX      ?= g++-11
CPPFLAGS := -I. -I../common -I../UserCommon   # headers go in CPPFLAGS
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic -O2 -fPIC
LDFLAGS  := -shared
LDLIBS   :=

SRCS := $(wildcard *.cpp) $(wildcard ../Userבג ץץCommon/*.cpp)
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(SO_NAME)

$(SO_NAME): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(SO_NAME)
