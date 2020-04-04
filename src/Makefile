CXX = g++
CC = gcc
INC = -I../include/ -I../include/bullet/ -I/usr/include/freetype2
CXXFLAGS = $(INC) -Wall -Wextra -Werror -pedantic
LDFLAGS = -L../lib/
LDLIBS =  -lGL -lGLEW -lglfw -lz -fopenmp -lfreetype -lassimp -lIrrXML -lBulletDynamics -lBulletCollision -lLinearMath
OBJPATH = ../bin
EXECPATH = ../bin
PARTSPAHT = parts

MAINSRCS = main.cpp Ship.cpp BasePart.cpp CommandModule.cpp ServiceModule.cpp LaunchEscapeAssembly.cpp ResourceContainer.cpp Engine.cpp
MAINOBJS = $(foreach source, $(MAINSRCS), $(OBJPATH)/$(source:.cpp=.o))

OPENGLSRCS = opengl.cpp log.cpp utils.cpp gl_utils.cpp maths_funcs.cpp Camera.cpp Input.cpp WindowHandler.cpp Text2D.cpp FontAtlas.cpp Frustum.cpp DebugOverlay.cpp Model.cpp BtWrapper.cpp Object.cpp RenderContext.cpp App.cpp
OPENGLOBJS = $(foreach source, $(OPENGLSRCS), $(OBJPATH)/$(source:.cpp=.o))

all: opengl main

main: $(MAINOBJS)
	$(CXX) $(CXXFLAGS) $(MAINOBJS) -o $(EXECPATH)/main

$(OBJPATH)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o $(OBJPATH)/main.o

$(OBJPATH)/%.o: $(PARTSPAHT)/%.cpp $(PARTSPAHT)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

opengl: $(OPENGLOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OPENGLOBJS) -o $(EXECPATH)/opengl $(LDLIBS)

$(OBJPATH)/opengl.o: opengl.cpp
	$(CXX) $(CXXFLAGS) -c opengl.cpp -o $(OBJPATH)/opengl.o

$(OBJPATH)/%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


clean: 
	rm $(OBJPATH)/*.o $(EXECPATH)/main $(EXECPATH)/opengl
