CXX = g++
CC = gcc
INC = -I../include/ -I../include/bullet/ -I/usr/include/freetype2
CXXFLAGS = $(INC) -Wall -Wextra -Werror -pedantic
LDFLAGS = -L../lib/
LDLIBS =  -lGL -lGLEW -lglfw -lz -fopenmp -lfreetype -lassimp -lIrrXML -lBulletDynamics -lBulletCollision -lLinearMath
OBJPATH = ../bin
EXECPATH = ../bin
PARTSPAHT = parts

MAINSRCS = main.cpp log.cpp utils.cpp gl_utils.cpp maths_funcs.cpp Camera.cpp Input.cpp WindowHandler.cpp Text2D.cpp FontAtlas.cpp Frustum.cpp DebugOverlay.cpp Model.cpp BtWrapper.cpp Object.cpp RenderContext.cpp App.cpp BaseApp.cpp BasePart.cpp EditorGUI.cpp PartsPanelGUI.cpp
MAINOBJS = $(foreach source, $(MAINSRCS), $(OBJPATH)/$(source:.cpp=.o))

all: main

main: $(MAINOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(MAINOBJS) -o $(EXECPATH)/main $(LDLIBS)

$(OBJPATH)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o $(OBJPATH)/main.o

$(OBJPATH)/%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


clean: 
	rm $(OBJPATH)/*.o $(EXECPATH)/main
