#include <GL/freeglut.h>
#include <string>
#include <math.h>

// Função para checar a colisão entre dois objetos
bool collision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// Desenha uma string na posição (x, y) determinada com a fonte escolhida
void draw_text(float x, float y, void *font, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

// Função auxiliar para desenhar círculos
void draw_circle(float cx, float cy, float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy); // Centro do círculo
        for (int i = 0; i <= segments; i++) {
            float theta = 2.0f * 3.1415926f * (float)i / (float)segments;
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);
            glVertex2f(cx + x, cy + y);
        }
    glEnd();
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

GLuint load_texture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}