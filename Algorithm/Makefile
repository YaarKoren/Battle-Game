# ===== Algorithm/Makefile =====
IDS     := 207177197_301251571
SO_NAME := Algorithm_$(IDS).so

CXX      ?= g++-11
CPPFLAGS := -I. -I../common -I../UserCommon   # headers go in CPPFLAGS
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic -O2 -fPIC
LDFLAGS  := -shared
LDLIBS   :=

# Add every .cpp that DEFINES symbols you call from the GM
GM_SRC := \
    MyGameManager.cpp \
    ../UserCommon/Board.cpp \
    ../UserCommon/MyBattleInfo.cpp \
    ../UserCommon/SatelliteViewImpl.cpp \
    ../UserCommon/Shell.cpp \
    ../UserCommon/Tank.cpp


GM_OBJ := $(GM_SRC:.cpp=.o)
GM_DEP := $(GM_OBJ:.o=.d)

# All .cpp here + UserCommon if you need it
SRCS := $(wildcard *.cpp) $(wildcard ../UserCommon/*.cpp)
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(SO_NAME)

$(SO_NAME): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(SO_NAME)
