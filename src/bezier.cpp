#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include "utils.h"
#include "modules.h"

// Pega as variáveis do viewport configuradas no reshape
extern module_type current_module;
extern int viewport_width, viewport_height, viewport_x, viewport_y; 

struct Point2D {
    float x, y;
};

// 4 pontos de controle
GLfloat control_points[4][3] = {
    {500.0f, 700.0f, 0.0f}, // P0
    {700.0f, 200.0f, 0.0f}, // P1
    {1100.0f, 800.0f, 0.0f},// P2
    {1400.0f, 300.0f, 0.0f} // P3
};

int selected_point = -1; // Índice do ponto sendo arrastado (-1 = nenhum)
const float SELECTION_RADIUS = 15.0f; // Raio para conseguir selecionar o ponto de controle

void bezier_display() {
    // Limpa a tela com a cor roxo escuro
    glClearColor(0.216f, 0.145f, 0.286f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1600, 900, 0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Cor do texto e da linha divisória
    glColor3f(0.918f, 0.804f, 0.761f); 

    // Linha divisória
    glBegin(GL_LINES);
        glVertex2f(450, 0);
        glVertex2f(450, 900);
    glEnd();

    draw_text(30, 80, GLUT_BITMAP_HELVETICA_18, "glMap1f(GL_MAP1_VERTEX_3,");
    draw_text(30, 120, GLUT_BITMAP_HELVETICA_18, "0.0, 1.0, 3, 4,");
    draw_text(30, 160, GLUT_BITMAP_HELVETICA_18, "&control_points[0][0]);");
    draw_text(30, 200, GLUT_BITMAP_HELVETICA_18, "glEnable(GL_MAP1_VERTEX_3);");
    draw_text(30, 240, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_LINE_STRIP);");
    draw_text(30, 280, GLUT_BITMAP_HELVETICA_18, "for (float t = 0.0f; t <= 1.0f; t += 0.01f) {");
    draw_text(30, 320, GLUT_BITMAP_HELVETICA_18, "    glEvalCoord1f((GLfloat)t / 1.0);");
    draw_text(30, 360, GLUT_BITMAP_HELVETICA_18, "}");
    draw_text(30, 400, GLUT_BITMAP_HELVETICA_18, "glEnd();");
    draw_text(30, 440, GLUT_BITMAP_HELVETICA_18, "glDisable(GL_MAP1_VERTEX_3);");

    // Renderiza o texto do código na tela com os valores atuais
    draw_text(30, 520, GLUT_BITMAP_HELVETICA_18, "PONTOS DE CONTROLE ATUAIS:");
    char buffer[100];
    for (int i = 0; i < 4; i++) {
        snprintf(buffer, sizeof(buffer), "P%d = { %d, %d }", i, (int)control_points[i][0], (int)control_points[i][1]);
        draw_text(30, 560 + (i * 40), GLUT_BITMAP_HELVETICA_18, buffer);
    }

    draw_text(30, 860, GLUT_BITMAP_HELVETICA_18, "Aperte ESC para voltar!");

    // --- RENDERIZAÇÃO DA CURVA ---
    
    // Desenha o polígono de controle (linhas tracejadas)
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x00FF);
    glColor3f(0.5f, 0.5f, 0.6f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 4; i++) {
        glVertex2f(control_points[i][0], control_points[i][1]);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // Desenha a Curva de Bézier
    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(3.0f);

    // Configura a curva Bézier
    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &control_points[0][0]);
    glEnable(GL_MAP1_VERTEX_3);

    glBegin(GL_LINE_STRIP);
    for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
        glEvalCoord1f((GLfloat)t / 1.0);
    }
    glEnd();
    glDisable(GL_MAP1_VERTEX_3);
    glLineWidth(1.0f);

    // Desenha quadrados nos pontos de controle
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 4; i++) {
        // Se estiver selecionado, muda levemente a cor para dar feedback
        if (i == selected_point) glColor3f(1.0f, 0.5f, 0.5f);
        else glColor3f(0.918f, 0.804f, 0.761f);
        glVertex2f(control_points[i][0], control_points[i][1]);
    }
    glEnd();

    glutSwapBuffers();
}

// Converte a coordenada do mouse da janela para o nosso espaço 1600x900
Point2D map_mouse_to_ortho(int x, int y) {
    // 1. Remove o deslocamento da centralização da viewport
    int local_x = x - viewport_x;
    int local_y = y - viewport_y;

    // 2. Transforma a proporção da viewport para 1600x900
    Point2D mapped;
    mapped.x = ((float)local_x / viewport_width) * 1600.0f;
    mapped.y = ((float)local_y / viewport_height) * 900.0f;
    
    return mapped;
}

void bezier_mouse(int button, int state, int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // Verifica se clicou perto de algum ponto
            for (int i = 0; i < 4; i++) {
                float dx = mouse_pos.x - control_points[i][0];
                float dy = mouse_pos.y - control_points[i][1];
                if (std::sqrt(dx*dx + dy*dy) <= SELECTION_RADIUS * 2.0f) {
                    selected_point = i;
                    break;
                }
            }
        } else if (state == GLUT_UP) {
            selected_point = -1; // Soltou o clique
        }
    }
    glutPostRedisplay();
}

void bezier_motion(int x, int y) {
    if (selected_point != -1) {
        Point2D mouse_pos = map_mouse_to_ortho(x, y);
        
        // Limita o eixo x para não chegar nos textos
        if (mouse_pos.x < 460.0f) mouse_pos.x = 460.0f;
        
        control_points[selected_point][0] = mouse_pos.x;
        control_points[selected_point][1] = mouse_pos.y;
        glutPostRedisplay(); // Atualiza a tela
    }
}

void bezier_keyboard(unsigned char key) {
    switch (key)
    {
    case 27:
        current_module = MENU;
    }
    glutPostRedisplay();
}