# Name: Eron Ristich
# Date: 5/10/22

OBJS = water.o kernel.o main.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
LDLIBS = -Llib -lmingw32 -lopengl32 -lSDL2_ttf -lglew32 -lglu32 -lfreeglut -lSDL2main -lSDL2 -lSDL2_image -lglew32mx -lassimp.dll
INC = -Iinclude

EWS.exe : $(OBJS)
	$(CC) $(LFLAGS) $(INC) $(OBJS) -o EWS.exe $(LDLIBS)

water.o : objects/water.h objects/helper.h objects/water.cpp
	$(CC) $(CFLAGS) $(INC) objects/water.cpp

kernel.o : objects/skybox.h objects/camera.h objects/helper.h objects/water.h kernel/kernel.h kernel/kernel.cpp
	$(CC) $(CFLAGS) $(INC) kernel/kernel.cpp

main.o : objects/camera.h objects/helper.h kernel/kernel.h main.cpp
	$(CC) $(CFLAGS) $(INC) main.cpp

clean:
	\rm *.o *~ EWS.exe
