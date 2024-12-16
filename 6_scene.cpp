#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct Vertex {
    float x, y, z;    
    float u, v;       
};

struct Face {
    std::vector<int> vertexIndices;
    std::vector<int> textureIndices;
};

struct Model {
    std::vector<Vertex> vertices;
    std::vector<glm::vec2> textureCoords;
    std::vector<Face> faces;
};

Model loadOBJ(const std::string& filename) {
    Model model;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return model;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            Vertex vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            model.vertices.push_back(vertex);
        }
        else if (prefix == "vt") {
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            model.textureCoords.push_back(texCoord);
        }
        else if (prefix == "f") {
            Face face;
            std::string vertexInfo;
            while (iss >> vertexInfo) {
                std::istringstream vertexStream(vertexInfo);
                int vertexIndex, textureIndex;
                char slash;
                vertexStream >> vertexIndex >> slash >> textureIndex;
                face.vertexIndices.push_back(vertexIndex - 1);
                face.textureIndices.push_back(textureIndex - 1);
            }
            model.faces.push_back(face);
        }
    }
    file.close();

   
    for (auto& face : model.faces) {
        for (size_t i = 0; i < face.vertexIndices.size(); ++i) {
            int vertexIdx = face.vertexIndices[i];
            int texIdx = face.textureIndices[i];

            model.vertices[vertexIdx].u = model.textureCoords[texIdx].x;
            model.vertices[vertexIdx].v = model.textureCoords[texIdx].y;
        }
    }

    return model;
}

GLuint loadTexture(const std::string& filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texture;
}

const char* vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

in vec2 TexCoord;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform int lightingEnabled;
uniform float lightIntensity;

void main() {
    vec3 color = texture(texture1, TexCoord).rgb;
    if (lightingEnabled == 1) {
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(lightDir, normalize(FragPos)), 0.0) * lightIntensity;
 
        vec3 result = color * (0.1 + diff);
        FragColor = vec4(result, 1.0);
    } else {
        FragColor = vec4(color, 1.0);
    }
}
)";


void drawModel(const Model& model, GLuint shaderProgram, GLuint texture) {
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, texture);

    std::vector<float> vertices;
    for (const auto& vertex : model.vertices) {
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);
        vertices.push_back(vertex.u);
        vertices.push_back(vertex.v);
    }

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    std::vector<unsigned int> indices;
    for (const auto& face : model.faces) {
        for (int index : face.vertexIndices) {
            indices.push_back(index);
        }
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

GLuint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    return shader;
}

GLuint createProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


int main() {
    sf::Window window(sf::VideoMode(800, 600), "OBJ Viewer Scene", sf::Style::Default, sf::ContextSettings(24));

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    GLuint shaderProgram = createProgram(vertexShaderSource, fragmentShaderSource);

    Model tableModel = loadOBJ("table.obj");
    Model chairModel = loadOBJ("chair.obj");

    GLuint texture = loadTexture("image2.png");

    glm::mat4 modelMat = glm::mat4(1.0f);
    glm::vec3 viewPosition(0.0f, 0.0f, 5.0f);
    glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glm::vec3 lightPosition(0.0f, 1000.0f, 1000.0f);
    bool lightingEnabled = true;
    float lightIntensity = 2.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            viewPosition.z -= 0.1f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            viewPosition.z += 0.1f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            viewPosition.x -= 0.1f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            viewPosition.x += 0.1f;
        }
  
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0)) {
            lightingEnabled = !lightingEnabled;
            if (!lightingEnabled) {
                lightIntensity = 0.0f;  
            }
        }

        if (lightingEnabled) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                lightIntensity = 0.0f; 
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                lightIntensity = 5.0f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                lightIntensity = 10.0f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
                lightIntensity = 40.0f;
            }
        }

        glm::mat4 viewMat = glm::lookAt(viewPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        GLuint lightingEnabledLoc = glGetUniformLocation(shaderProgram, "lightingEnabled");
        GLuint lightIntensityLoc = glGetUniformLocation(shaderProgram, "lightIntensity");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMat));
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPosition));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPosition));
        glUniform1i(lightingEnabledLoc, lightingEnabled ? 1 : 0);
        glUniform1f(lightIntensityLoc, lightIntensity);

        // Rysowanie stołu
        glm::mat4 tableTransform = glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tableTransform));
        drawModel(tableModel, shaderProgram, texture);

        // Rysowanie krzesła
        glm::mat4 chairTransform = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(chairTransform));      
        drawModel(chairModel, shaderProgram, texture);

        window.display();
    }

    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture);

    return 0;
}
