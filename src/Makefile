CXX = g++
CC = gcc
INC = -I../include/ -I../include/bullet/ -I/usr/include/freetype2 -I../include/imgui/
CXXFLAGS = $(INC) -Wall -Wextra -Werror -pedantic -ubsan -g
LDFLAGS = -L../lib/
LDLIBS =  -lGL -lGLEW -lglfw -lz -fopenmp -lfreetype -lassimp -lIrrXML -lBulletDynamics -lBulletCollision -lLinearMath -lpng
OBJPATH = ../bin
EXECPATH = ../bin

BASESRCS = log.cpp utils.cpp gl_utils.cpp maths_funcs.cpp Camera.cpp Input.cpp WindowHandler.cpp Text2D.cpp FontAtlas.cpp Frustum.cpp DebugOverlay.cpp Model.cpp BtWrapper.cpp Object.cpp RenderContext.cpp BaseApp.cpp BasePart.cpp EditorGUI.cpp PartsPanelGUI.cpp id_manager.cpp Vessel.cpp AssetManager.cpp AssetManagerInterface.cpp Player.cpp Resource.cpp assets_utils.cpp StagingPanelGUI.cpp Kinematic.cpp DebugDrawer.cpp
BASEOBJS = $(foreach source, $(BASESRCS), $(OBJPATH)/$(source:.cpp=.o))

PARTSSRCS = $(wildcard parts/*.cpp)
PARTSOBJS = $(foreach source, $(PARTSSRCS:parts/%=%), $(OBJPATH)/$(source:.cpp=.o))

MAIN_APP_SCRS = main.cpp App.cpp
MAIN_APP_OBJS = $(foreach source, $(MAIN_APP_SCRS), $(OBJPATH)/$(source:.cpp=.o))

PLANETARIUM_APP_SRCS = main_planetarium.cpp Planetarium.cpp
PLANETARIUM_APP_OBJS = $(foreach source, $(PLANETARIUM_APP_SRCS), $(OBJPATH)/$(source:.cpp=.o))

IMGUISRCS = $(wildcard ../thirdparty/imgui/*.cpp)
IMGUIOBJS = $(foreach source, $(IMGUISRCS:../thirdparty/imgui/%=%), $(OBJPATH)/$(source:.cpp=.o))

MAINOBJS = $(MAIN_APP_OBJS) $(BASEOBJS) $(IMGUIOBJS) $(PARTSOBJS)
PLANETARIUMOBJS = $(PLANETARIUM_APP_OBJS) $(BASEOBJS) $(IMGUIOBJS) $(PARTSOBJS)


all: main planetarium

main: $(MAINOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(MAINOBJS) -o $(EXECPATH)/main $(LDLIBS)

planetarium: $(PLANETARIUMOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PLANETARIUMOBJS) -o $(EXECPATH)/planetarium $(LDLIBS)

# temporal, need to find a better system
$(OBJPATH)/%.o: parts/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJPATH)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui
$(OBJPATH)/%.o: ../thirdparty/imgui/%.cpp
	$(CXX) $(INC) -O3 -Wall -Wformat -DIMGUI_IMPL_OPENGL_LOADER_GLEW -c $< -o $@

clean: 
	rm -f $(OBJPATH)/$(BASEOBJS) $(OBJPATH)/$(MAIN_APP_OBJS) $(OBJPATH)/$(PLANETARIUM_APP_OBJS) $(EXECPATH)/main $(EXECPATH)/planetarium
clean-main:
	rm -f $(OBJPATH)/$(MAIN_APP_OBJS) $(EXECPATH)/main
clean-planetarium:
	rm -f $(OBJPATH)/$(PLANETARIUM_APP_OBJS) $(EXECPATH)/planetarium
clean-imgui:
	rm -f $(OBJPATH)/$(IMGUIOBJS)
clean-all:
	rm -f $(OBJPATH)/*.o $(EXECPATH)/main
