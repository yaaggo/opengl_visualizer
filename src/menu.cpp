#include <GL/freeglut.h>
#include <string>
#include "menu.h"
#include "modules.h"
#include "utils.h"

static int menu_mouse_x = 0;
static int menu_mouse_y = 0;
static int menu_click_state = -1;

static void draw_stroke_text_centered(float center_x, float center_y, float scale, float line_width, const std::string& text) {
    glPushMatrix();
    glLineWidth(line_width);
    float text_w = (float)glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)text.c_str()) * scale;
    float start_x = center_x - text_w / 2.0f;
    float start_y = center_y - (119.0f * scale / 2.0f);
    glTranslatef(start_x, start_y, 0.0f);
    glScalef(scale, scale, scale);
    for (char c : text) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    }
    glPopMatrix();
    glLineWidth(1.0f);
}

void menu_display() {
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 450.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.216f, 0.145f, 0.286f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    float bezier_y1 = 260.0f;
    float bezier_y2 = 310.0f;
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

    draw_stroke_text_centered(b_center_x, 285.0f, 0.18f, 2.0f, "BEZIER");

    float vis_y1 = 180.0f;
    float vis_y2 = 230.0f;
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

    draw_stroke_text_centered(b_center_x, 205.0f, 0.18f, 2.0f, "VISUALIZER");

    float proj_y1 = 100.0f;
    float proj_y2 = 150.0f;
    bool hover_proj = (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= proj_y1 && menu_mouse_y <= proj_y2);

    if (hover_proj) {
        if (menu_click_state == GLUT_DOWN) {
            glColor3f(0.18f, 0.12f, 0.24f);
        } else {
            glColor3f(0.353f, 0.243f, 0.459f);
        }
    } else {
        glColor3f(0.282f, 0.192f, 0.369f);
    }
    glBegin(GL_QUADS);
    glVertex2f(b_x1, proj_y1);
    glVertex2f(b_x2, proj_y1);
    glVertex2f(b_x2, proj_y2);
    glVertex2f(b_x1, proj_y2);
    glEnd();

    if (hover_proj) {
        glColor3f(1.0f, 0.9f, 0.85f);
    } else {
        glColor3f(0.918f, 0.804f, 0.761f);
    }
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(b_x1, proj_y1);
    glVertex2f(b_x2, proj_y1);
    glVertex2f(b_x2, proj_y2);
    glVertex2f(b_x1, proj_y2);
    glEnd();
    glLineWidth(1.0f);

    draw_stroke_text_centered(b_center_x, 125.0f, 0.18f, 2.0f, "PROJECTIONS");

    glPushMatrix();
    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(1.0f);
    glTranslatef(10.0f, 10.0f, 0.0f);
    glScalef(0.1f, 0.1f, 0.1f);
    glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*)"Feito por: Gabriel Coelho, Pedro Lucas, Rafael Emanuel e Yago Guirra.");
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

        float bezier_y1 = 260.0f;
        float bezier_y2 = 310.0f;
        float vis_y1 = 180.0f;
        float vis_y2 = 230.0f;
        float proj_y1 = 100.0f;
        float proj_y2 = 150.0f;

        if (state == GLUT_UP) {
            if (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= bezier_y1 && menu_mouse_y <= bezier_y2) {
                current_module = BEZIER;
            } else if (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= vis_y1 && menu_mouse_y <= vis_y2) {
                current_module = VISUALIZER;
            } else if (menu_mouse_x >= b_x1 && menu_mouse_x <= b_x2 && menu_mouse_y >= proj_y1 && menu_mouse_y <= proj_y2) {
                current_module = PROJECTIONS;
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
