#include <GL/freeglut.h>

void menu_display() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 450.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.216f, 0.145f, 0.286f, 1.0f); // Cor de fundo
    glClear(GL_COLOR_BUFFER_BIT);

    /*
    // Grade para ajudar a verificar centralização
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
        glVertex2f(0.0f, 225.0f);
        glVertex2f(800.0f, 225.0f);
        glVertex2f(400.0f, 0.0f);
        glVertex2f(400.0f, 450.0f);
    glEnd();
    glPopMatrix();
    */

    // Título
    glPushMatrix();
    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(4.0f);
    glTranslatef(((800.0f-((float)glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)"Simulador OpenGL")*0.4f))/2.0f), 392.38f, 0.0f);
    glScalef(0.4f,0.4f,0.4f);
    glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*)"Simulador OpenGL");
    glPopMatrix();

    // Créditos
    glPushMatrix();
    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(1.0f);
    glTranslatef(10.0f, 10.0f, 0.0f);
    glScalef(0.1f,0.1f,0.1f);
    glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*)"Feito por: Gabriel Coelho, Rafael Emanuel, Lucas Ferreira e Yago Guirra.");
    glPopMatrix();

    glutSwapBuffers();
}