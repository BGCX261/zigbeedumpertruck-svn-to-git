CXXFLAGS=`sdl-config --cflags` -g -W -Wall
LDFLAGS=`sdl-config --libs` -lSDL_ttf
OBJECTS=gui_core.o rs232.o
EXECUTABLE=main
all: main
main: $(OBJECTS) main.o
	$(CXX) $(LDFLAGS) $(OBJECTS) main.o -o $(EXECUTABLE)

clean:
	rm -f $(OBJECTS) main.o
	rm -f $(EXECUTABLE)
