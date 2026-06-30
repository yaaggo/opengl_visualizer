#include <GL/freeglut.h>
#include "modules.h"
#include "menu.h"
#include "bezier.h"
#include "visualizer.h"
#include "projections.h"

module_type current_module = MENU;
int viewport_width, viewport_height, viewport_x = 0, viewport_y = 0;

void display_callback() {
    switch (current_module) {
        case MENU:
            menu_display();
            break;
        case BEZIER:
            bezier_display();
            break;
        case VISUALIZER:
            visualizer_display();
            break;
        case PROJECTIONS:
            projections_display();
    }
}

void mouse_callback(int button, int state, int x, int y) {
    switch (current_module) {
        case MENU:
            menu_mouse(button, state, x, y);
            break;
        case BEZIER:
            bezier_mouse(button, state, x, y);
            break;
        case VISUALIZER:
            visualizer_mouse(button, state, x, y);
            break;
        case PROJECTIONS:
            projections_mouse(button, state, x, y);
            break;
    }
}

void motion_callback(int x, int y) {
    switch (current_module) {
        case MENU:
            menu_motion(x, y);
            break;
        case BEZIER:
            bezier_motion(x, y);
            break;
        case VISUALIZER:
            visualizer_motion(x, y);
            break;
        case PROJECTIONS:
            projections_motion(x, y);
            break;
    }
}

void passive_motion_callback(int x, int y) {
    switch (current_module) {
        case MENU:
            menu_motion(x, y);
            break;
        case BEZIER:
            break;
        case VISUALIZER:
            visualizer_passive_motion(x, y);
            break;
        case PROJECTIONS:
            projections_passive_motion(x, y);
            break;
    }
}

void keyboard_callback(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    if (key == 27) {
        if (current_module == MENU) {
            exit(0);
        }
        current_module = MENU;
        glutPostRedisplay();
        return;
    }
    switch (current_module) {
        case MENU:
            break;
        case BEZIER:
            bezier_keyboard(key);
            break;
        case VISUALIZER:
            visualizer_keyboard(key);
            break;
        case PROJECTIONS:
            projections_keyboard(key);
            break;
    }
}

void special_callback(int key, int x, int y) {
    (void)x;
    (void)y;
    switch (current_module) {
        case MENU:
            break;
        case BEZIER:
            break;
        case VISUALIZER:
            break;
        case PROJECTIONS:
            projections_special(key);
            break;
    }
}

void keyboard_up_callback(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    switch (current_module) {
        case MENU:
            break;
        case BEZIER:
            break;
        case VISUALIZER:
            visualizer_keyboard_up(key);
            break;
        case PROJECTIONS:
            break;
    }
}

void timer_callback(int value) {
    (void)value;
    if (current_module == VISUALIZER) {
        visualizer_update();
    } else if (current_module == PROJECTIONS) {
        projections_update();
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer_callback, 0);
}

void reshape_callback(int width, int height) {
    float target_aspect_ratio = 16.0f / 9.0f;
    float window_aspect_ratio = (float)width / (float)height;

    if (height == 0) {
        height = 1;
    }

    if (window_aspect_ratio > target_aspect_ratio) {
        viewport_height = height;
        viewport_width = (int)(height * target_aspect_ratio);
        viewport_x = (width - viewport_width) / 2;
    } else {
        viewport_width = width;
        viewport_height = (int)(width / target_aspect_ratio);
        viewport_y = (height - viewport_height) / 2;
    }

    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    glutPostRedisplay();
}
