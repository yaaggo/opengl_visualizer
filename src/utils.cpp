#include <GL/freeglut.h>
#include <math.h>

// Função para checar a colisão entre dois objetos
bool collision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// Desenha uma string na posição (x, y) determinada com a fonte escolhida
void draw_text(float x, float y, void *font, const unsigned char* text) {
    glRasterPos2f(x, y);
    glutBitmapString(font, text);
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