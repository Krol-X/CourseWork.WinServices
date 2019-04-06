CPP      = g++.exe
#CC       = gcc.exe
WINDRES  = windres.exe

RES      = ksvcctl.res
OBJ      = ksvcctl.o $(RES)
LINKOBJ  = $(OBJ)
LIBS     = -L"P:/Dev-Cpp/MinGW64/x86_64-w64-mingw32/lib32" -L"P:/Dev-Cpp/MinGW64/x86_64-w64-mingw32/lib" -static-libgcc -mwindows -lcomctl32 -m32 -g3
CXXINCS  = -I"P:/Dev-Cpp/MinGW64/include" -I"P:/Dev-Cpp/MinGW64/x86_64-w64-mingw32/include" -I"P:/Dev-Cpp/MinGW64/lib/gcc/x86_64-w64-mingw32/4.9.2/include" -I"P:/Dev-Cpp/MinGW64/lib/gcc/x86_64-w64-mingw32/4.9.2/include/c++"
BIN      = ksvcctl.exe
CXXFLAGS = $(CXXINCS) -m32 -Wall -g3
RM       = rm.exe -f

all: $(BIN)

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

ksvcctl.o: ksvcctl.cpp
	$(CPP) -c ksvcctl.cpp -o ksvcctl.o $(CXXFLAGS)

ksvcctl.res: ksvcctl.rc
	$(WINDRES) -i ksvcctl.rc -F pe-i386 --input-format=rc -o ksvcctl.res -O coff
