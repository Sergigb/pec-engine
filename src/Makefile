CXX = g++
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic 
LDFLAGS = -lfreetype
OBJPATH = ../bin
EXECPATH = ../bin
PARTSPAHT = parts/
GLFLAGS = -lGL -lGLEW -lglfw -lz -fopenmp
INC = -I../include/ -I../include/bullet/ -I/usr/include/freetype2 
LIBS = ../lib/libassimp.a ../lib/libIrrXML.a ../lib/libBulletDynamics.a ../lib/libBulletCollision.a ../lib/libLinearMath.a


all: opengl output

output: $(OBJPATH)/main.o $(OBJPATH)/BasePart.o $(OBJPATH)/Ship.o $(OBJPATH)/CommandModule.o $(OBJPATH)/LaunchEscapeAssembly.o $(OBJPATH)/ResourceContainer.o $(OBJPATH)/Engine.o $(OBJPATH)/ServiceModule.o
	$(CXX) $(OBJPATH)/main.o $(OBJPATH)/BasePart.o  $(OBJPATH)/Ship.o $(OBJPATH)/CommandModule.o $(OBJPATH)/LaunchEscapeAssembly.o $(OBJPATH)/ResourceContainer.o $(OBJPATH)/Engine.o $(OBJPATH)/ServiceModule.o -o $(EXECPATH)/output $(CFLAGS)

$(OBJPATH)/main.o: main.cpp
	$(CXX) -c main.cpp -o $(OBJPATH)/main.o  $(CFLAGS)

$(OBJPATH)/Ship.o: Ship.cpp Ship.hpp
	$(CXX) -c Ship.cpp -o $(OBJPATH)/Ship.o  $(CFLAGS)

$(OBJPATH)/BasePart.o: BasePart.cpp BasePart.hpp
	$(CXX) -c BasePart.cpp -o $(OBJPATH)/BasePart.o  $(CFLAGS)

$(OBJPATH)/CommandModule.o: $(PARTSPAHT)CommandModule.cpp $(PARTSPAHT)CommandModule.hpp
	$(CXX) -c $(PARTSPAHT)CommandModule.cpp -o $(OBJPATH)/CommandModule.o  $(CFLAGS)

$(OBJPATH)/ServiceModule.o: $(PARTSPAHT)ServiceModule.cpp $(PARTSPAHT)ServiceModule.hpp
	$(CXX) -c $(PARTSPAHT)ServiceModule.cpp -o $(OBJPATH)/ServiceModule.o  $(CFLAGS)

$(OBJPATH)/LaunchEscapeAssembly.o: $(PARTSPAHT)LaunchEscapeAssembly.cpp $(PARTSPAHT)LaunchEscapeAssembly.hpp
	$(CXX) -c $(PARTSPAHT)LaunchEscapeAssembly.cpp -o $(OBJPATH)/LaunchEscapeAssembly.o  $(CFLAGS)

$(OBJPATH)/ResourceContainer.o: $(PARTSPAHT)ResourceContainer.cpp $(PARTSPAHT)ResourceContainer.hpp
	$(CXX) -c $(PARTSPAHT)ResourceContainer.cpp -o $(OBJPATH)/ResourceContainer.o  $(CFLAGS)

$(OBJPATH)/Engine.o: $(PARTSPAHT)Engine.cpp $(PARTSPAHT)Engine.hpp
	$(CXX) -c $(PARTSPAHT)Engine.cpp -o $(OBJPATH)/Engine.o  $(CFLAGS)

