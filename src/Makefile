CXX := g++
CC := gcc
INC := -I../include/ -I../include/bullet/ -I/usr/include/freetype2 -I../include/imgui/
CXXFLAGS := $(INC) -Wall -Wextra -Werror -pedantic -ubsan -g
LDFLAGS := -L../lib/
LDLIBS :=  -lGL -lGLEW -lglfw -lz -fopenmp -lfreetype -lassimp -lIrrXML -lBulletDynamics -lBulletCollision -lLinearMath -lpng
OBJPATH := ../bin
EXECPATH := ../bin

# base app objects
CORESRCS := $(shell find core/ -name "*.cpp")
ASSETSSRCS := $(shell find assets/ -name "*.cpp")
GUISRCS := $(shell find GUI/ -name "*.cpp")

BASESRCS := $(CORESRCS) $(ASSETSSRCS) $(GUISRCS)
BASEOBJS := $(foreach source, $(BASESRCS), $(OBJPATH)/$(source:.cpp=.o))

#main game objects
MAIN_APP_SCRS := main.cpp App.cpp GameEditor.cpp
MAIN_APP_OBJS := $(foreach source, $(MAIN_APP_SCRS), $(OBJPATH)/$(source:.cpp=.o))

#planetarium objects
PLANETARIUM_APP_SRCS := main_planetarium.cpp Planetarium.cpp
PLANETARIUM_APP_OBJS := $(foreach source, $(PLANETARIUM_APP_SRCS), $(OBJPATH)/$(source:.cpp=.o))

#imgui
IMGUISRCS := $(wildcard ../thirdparty/imgui/*.cpp)
IMGUIOBJS := $(foreach source, $(IMGUISRCS:../thirdparty/imgui/%=%), $(OBJPATH)/$(source:.cpp=.o))

MAINOBJS := $(MAIN_APP_OBJS) $(BASEOBJS) $(IMGUIOBJS)
PLANETARIUMOBJS := $(PLANETARIUM_APP_OBJS) $(BASEOBJS) $(IMGUIOBJS)

.PHONY: clean clean-main clean-planetarium clean-imgui clean-all

all: main planetarium

main: $(MAINOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(MAINOBJS) -o $(EXECPATH)/main $(LDLIBS)

planetarium: $(PLANETARIUMOBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PLANETARIUMOBJS) -o $(EXECPATH)/planetarium $(LDLIBS)

$(OBJPATH)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# imgui
$(OBJPATH)/%.o: ../thirdparty/imgui/%.cpp
	$(CXX) $(INC) -O3 -Wall -Wformat -DIMGUI_IMPL_OPENGL_LOADER_GLEW -c $< -o $@

clean: 
	rm -f $(BASEOBJS) $(MAIN_APP_OBJS) $(PLANETARIUM_APP_OBJS) $(EXECPATH)/main $(EXECPATH)/planetarium
clean-main:
	rm -f $(MAIN_APP_OBJS) $(EXECPATH)/main
clean-planetarium:
	rm -f $(PLANETARIUM_APP_OBJS) $(EXECPATH)/planetarium
clean-imgui:
	rm -f $(IMGUIOBJS)
clean-all:
	rm -rf $(OBJPATH)/* $(EXECPATH)/*