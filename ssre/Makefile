SRCS := src/main.cpp \
	src/ssre_rasterize.cpp \
	src/ssre_sdl.cpp \
	src/ssre_math.cpp \
	src/ssre_view.cpp \
	src/ssre_removal.cpp \
	src/ssre_buffer.cpp \
	src/ssre_lighting.cpp \
	src/ssre_texture.cpp
OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
DEPS := $(OBJS:.o=.d)
TARGET := ssre

CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -fexceptions -fPIC -std=c++11
LIBS := -lSDL2 -lSDL2_image
INCLUDES := -I ./include

ifeq ($(RELEASE), 1)
CXXFLAGS += -O3
else
CXXFLAGS += -g3 -O0
DEFINES = -DDEBUG
endif

.PHONY: all clean run check

all : $(TARGET)

clean :
	rm -f $(TARGET)
	rm -f $(OBJS)
	rm -r $(DEPS)

run : $(TARGET)
	./$(TARGET)

check : $(SRCS)
	cppcheck --enable=all --suppress=missingIncludeSystem $(INCLUDES) $(SRCS)

include $(DEPS)

$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

$(DEPS) : %.d : %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM -MT "$*.o $@" $(INCLUDES) $< > $@

$(OBJS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -o $@ -c $< 

