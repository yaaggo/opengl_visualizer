#include <GL/glut.h>
#include <cstring>

struct app_config {
    int window_width;
    int window_height;
    const char* window_title;
    const char* text_message;
};

struct app_state {
    app_config config;
};

app_state g_app;

void draw_text(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    int length = (int)std::strlen(text);
    for (int i = 0; i < length; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void on_display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, g_app.config.window_width, 0.0, g_app.config.window_height);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    
    int text_width = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)g_app.config.text_message);
    float x = (g_app.config.window_width - text_width) / 2.0f;
    float y = (g_app.config.window_height - 18.0f) / 2.0f;
    
    draw_text(x, y, g_app.config.text_message);
    
    glutSwapBuffers();
}

void on_reshape(int width, int height) {
    g_app.config.window_width = width;
    g_app.config.window_height = height;
    glViewport(0, 0, width, height);
}

void init_app_state() {
    g_app.config.window_width = 800;
    g_app.config.window_height = 600;
    g_app.config.window_title = "opengl visualizer";
    g_app.config.text_message = "oi gabriel tudo bem";
}

int main(int argc, char** argv) {
    init_app_state();
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(g_app.config.window_width, g_app.config.window_height);
    glutCreateWindow(g_app.config.window_title);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    glutDisplayFunc(on_display);
    glutReshapeFunc(on_reshape);
    
    glutMainLoop();
    return 0;
}
