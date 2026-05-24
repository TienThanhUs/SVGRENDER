CXX = g++
CXXFLAGS = -std=c++20
SFML_LIBS = -I/opt/homebrew/Cellar/sfml/3.0.2/include \
-L/opt/homebrew/Cellar/sfml/3.0.2/lib \
-L/opt/homebrew/Cellar/sfml@2/2.6.2_1/lib \
 -lsfml-graphics -lsfml-window -lsfml-system
SOURCES = Base.cpp ShapeReader.cpp main.cpp ShapeRenderer.cpp SVGParse.cpp SVGRender.cpp
TARGET = svg_reader

all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(SFML_LIBS)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)

.PHONY: all clean run
