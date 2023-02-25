CFLAGS  = -Wall -g -ggdb --std=gnu++14
INCLUDE = -I`llvm-config-6.0 --includedir` -IbJou/ -IbJou/include/ -IbJou/tclap/ -IbJou/nolibc_syscall/
LDFLAGS = `llvm-config-6.0 --ldflags --system-libs` -L./ -lLLVM-6.0 -lpthread -lnolibc_syscall
DEFINES = -DBJOU_USE_COLOR -DNDEBUG -fno-rtti

#bJou := $(filter-out bJou/src/main.cpp, $(wildcard bJou/src/*.cpp)) $(wildcard bJou/tclap/*.cpp)
bJou := $(wildcard bJou/src/*.cpp) $(wildcard bJou/tclap/*.cpp)
nolibc_syscall = bJou/nolibc_syscall/nolibc_syscall.c
bJouDemangle = bJou/src/bJouDemangle.c
getRSS = bJou/src/getRSS.c

OUT = p

all:
	clang -Wall -g -ggdb $(INCLUDE) $(DEFINES) -c $(getRSS)
	clang -Wall -g -ggdb $(INCLUDE) $(DEFINES) -c $(bJouDemangle)
	clang++ $(CFLAGS) $(INCLUDE) $(DEFINES) -o $(OUT) $(bJou) getRSS.o bJouDemangle.o $(LDFLAGS)

libnolib_syscall:
	clang -Wall -g -ggdb $(INCLUDE) $(DEFINES) -shared -fpic -o libnolibc_syscall.so $(nolibc_syscall) 
	cp libnolibc_syscall.so /usr/lib/

test:
	./$(OUT) hello.bjou -I bJou/modules

clean:
	rm p *.o
