#Copyright Notice:
#The files within this zip file are copyrighted by Lazy Foo' Productions (2004-2014)
#and may not be redistributed without written permission.

#OBJS specifies which files to compile as part of the project
OBJS = dxf.c bmp.c graph.c dxf_graph.c interface.c shape2.c list.c dxf_create.c dxf_attract.c tinyfiledialogs.c

#CC specifies which compiler we're using
CC = gcc

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -I. -Iinclude\SDL2

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -Llib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
COMPILER_FLAGS =

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = `sdl2-config --cflags --libs`

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = teste9

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)