OBJ_DIR = build
CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(CPP_FILES:%.cpp=$(OBJ_DIR)/%.o)
EXECUTABLE = obj-intel64/mica.so
DIRS = build obj-intel64

all: $(DIRS) $(OBJ_FILES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_FILES)
	g++ -shared -Wl,--hash-style=sysv -Wl,-rpath=../../../intel64/runtime/cpplibs -Wl,-Bsymbolic -Wl,--version-script=../../../source/include/pin/pintool.ver -o $@ $(OBJ_DIR)/*.o -lbz2 -lz ../../../source/tools/InstLib/obj-intel64/controller.a  -L../../../intel64/runtime/cpplibs -L../../../intel64/lib -L../../../intel64/lib-ext -L../../../intel64/runtime/glibc -L../../../extras/xed-intel64/lib -lpin -lxed -lpindwarf -ldl

$(OBJ_DIR)/%.o: %.cpp
	g++ -DBIGARRAY_MULTIPLIER=1 -Wall -Werror -Wno-unknown-pragmas -fno-stack-protector -DTARGET_IA32E -DHOST_IA32E -fPIC -DTARGET_LINUX  -I../../../source/include/pin -I../../../source/include/pin/gen -I../../../extras/components/include -I../../../extras/xed-intel64/include -I../../../source/tools/InstLib -O3 -fomit-frame-pointer -fno-strict-aliasing   -c -o $@ $<

$(DIRS):
	mkdir -p $(OBJ_DIR)
	mkdir -p obj-intel64

clean:
	rm -rf $(OBJ_DIR)
	rm -rf obj-intel64

.PHONY: all clean
