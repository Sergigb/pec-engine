CXX := g++
CC := gcc
INC := -I../include/ -I../include/bullet/ -I/usr/include/freetype2 -I../include/imgui/ -I../include/tinyxml2/
CXXFLAGS := $(INC) -Wall -Wextra -Werror -pedantic -ubsan -MMD -std=c++11
LDFLAGS := -L../lib/
LDLIBS :=  -lGL -lGLEW -lglfw -fopenmp -lfreetype -lassimp -lBulletDynamics -lBulletCollision -lLinearMath -lpng -ltinyxml2 -lzlibstatic
OBJPATH := ../bin
EXECPATH := ../bin

ifdef RELEASE
	CXXFLAGS := $(CXXFLAGS) -O3 -no-pie
else
	CXXFLAGS := $(CXXFLAGS) -ggdb
endif

# base app objects
CORESRCS := $(shell find core/ -name "*.cpp")
ASSETSSRCS := $(shell find assets/ -name "*.cpp")
GUISRCS := $(shell find GUI/ -name "*.cpp")
RENDERERSRCS := $(shell find renderers/ -name "*.cpp")

BASESRCS := $(CORESRCS) $(ASSETSSRCS) $(GUISRCS) $(RENDERERSRCS)
BASEOBJS := $(foreach source, $(BASESRCS), $(OBJPATH)/$(source:.cpp=.o))

DEPENDS_BASE = ${BASEOBJS:.o=.d}

#main game objects
MAIN_APP_SCRS := main.cpp App.cpp game_components/GameEditor.cpp game_components/GameSimulation.cpp
MAIN_APP_OBJS := $(foreach source, $(MAIN_APP_SCRS), $(OBJPATH)/$(source:.cpp=.o))

#planet renderer objects
PLANET_RENDERER_APP_SRCS := main_planet_renderer.cpp PlanetRenderer.cpp
PLANET_RENDERER_APP_OBJS := $(foreach source, $(PLANET_RENDERER_APP_SRCS), $(OBJPATH)/$(source:.cpp=.o))

# planetarium test thing
PLANETARIUM_APP_SRCS := main_planetarium.cpp Planetarium.cpp
PLANETARIUM_APP_OBJS := $(foreach source, $(PLANETARIUM_APP_SRCS), $(OBJPATH)/$(source:.cpp=.o))

DEPENDS = $(DEPENDS_BASE) ${MAIN_APP_OBJS:.o=.d} ${PLANET_RENDERER_APP_OBJS:.o=.d} ${PLANETARIUM_APP_OBJS:.o=.d}

#imgui
IMGUISRCS := $(wildcard ../thirdparty/imgui/*.cpp)
IMGUIOBJS := $(foreach source, $(IMGUISRCS:../thirdparty/imgui/%=%), $(OBJPATH)/$(source:.cpp=.o))

MAINOBJS := $(MAIN_APP_OBJS) $(BASEOBJS) $(IMGUIOBJS)
PLENET_RENDERER_OBJS := $(PLANET_RENDERER_APP_OBJS) $(BASEOBJS) $(IMGUIOBJS)
PLANETARIUMOBJS := $(PLANETARIUM_APP_OBJS) $(BASEOBJS) $(IMGUIOBJS)

.PHONY: clean clean-main clean-planetarium clean-planet-renderer clean-imgui clean-all

all: main planet-renderer planetarium

main: $(MAINOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(MAINOBJS) -o $(EXECPATH)/main $(LDLIBS)

planet-renderer: $(PLENET_RENDERER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PLENET_RENDERER_OBJS) -o $(EXECPATH)/planet-renderer $(LDLIBS)

planetarium: $(PLANETARIUMOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PLANETARIUMOBJS) -o $(EXECPATH)/planetarium $(LDLIBS)

$(OBJPATH)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui
$(OBJPATH)/%.o: ../thirdparty/imgui/%.cpp
	$(CXX) $(INC) -O3 -Wall -Wformat -DIMGUI_IMPL_OPENGL_LOADER_GLEW -c $< -o $@

clean: 
	rm -f $(BASEOBJS) $(MAIN_APP_OBJS) $(PLANET_RENDERER_APP_OBJS) ${DEPENDS} $(EXECPATH)/main $(EXECPATH)/planet-renderer
clean-main:
	rm -f $(MAIN_APP_OBJS) ${DEPENDS} $(EXECPATH)/main
clean-planet-renderer:
	rm -f $(PLANET_RENDERER_APP_OBJS) ${DEPENDS} $(EXECPATH)/planet-renderer
clean-planetarium:
	rm -f $(PLANETARIUM_APP_OBJS) ${DEPENDS} $(EXECPATH)/planetarium
clean-imgui:
	rm -f $(IMGUIOBJS)
clean-all:
	rm -rf $(OBJPATH)/* $(EXECPATH)/*

-include ${DEPENDS}
