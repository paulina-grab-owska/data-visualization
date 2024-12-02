// Nagłówki
#include <iostream>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Kody shaderów
const GLchar* vertexSource = R"glsl(
#version 150 core
in vec3 position;
in vec3 color;
in vec2 texCoord;
out vec3 Color;
out vec2 TexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    Color = color;
    TexCoord = texCoord;
    gl_Position = proj * view * model * vec4(position, 1.0);
}
)glsl";

const GLchar* fragmentSource = R"glsl(
#version 150 core
in vec2 TexCoord;
out vec4 outColor;
uniform sampler2D texture1;

void main() {
    outColor = texture(texture1, TexCoord);
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

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float yaw = -90.0f;
float pitch = 00.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;

int main() {
    // Ustawienia okna i kontekstu
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;

    // Okno renderingu i Inicjalizacja GLEW
    sf::Window window(sf::VideoMode(800, 600), "texture", sf::Style::Titlebar | sf::Style::Close, settings);
    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    float vertices[] = {

        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,     0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,     0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,     0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,     1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,     1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,     1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,     0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,     0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,     1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 1.0f,     1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.5f, 0.5f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.5f, 0.5f,     0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.5f, 0.0f, 0.5f,     0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,     1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.5f,     1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.5f, 0.0f, 1.0f,     1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,     0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,     0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.5f, 1.0f, 0.0f,     0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.5f,     1.0f, 0.0f,

         -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,  0.5f, 0.0f, 0.5f,    1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.5f, 0.0f,    1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.5f, 0.0f,    1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.5f, 1.0f, 0.5f,    0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,    0.0f, 1.0f,

         -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 1.0f,    0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.5f,    1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,    1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,    1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.5f,    0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 1.0f,    0.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompile(vertexShader, "vertexShader");

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompile(fragmentShader, "fragmentShader");

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texCoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("image2.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << stbi_failure_reason() << std::endl;
    }

    stbi_image_free(data);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    sf::Clock clock;

    while (window.isOpen()) {
        float currentFrame = clock.getElapsedTime().asSeconds();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, texture);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        GLint uniView = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(currentFrame * 50.0f), glm::vec3(1.0f, 0.3f, 0.5f)); 
        GLint uniModel = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

        glDrawArrays(GL_TRIANGLES, 0, 36);

        window.display();
    }

    glDeleteTextures(1, &texture);
    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    return 0;
}
