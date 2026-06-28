#include <GL/freeglut.h>
#include <string>
#include "menu.h"
#include "modules.h"
#include "utils.h"

static int menu_mouse_x = 0;
static int menu_mouse_y = 0;
static int menu_click_state = -1;

void menu_display() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 450.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.216f, 0.145f, 0.286f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();
    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(4.0f);
    float title_x = (800.0f - ((float)glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)"Simulador OpenGL") * 0.4f)) / 2.0f;
    glTranslatef(title_x, 350.0f, 0.0f);
    glScalef(0.4f, 0.4f, 0.4f);
    glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*)"Simulador OpenGL");
    glPopMatrix();

    float b_center_x = 400.0f;
    float b_width = 260.0f;
    float b_x1 = b_center_x - b_width / 2.0f;
    float b_x2 = b_center_x + b_width / 2.0f;

    float bezier_y1 = 215.0f;
    float bezier_y2 = 265.0f;
    bool hover_bezier = (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= bezier_y1 && menu_mouse_y <= bezier_y2);

    if (hover_bezier) {
        if (menu_click_state == GLUT_DOWN) {
            glColor3f(0.18f, 0.12f, 0.24f);
        } else {
            glColor3f(0.353f, 0.243f, 0.459f);
        }
    } else {
        glColor3f(0.282f, 0.192f, 0.369f);
    }
    glBegin(GL_QUADS);
    glVertex2f(b_x1, bezier_y1);
    glVertex2f(b_x2, bezier_y1);
    glVertex2f(b_x2, bezier_y2);
    glVertex2f(b_x1, bezier_y2);
    glEnd();

    if (hover_bezier) {
        glColor3f(1.0f, 0.9f, 0.85f);
    } else {
        glColor3f(0.918f, 0.804f, 0.761f);
    }
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(b_x1, bezier_y1);
    glVertex2f(b_x2, bezier_y1);
    glVertex2f(b_x2, bezier_y2);
    glVertex2f(b_x1, bezier_y2);
    glEnd();
    glLineWidth(1.0f);

    std::string bezier_label = "BEZIER";
    int bezier_w = 0;
    for (char c : bezier_label) {
        bezier_w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
    }
    draw_text(b_center_x - (bezier_w / 2.0f), 234.0f, GLUT_BITMAP_HELVETICA_18, bezier_label);

    float vis_y1 = 135.0f;
    float vis_y2 = 185.0f;
    bool hover_vis = (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= vis_y1 && menu_mouse_y <= vis_y2);

    if (hover_vis) {
        if (menu_click_state == GLUT_DOWN) {
            glColor3f(0.18f, 0.12f, 0.24f);
        } else {
            glColor3f(0.353f, 0.243f, 0.459f);
        }
    } else {
        glColor3f(0.282f, 0.192f, 0.369f);
    }
    glBegin(GL_QUADS);
    glVertex2f(b_x1, vis_y1);
    glVertex2f(b_x2, vis_y1);
    glVertex2f(b_x2, vis_y2);
    glVertex2f(b_x1, vis_y2);
    glEnd();

    if (hover_vis) {
        glColor3f(1.0f, 0.9f, 0.85f);
    } else {
        glColor3f(0.918f, 0.804f, 0.761f);
    }
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(b_x1, vis_y1);
    glVertex2f(b_x2, vis_y1);
    glVertex2f(b_x2, vis_y2);
    glVertex2f(b_x1, vis_y2);
    glEnd();
    glLineWidth(1.0f);

    std::string vis_label = "VISUALIZER";
    int vis_w = 0;
    for (char c : vis_label) {
        vis_w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
    }
    draw_text(b_center_x - (vis_w / 2.0f), 154.0f, GLUT_BITMAP_HELVETICA_18, vis_label);

    glPushMatrix();
    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(1.0f);
    glTranslatef(10.0f, 10.0f, 0.0f);
    glScalef(0.1f, 0.1f, 0.1f);
    glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*)"Feito por: Gabriel Coelho, Rafael Emanuel, Lucas Ferreira e Yago Guirra.");
    glPopMatrix();

    glutSwapBuffers();
}

void menu_mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        menu_click_state = state;
        float local_x = x - viewport_x;
        float local_y = y - viewport_y;
        menu_mouse_x = (local_x / viewport_width) * 800.0f;
        menu_mouse_y = (1.0f - local_y / viewport_height) * 450.0f;

        float b_center_x = 400.0f;
        float b_width = 260.0f;
        float b_x1 = b_center_x - b_width / 2.0f;
        float b_x2 = b_center_x + b_width / 2.0f;

        float bezier_y1 = 215.0f;
        float bezier_y2 = 265.0f;
        float vis_y1 = 135.0f;
        float vis_y2 = 185.0f;

        if (state == GLUT_UP) {
            if (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= bezier_y1 && menu_mouse_y <= bezier_y2) {
                current_module = BEZIER;
            } else if (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= vis_y1 && menu_mouse_y <= vis_y2) {
                current_module = VISUALIZER;
            }
        }
    }
    glutPostRedisplay();
}

void menu_motion(int x, int y) {
    float local_x = x - viewport_x;
    float local_y = y - viewport_y;
    menu_mouse_x = (local_x / viewport_width) * 800.0f;
    menu_mouse_y = (1.0f - local_y / viewport_height) * 450.0f;
    glutPostRedisplay();
}