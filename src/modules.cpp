#include <GL/freeglut.h>
#include "modules.h"
#include "menu.h"
#include "bezier.h"

// Variável para alternar entre os módulos
module_type current_module = BEZIER;
int viewport_width, viewport_height, viewport_x = 0, viewport_y = 0;

// Funções de callback do glut por módulo

void display_callback() {
    switch(current_module) {
        case MENU:
            menu_display();
            break;
        case BEZIER:
            bezier_display();
            break;
    }
}

void mouse_callback(int button, int state, int x, int y) {
    switch(current_module) {
        case MENU:
            break;
        case BEZIER:
            bezier_mouse(button, state, x, y);
            break;
    }
}

void motion_callback(int x, int y) {
    switch(current_module) {
        case MENU:
            break;
        case BEZIER:
            bezier_motion(x, y);
            break;
    }
}

void keyboard_callback(unsigned char key, int x, int y) {
    switch(current_module) {
        case MENU:
            break;
        case BEZIER:
            bezier_keyboard(key);
            break;
    }
}

// Função de callback para reajustar a tela mantendo uma proporção 16:9
void reshape_callback(int width, int height) {
    // Proporção desejada
    float target_aspect_ratio = 16.0f / 9.0f;
    // Proporção atual da tela
    float window_aspect_ratio = (float)width / (float)height;

    if (height == 0) height = 1; // Impede divisões por 0

    // Se a proporção da tela for maior que a proporção desejada:
    // Define a altura do viewport como a altura da tela e encontra a largura com base na proporção
    // Se a proporção da tela for menor que a proporção desejada:
    // Define a largura do viewport como a largura da tela e encontra a altura com base na proporção
    if (window_aspect_ratio > target_aspect_ratio) {
        viewport_height = height;
        viewport_width = (int)(height * target_aspect_ratio);
        viewport_x = (width - viewport_width) / 2;
    } else {
        viewport_width = width;
        viewport_height = (int)(width / target_aspect_ratio);
        viewport_y = (height - viewport_height) / 2;
    }

    // Posiciona a viewport no centro da tela mantendo a proporção
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    glutPostRedisplay();
}