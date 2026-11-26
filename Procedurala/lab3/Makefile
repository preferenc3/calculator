CC = clang
CXX = clang++
CFLAGS = -Wall -Wextra -O2 -fPIC
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LIB_TARGET = mylib.dylib
    LIB_LDFLAGS = -dynamiclib
    APP_LDFLAGS =
else
    LIB_TARGET = mylib.so
    LIB_LDFLAGS = -shared
    APP_LDFLAGS = -ldl
endif

APP_TARGET = main

all: $(LIB_TARGET) $(APP_TARGET)

$(LIB_TARGET): mylib.c mylib.h
	$(CC) $(CFLAGS) $(LIB_LDFLAGS) -o $@ $<

$(APP_TARGET): main.cpp mylib.h
	$(CXX) $(CXXFLAGS) main.cpp -o $@ $(APP_LDFLAGS)

run: $(LIB_TARGET) $(APP_TARGET)
	./$(APP_TARGET)

clean:
	rm -f $(APP_TARGET) mylib.dylib mylib.so mylib.dll *.o

.PHONY: all run clean
