CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = $(shell pkg-config --cflags glfw3 assimp) -I../dependencies
CFLAGS = -Wall -std=c++20 -fPIC -m64 -fexceptions
RCFLAGS =
RESINC =
LIBDIR =
LIB = $(shell pkg-config --libs glfw3 assimp) -lGL
LDFLAGS = -m64

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -g -D_DEBUG
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/libengine.so

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/libengine.so

OBJ_DEBUG = $(OBJDIR_DEBUG)/engine.o $(OBJDIR_DEBUG)/camera.o $(OBJDIR_DEBUG)/light.o $(OBJDIR_DEBUG)/mesh.o $(OBJDIR_DEBUG)/node.o $(OBJDIR_DEBUG)/loader.o $(OBJDIR_DEBUG)/texture.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/engine.o $(OBJDIR_RELEASE)/camera.o $(OBJDIR_RELEASE)/light.o $(OBJDIR_RELEASE)/mesh.o $(OBJDIR_RELEASE)/node.o $(OBJDIR_RELEASE)/loader.o $(OBJDIR_RELEASE)/texture.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d bin/Debug || mkdir -p bin/Debug
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) -shared $(LIBDIR_DEBUG) $(OBJ_DEBUG)  -o $(OUT_DEBUG) $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/engine.o: engine.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c engine.cpp -o $(OBJDIR_DEBUG)/engine.o

$(OBJDIR_DEBUG)/camera.o: camera.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c camera.cpp -o $(OBJDIR_DEBUG)/camera.o

$(OBJDIR_DEBUG)/light.o: light.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c light.cpp -o $(OBJDIR_DEBUG)/light.o

$(OBJDIR_DEBUG)/mesh.o: mesh.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c mesh.cpp -o $(OBJDIR_DEBUG)/mesh.o

$(OBJDIR_DEBUG)/node.o: node.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c node.cpp -o $(OBJDIR_DEBUG)/node.o

$(OBJDIR_DEBUG)/loader.o: loader.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c loader.cpp -o $(OBJDIR_DEBUG)/loader.o

$(OBJDIR_DEBUG)/texture.o: texture.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c texture.cpp -o $(OBJDIR_DEBUG)/texture.o

clean_debug:
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf bin/Debug
	rm -rf $(OBJDIR_DEBUG)

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 

release: before_release out_release after_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) -shared $(LIBDIR_RELEASE) $(OBJ_RELEASE)  -o $(OUT_RELEASE) $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/engine.o: engine.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c engine.cpp -o $(OBJDIR_RELEASE)/engine.o

$(OBJDIR_RELEASE)/camera.o: camera.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c camera.cpp -o $(OBJDIR_RELEASE)/camera.o

$(OBJDIR_RELEASE)/light.o: light.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c light.cpp -o $(OBJDIR_RELEASE)/light.o

$(OBJDIR_RELEASE)/mesh.o: mesh.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c mesh.cpp -o $(OBJDIR_RELEASE)/mesh.o

$(OBJDIR_RELEASE)/node.o: node.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c node.cpp -o $(OBJDIR_RELEASE)/node.o

$(OBJDIR_RELEASE)/loader.o: loader.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c loader.cpp -o $(OBJDIR_RELEASE)/loader.o

$(OBJDIR_RELEASE)/texture.o: texture.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c texture.cpp -o $(OBJDIR_RELEASE)/texture.o

clean_release:
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release
