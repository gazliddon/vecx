
CPPFLAGS =  -c -std=c++14 -Wall \
			-Wextra -Wno-unused-parameter \
			-Wno-unused-variable \
			-Wno-unused-function \
			$(shell sdl2-config --cflags)

LIBS :=  $(shell sdl2-config --libs)
OBJECTS := e6809.o e8910.o osint.o vecx.o cscreen.o
TARGET := vecx
CLEANFILES := $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	g++ -o $@ $^ $(LIBS)

%.o : %.cpp
	g++ $(CPPFLAGS) $<

clean:
	$(RM) $(CLEANFILES)

