# LEFT makefile

CC=g++
DBGCFLAGS=-g -D_DEBUG -c -Isrc/include
CFLAGS=-O2 -c -Isrc/include
LDFLAGS=-lrt -lboost_system -lpng -lXext -lX11 -lXxf86vm -lXmu -lXi -lGL -lGLU -lglut -lm

SOURCES=	src/main.cpp \
		src/world/Map.cpp \
		src/world/Projectile.cpp \
		src/world/RobotModel.cpp \
		src/world/RobotRocketEffect.cpp \
		src/world/RobotStabilizeEffect.cpp \
		src/world/BFGEffect.cpp
			
GLSOURCES=	src/gl/clipper.cpp \
		src/gl/GLFont.cpp \
		src/gl/GLParticle.cpp \
		src/gl/GLParticleEffect.cpp \
		src/gl/GLResources.cpp \
		src/gl/GLShader.cpp \
		src/gl/GLSprite.cpp \
		src/gl/GLUtil.cpp \
		src/gl/GLWindow.cpp
			

ALSOURCES=	src/al/aldlist.cpp \
		src/al/LoadOAL.cpp \
		src/al/SoundPlayer.cpp

OBJECTS=$(patsubst %.cpp,.obj/%.o,$(SOURCES) $(GLSOURCES))
DBGOBJECTS=$(patsubst %.cpp,.obj/%.dbg.o,$(SOURCES) $(GLSOURCES))

TARGET=left
DBGTARGET=dleft

all: $(SOURCES) $(TARGET) $(DBGTARGET)
	
$(TARGET): $(OBJECTS) 
	@echo + ld $@
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
$(DBGTARGET): $(DBGOBJECTS)
	@echo + ld $@
	@$(CC) $(DBGOBJECTS) $(LDFLAGS) -o $@

.obj/%.o: %.cpp
	@echo + cc $<
	@$(CC) $(CFLAGS) $< -o $@
.obj/%.dbg.o: %.cpp
	@echo + cc $<
	@$(CC) $(DBGCFLAGS) $< -o $@	

clean:
	@rm -rf .obj
	@mkdir -p .obj
	@mkdir -p .obj/src
	@mkdir -p .obj/src/gl
	@mkdir -p .obj/src/world

run: $(TARGET)
	@rm -f ~/$(TARGET)
	@cp $(TARGET) ~/
	@~/$(TARGET) &

debug: $(DBGTARGET)
	@rm -f ~/$(DBGTARGET)
	@cp $(DBGTARGET) ~/
	@gdb ~/$(DBGTARGET)

