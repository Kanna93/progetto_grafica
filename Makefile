# Project: progettoCar4

CPP  = g++
CC   = gcc
BIN  = progetto_grafica

OBJ  = main.o car.o mesh.o GLText.o glm.o
LINKOBJ  = main.o car.o mesh.o GLText.o glm.o

# Library linking
OS := $(shell uname)
FRMPATH = -IC:\MinGW\include
LIBS = -Lc:\MinGW\lib -lmingw32 -static-libgcc -static-libstdc++ -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lopengl32 -lglu32

FLAG = -Wno-deprecated
RM = rm -f
CFLAGS = -c -Wall -Ic:\MinGW\include 

all: $(BIN)

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(FRMPATH) $(LIBS)

main.o: main.cpp
	$(CPP) -c $(FRMPATH) main.cpp -o main.o

car.o: car.cpp
	$(CPP) -c $(FRMPATH) car.cpp -o car.o

mesh.o: mesh.cpp
	$(CPP) -c $(FRMPATH) mesh.cpp -o mesh.o

GLText.o: GLText.cpp
	$(CPP) -c $(FRMPATH) GLText.cpp -o GLText.o
	
glm.o: glm.cpp
	$(CPP) -c $(FRMPATH) glm.cpp -o glm.o

run: $(BIN)
	./$(BIN)
