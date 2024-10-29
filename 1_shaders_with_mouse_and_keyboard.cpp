
// Nagłówki
//#include "stdafx.h"
#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <cmath>


// Kody shaderów
const GLchar* vertexSource = R"glsl(
#version 150 core
in vec2 position;
in vec3 color;
out vec3 Color;
void main(){
Color = color;
gl_Position = vec4(position, 0.0, 1.0);
}
)glsl";

const GLchar* fragmentSource = R"glsl(
#version 150 core
in vec3 Color;
out vec4 outColor;
void main()
{
outColor = vec4(Color, 1.0);
}
)glsl";


// Funkcja do sprawdzania kompilacji shaderów
void checkShaderCompile(GLuint shader, const char* shaderName) {
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success) {
		std::cout << "Compilation " << shaderName << " OK\n";
	}
	else {
		GLchar infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "Compilation " << shaderName << " ERROR\n" << infoLog << std::endl;
	}
}

// Funkcja do sprawdzania linkowania programu
void checkProgramLink(GLuint program) {
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success) {
		std::cout << "Program linking OK\n";
	}
	else {
		GLchar infoLog[512];
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cerr << "Program linking ERROR\n" << infoLog << std::endl;
	}
}

// zmienne globalne 
int points_;
GLfloat* vertices = new GLfloat[points_ * 6];

GLenum drawPrimitive = GL_TRIANGLE_FAN; //domyślny prymityw


void updateVertices(GLuint vbo, int points_) {
	delete[] vertices;
	vertices = new GLfloat[points_ * 6];

	float radius = 0.5f;
	float angleStep = 2 * 3.14 / points_;

	for (int i = 0; i < points_; i++) {
		float angle = i * angleStep;
		vertices[i * 6 + 0] = radius * cos(angle); // X
		vertices[i * 6 + 1] = radius * sin(angle); // Y
		vertices[i * 6 + 2] = 0.0f;                // Z

		// RGB - kolory dla każdego wierzchołka
		vertices[i * 6 + 3] = (i % 2 == 0) ? 1.0f : 0.0f; // R
		vertices[i * 6 + 4] = (i % 3 == 0) ? 1.0f : 0.0f; // G
		vertices[i * 6 + 5] = (i % 4 == 0) ? 1.0f : 0.0f; // B
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points_ * 6, vertices, GL_STATIC_DRAW);
}


int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;

	// Okno renderingu
	sf::Window window(sf::VideoMode(800, 600, 32), "OpenGL", sf::Style::Titlebar | sf::Style::Close, settings);

	// Inicjalizacja GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	// Utworzenie VAO (Vertex Array Object)
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Utworzenie VBO (Vertex Buffer Object)
	// i skopiowanie do niego danych wierzchołkowych
	GLuint vbo;
	glGenBuffers(1, &vbo);

	// Przesłanie danych do pamięci karty graficznej
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points_ * 6, vertices, GL_STATIC_DRAW);

	// Kompilacja shaderów
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	checkShaderCompile(vertexShader, "vertexShader");

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	checkShaderCompile(fragmentShader, "fragmentShader");

	// Zlinkowanie programu
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	checkProgramLink(shaderProgram);
	glUseProgram(shaderProgram);

	// Specyfikacja formatu danych wierzchołkowych
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	// Pętla zdarzeń
	bool running = true;
	while (running) {
		sf::Event windowEvent;
		while (window.pollEvent(windowEvent)) {
			if (windowEvent.type == sf::Event::Closed) {
				running = false;
			}
			// obsługa ruchu myszki i liczby wierzchołków
			if (windowEvent.type == sf::Event::MouseMoved) {
				int mouseY = windowEvent.mouseMove.y;
				points_ = std::max(3, mouseY / 30); // minimum 3
				updateVertices(vbo, points_);
			}

			// obsługa klawiatury i prytmitywu
			if (windowEvent.type == sf::Event::KeyPressed) {
				switch (windowEvent.key.code) {
				case sf::Keyboard::Num1:
					drawPrimitive = GL_POINTS;
					break;
				case sf::Keyboard::Num2:
					drawPrimitive = GL_LINES;
					break;
				case sf::Keyboard::Num3:
					drawPrimitive = GL_LINE_STRIP;
					break;
				case sf::Keyboard::Num4:
					drawPrimitive = GL_LINE_LOOP;
					break;
				case sf::Keyboard::Num5:
					drawPrimitive = GL_TRIANGLES;
					break;
				case sf::Keyboard::Num6:
					drawPrimitive = GL_TRIANGLE_STRIP;
					break;
				case sf::Keyboard::Num7:
					drawPrimitive = GL_TRIANGLE_FAN;
					break;
				case sf::Keyboard::Num8:
					drawPrimitive = GL_QUADS;
					break;
				case sf::Keyboard::Num9:
					drawPrimitive = GL_QUAD_STRIP;
					break;
				case sf::Keyboard::Num0:
					drawPrimitive = GL_POLYGON;
					break;
				}
			}
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(drawPrimitive, 0, points_);

		window.display();
	}

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	delete[] vertices;

	window.close();
	return 0;


}
