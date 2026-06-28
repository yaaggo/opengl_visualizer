#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include "utils.h"
#include "modules.h"

extern module_type current_module;
extern int viewport_width, viewport_height, viewport_x, viewport_y; 

struct Point2D {
    float x, y;
};

GLfloat control_points[4][3] = {
    {200.0f, 400.0f, 0.0f},
    {500.0f, 150.0f, 0.0f},
    {1100.0f, 150.0f, 0.0f},
    {1400.0f, 400.0f, 0.0f}
};

int selected_point = -1;
const float SELECTION_RADIUS = 15.0f;

void draw_container(float x1, float y1, float x2, float y2) {
    glColor3f(0.169f, 0.106f, 0.231f);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();

    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
    glLineWidth(1.0f);
}

void bezier_display() {
    glClearColor(0.216f, 0.145f, 0.286f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1600, 900, 0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(50.0f, 35.0f, GLUT_BITMAP_HELVETICA_18, "Curva de Bezier - Arraste os pontos para alterar a curva");
    draw_text(1420.0f, 35.0f, GLUT_BITMAP_HELVETICA_18, "ESC para voltar");

    draw_container(50.0f, 50.0f, 1550.0f, 530.0f);
    draw_container(50.0f, 570.0f, 1550.0f, 850.0f);

    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x00FF);
    glColor3f(0.5f, 0.5f, 0.6f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 4; i++) {
        glVertex2f(control_points[i][0], control_points[i][1]);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(3.0f);
    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &control_points[0][0]);
    glEnable(GL_MAP1_VERTEX_3);
    glBegin(GL_LINE_STRIP);
    for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
        glEvalCoord1f((GLfloat)t / 1.0);
    }
    glEnd();
    glDisable(GL_MAP1_VERTEX_3);
    glLineWidth(1.0f);

    glPointSize(12.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 4; i++) {
        if (i == selected_point) {
            glColor3f(1.0f, 0.5f, 0.5f);
        } else {
            glColor3f(0.918f, 0.804f, 0.761f);
        }
        glVertex2f(control_points[i][0], control_points[i][1]);
    }
    glEnd();

    glColor3f(0.918f, 0.804f, 0.761f);
    
    draw_text(80.0f, 605.0f, GLUT_BITMAP_HELVETICA_18, "GLfloat control_points[4][3] = {");
    
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "    { %.1ff, %.1ff, 0.0f },", control_points[0][0], control_points[0][1]);
    draw_text(80.0f, 637.0f, GLUT_BITMAP_HELVETICA_18, buffer);
    
    snprintf(buffer, sizeof(buffer), "    { %.1ff, %.1ff, 0.0f },", control_points[1][0], control_points[1][1]);
    draw_text(80.0f, 669.0f, GLUT_BITMAP_HELVETICA_18, buffer);
    
    snprintf(buffer, sizeof(buffer), "    { %.1ff, %.1ff, 0.0f },", control_points[2][0], control_points[2][1]);
    draw_text(80.0f, 701.0f, GLUT_BITMAP_HELVETICA_18, buffer);
    
    snprintf(buffer, sizeof(buffer), "    { %.1ff, %.1ff, 0.0f }", control_points[3][0], control_points[3][1]);
    draw_text(80.0f, 733.0f, GLUT_BITMAP_HELVETICA_18, buffer);
    
    draw_text(80.0f, 765.0f, GLUT_BITMAP_HELVETICA_18, "};");

    draw_text(750.0f, 605.0f, GLUT_BITMAP_HELVETICA_18, "glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &control_points[0][0]);");
    draw_text(750.0f, 637.0f, GLUT_BITMAP_HELVETICA_18, "glEnable(GL_MAP1_VERTEX_3);");
    draw_text(750.0f, 669.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_LINE_STRIP);");
    draw_text(750.0f, 701.0f, GLUT_BITMAP_HELVETICA_18, "for (float t = 0.0f; t <= 1.0f; t += 0.01f) {");
    draw_text(750.0f, 733.0f, GLUT_BITMAP_HELVETICA_18, "    glEvalCoord1f(t);");
    draw_text(750.0f, 765.0f, GLUT_BITMAP_HELVETICA_18, "}");
    draw_text(750.0f, 797.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
    draw_text(750.0f, 829.0f, GLUT_BITMAP_HELVETICA_18, "glDisable(GL_MAP1_VERTEX_3);");

    glutSwapBuffers();
}

Point2D map_mouse_to_ortho(int x, int y) {
    int local_x = x - viewport_x;
    int local_y = y - viewport_y;
    Point2D mapped;
    mapped.x = ((float)local_x / viewport_width) * 1600.0f;
    mapped.y = ((float)local_y / viewport_height) * 900.0f;
    return mapped;
}

void bezier_mouse(int button, int state, int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            for (int i = 0; i < 4; i++) {
                float dx = mouse_pos.x - control_points[i][0];
                float dy = mouse_pos.y - control_points[i][1];
                if (std::sqrt(dx*dx + dy*dy) <= SELECTION_RADIUS * 2.0f) {
                    selected_point = i;
                    break;
                }
            }
        } else if (state == GLUT_UP) {
            selected_point = -1;
        }
    }
    glutPostRedisplay();
}

void bezier_motion(int x, int y) {
    if (selected_point != -1) {
        Point2D mouse_pos = map_mouse_to_ortho(x, y);

        if (mouse_pos.x < 50.0f + SELECTION_RADIUS) {
            mouse_pos.x = 50.0f + SELECTION_RADIUS;
        }
        if (mouse_pos.x > 1550.0f - SELECTION_RADIUS) {
            mouse_pos.x = 1550.0f - SELECTION_RADIUS;
        }
        if (mouse_pos.y < 50.0f + SELECTION_RADIUS) {
            mouse_pos.y = 50.0f + SELECTION_RADIUS;
        }
        if (mouse_pos.y > 530.0f - SELECTION_RADIUS) {
            mouse_pos.y = 530.0f - SELECTION_RADIUS;
        }

        control_points[selected_point][0] = mouse_pos.x;
        control_points[selected_point][1] = mouse_pos.y;
        glutPostRedisplay();
    }
}

void bezier_keyboard(unsigned char key) {
    if (key == 27) {
        current_module = MENU;
    }
    glutPostRedisplay();
}