#include <GL/freeglut.h>
#include <string>
#include "visualizer.h"
#include "modules.h"
#include "utils.h"

void visualizer_display() {
    glClearColor(0.216f, 0.145f, 0.286f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 450.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(0.918f, 0.804f, 0.761f);
    
    std::string title = "MODULO VISUALIZADOR";
    std::string subtitle = "(EM BREVE)";
    std::string exit_msg = "Pressione ESC para voltar";
    
    int t_w = 0;
    for (char c : title) {
        t_w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
    }
    draw_text(400.0f - (t_w / 2.0f), 240.0f, GLUT_BITMAP_HELVETICA_18, title);
    
    int s_w = 0;
    for (char c : subtitle) {
        s_w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
    }
    draw_text(400.0f - (s_w / 2.0f), 200.0f, GLUT_BITMAP_HELVETICA_18, subtitle);
    
    int e_w = 0;
    for (char c : exit_msg) {
        e_w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
    }
    draw_text(400.0f - (e_w / 2.0f), 50.0f, GLUT_BITMAP_HELVETICA_18, exit_msg);
    
    glutSwapBuffers();
}

void visualizer_keyboard(unsigned char key) {
    if (key == 27) {
        current_module = MENU;
    }
}