opengl: $(OBJPATH)/opengl.o $(OBJPATH)/log.o $(OBJPATH)/utils.o $(OBJPATH)/gl_utils.o $(OBJPATH)/maths_funcs.o $(OBJPATH)/Camera.o $(OBJPATH)/Input.o $(OBJPATH)/WindowHandler.o $(OBJPATH)/FontAtlas.o $(OBJPATH)/Text2D.o $(OBJPATH)/Frustum.o $(OBJPATH)/DebugOverlay.o $(OBJPATH)/Model.o $(OBJPATH)/BtWrapper.o $(OBJPATH)/Object.o
	$(CXX) $(CFLAGS) $(INC) $(OBJPATH)/opengl.o $(OBJPATH)/log.o $(OBJPATH)/utils.o $(OBJPATH)/gl_utils.o $(OBJPATH)/maths_funcs.o $(OBJPATH)/Camera.o $(OBJPATH)/Input.o $(OBJPATH)/WindowHandler.o $(OBJPATH)/FontAtlas.o  $(OBJPATH)/Text2D.o $(OBJPATH)/Frustum.o $(OBJPATH)/DebugOverlay.o $(OBJPATH)/Model.o $(OBJPATH)/BtWrapper.o  $(OBJPATH)/Object.o $(LIBS) $(GLFLAGS) $(LDFLAGS) -o $(EXECPATH)/opengl

$(OBJPATH)/opengl.o: opengl.cpp
	$(CXX) -c opengl.cpp -o $(OBJPATH)/opengl.o $(CFLAGS) $(INC)

$(OBJPATH)/log.o: log.cpp log.hpp log.tpp
	$(CXX) -c log.cpp -o $(OBJPATH)/log.o $(CFLAGS)

$(OBJPATH)/utils.o: utils.cpp utils.hpp
	$(CXX) -c utils.cpp -o $(OBJPATH)/utils.o $(CFLAGS)

$(OBJPATH)/gl_utils.o: gl_utils.cpp gl_utils.hpp
	$(CXX) -c gl_utils.cpp -o $(OBJPATH)/gl_utils.o $(CFLAGS) $(INC)

$(OBJPATH)/maths_funcs.o: maths_funcs.cpp maths_funcs.hpp
	$(CXX) -c maths_funcs.cpp -o $(OBJPATH)/maths_funcs.o $(CFLAGS)

$(OBJPATH)/Camera.o: Camera.cpp Camera.hpp
	$(CXX) -c Camera.cpp -o $(OBJPATH)/Camera.o $(CFLAGS)

$(OBJPATH)/Input.o: Input.cpp Input.hpp
	$(CXX) -c Input.cpp -o $(OBJPATH)/Input.o $(CFLAGS)

$(OBJPATH)/WindowHandler.o: WindowHandler.cpp WindowHandler.hpp
	$(CXX) -c WindowHandler.cpp -o $(OBJPATH)/WindowHandler.o $(CFLAGS)

$(OBJPATH)/Text2D.o: Text2D.cpp Text2D.hpp 
	$(CXX) -c Text2D.cpp -o $(OBJPATH)/Text2D.o $(CFLAGS) $(INC)

$(OBJPATH)/FontAtlas.o: FontAtlas.cpp FontAtlas.hpp 
	$(CXX) -c FontAtlas.cpp -o $(OBJPATH)/FontAtlas.o $(CFLAGS) $(INC)

$(OBJPATH)/Frustum.o: Frustum.cpp Frustum.hpp 
	$(CXX) -c Frustum.cpp -o $(OBJPATH)/Frustum.o $(CFLAGS) $(INC)

$(OBJPATH)/DebugOverlay.o: DebugOverlay.cpp DebugOverlay.hpp 
	$(CXX) -c DebugOverlay.cpp -o $(OBJPATH)/DebugOverlay.o $(CFLAGS) $(INC)

$(OBJPATH)/Model.o: Model.cpp Model.hpp 
	$(CXX) -c Model.cpp -o $(OBJPATH)/Model.o $(CFLAGS) $(INC)

$(OBJPATH)/BtWrapper.o: BtWrapper.cpp BtWrapper.hpp 
	$(CXX) -c BtWrapper.cpp -o $(OBJPATH)/BtWrapper.o $(CFLAGS) $(INC)

$(OBJPATH)/Object.o: Object.cpp Object.hpp 
	$(CXX) -c Object.cpp -o $(OBJPATH)/Object.o $(CFLAGS) $(INC)

clean: 
	rm $(OBJPATH)/*.o $(EXECPATH)/output
