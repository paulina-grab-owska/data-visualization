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

const GLchar* vertexSource = R"glsl(
#version 150 core

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal; // Przemiana normalnych
    TexCoord = texCoord;
    gl_Position = proj * view * vec4(FragPos, 1.0);
}
)glsl";

const GLchar* fragmentSource = R"glsl(
#version 150 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

out vec4 outColor;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 ambientLightColor;
uniform vec3 diffuseLightColor;
uniform bool lightingEnabled;  // światło on/off
uniform float lightIntensity;  // intensywność światła

void main() {
    vec3 resultColor = vec3(0.0);

    if (lightingEnabled) {

        // Oświetlenie otoczenia
        float ambientStrength = 0.1 * lightIntensity;  //zmiana intensywności
        vec3 ambient = ambientStrength * ambientLightColor;

        // Oświetlenie rozproszone
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * diffuseLightColor * lightIntensity; //zwiększenie intesnyweności

        resultColor = ambient + diffuse;
    }

    // tektura + światło
    outColor = vec4(resultColor * vec3(texture(texture1, TexCoord)), 1.0);
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



glm::vec3 lightPos(1.2f, 1.0f, 2.0f); // pozycja światła
glm::vec3 ambientLightColor(1.0f);    // światło otoczenia
glm::vec3 diffuseLightColor(1.0f);    // światło rozproszone



int main() {

    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;

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

    // wektor normalnych do wierzchołków
    GLint NorAttrib = glGetAttribLocation(shaderProgram, "normal");
    glEnableVertexAttribArray(NorAttrib);
    glVertexAttribPointer(NorAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // shader: ustalenie pozycji światła
    GLint uniLightPos = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3fv(uniLightPos, 1, &lightPos[0]);

    // shader: ustalenie koloru światła otoczenia
    GLint uniAmbientLight = glGetUniformLocation(shaderProgram, "ambientLightColor");
    glUniform3fv(uniAmbientLight, 1, &ambientLightColor[0]);


    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    sf::Clock clock;

    bool lightingEnabled = true;  // oświetlenie włączone domyślnie
    float lightIntensity = 1.0f;  // początkowa intensywność oświetlenia


    while (window.isOpen()) {
        float currentFrame = clock.getElapsedTime().asSeconds();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Num1) {
                        lightingEnabled = !lightingEnabled;
                        if (lightingEnabled) {
                            lightIntensity = 0.0f;
                        }
                        
                    }
                    else if (event.key.code == sf::Keyboard::Num2) {
                        lightingEnabled = !lightingEnabled;
                        if (lightingEnabled) {
                            lightIntensity = 2.0f;
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Num3) {
                        lightingEnabled = !lightingEnabled;
                        if (lightingEnabled) {
                            lightIntensity = 4.0f;
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Num4) {
                        lightingEnabled = !lightingEnabled;
                        if (lightingEnabled) {
                            lightIntensity = 7.0f;
                        }
                    }
            }
        }

        // przekazanie światła do shadera
        GLint lightingLocation = glGetUniformLocation(shaderProgram, "lightingEnabled");
        glUniform1i(lightingLocation, lightingEnabled);
        GLint lightIntensityLocation = glGetUniformLocation(shaderProgram, "lightIntensity");
        glUniform1f(lightIntensityLocation, lightIntensity);

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
