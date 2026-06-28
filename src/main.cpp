#include <GL/freeglut.h>
#include "modules.h"

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
    g_app.config.window_width = 1280;
    g_app.config.window_height = 720;
    g_app.config.window_title = "OpenGL Visualizer";
}

int main(int argc, char** argv) {
    init_app_state();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(g_app.config.window_width, g_app.config.window_height);
    glutCreateWindow(g_app.config.window_title);
    glutFullScreen();

    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    glutMouseFunc(mouse_callback);
    glutMotionFunc(motion_callback);
    glutPassiveMotionFunc(passive_motion_callback);
    glutKeyboardFunc(keyboard_callback);
    glutKeyboardUpFunc(keyboard_up_callback);
    glutTimerFunc(16, timer_callback, 0);
    
    glutMainLoop();
    return 0;
}
