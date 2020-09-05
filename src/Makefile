CXX = g++
CC = gcc
INC = -I../include/ -I../include/bullet/ -I/usr/include/freetype2 -I../include/imgui/
CXXFLAGS = $(INC) -Wall -Wextra -Werror -pedantic
LDFLAGS = -L../lib/
LDLIBS =  -lGL -lGLEW -lglfw -lz -fopenmp -lfreetype -lassimp -lIrrXML -lBulletDynamics -lBulletCollision -lLinearMath
OBJPATH = ../bin
EXECPATH = ../bin
PARTSPAHT = parts

MAINSRCS = main.cpp log.cpp utils.cpp gl_utils.cpp maths_funcs.cpp Camera.cpp Input.cpp WindowHandler.cpp Text2D.cpp FontAtlas.cpp Frustum.cpp DebugOverlay.cpp Model.cpp BtWrapper.cpp Object.cpp RenderContext.cpp App.cpp BaseApp.cpp BasePart.cpp EditorGUI.cpp PartsPanelGUI.cpp id_manager.cpp Vessel.cpp AssetManager.cpp AssetManagerInterface.cpp Player.cpp
MAINOBJS = $(foreach source, $(MAINSRCS), $(OBJPATH)/$(source:.cpp=.o))

IMGUISRCS = $(wildcard ../thirdparty/imgui/*.cpp)
IMGUIOBJS = $(foreach source, $(IMGUISRCS:../thirdparty/imgui/%=%), $(OBJPATH)/$(source:.cpp=.o))

ALLOBJS = $(MAINOBJS) $(IMGUIOBJS)


all: main

main: $(ALLOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(ALLOBJS) -o $(EXECPATH)/main $(LDLIBS)

$(OBJPATH)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o $(OBJPATH)/main.o

$(OBJPATH)/%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui
$(OBJPATH)/%.o: ../thirdparty/imgui/%.cpp
	$(CXX) $(INC) -O3 -Wall -Wformat -DIMGUI_IMPL_OPENGL_LOADER_GLEW -c $< -o $@

clean: 
	rm $(OBJPATH)/$(MAINOBJS) $(EXECPATH)/main
clean-imgui:
	rm $(OBJPATH)/$(IMGUIOBJS)
clean-all:
	rm $(OBJPATH)/*.o $(EXECPATH)/main
