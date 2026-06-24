#include <GL/freeglut.h>
#include "menu.h"

// Variável para alternar entre os módulos
typedef enum {MENU, BEZIER} module_type;
module_type current_module = MENU;

struct app_config {
    int window_width;
    int window_height;
    const char* window_title;
};

struct app_state {
    app_config config;
};

app_state g_app;

void init_app_state() {
    g_app.config.window_width = 800;
    g_app.config.window_height = 450;
    g_app.config.window_title = "OpenGL Visualizer";
}

// Função de callback para as funções de renderização
void display_callback() {
    switch(current_module) {
        case MENU:
            menu_display();
            break;
        case BEZIER:
            break;
    }
}

// Função de callback para reajustar a tela mantendo uma proporção 16:9
void reshape_callback(int width, int height) {
    // Proporção desejada
    float target_aspect_ratio = 16.0f / 9.0f;
    // Proporção atual da tela
    float window_aspect_ratio = (float)width / (float)height;
    // Largura, altura, posição x e posição y da viewport
    int viewport_width, viewport_height, viewport_x = 0, viewport_y = 0;

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


int main(int argc, char** argv) {
    init_app_state();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(g_app.config.window_width, g_app.config.window_height);
    glutCreateWindow(g_app.config.window_title);

    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    
    glutMainLoop();
    return 0;
}
