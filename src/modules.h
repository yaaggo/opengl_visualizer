#ifndef MODULES_H
#define MODULES_H

typedef enum { MENU, BEZIER, VISUALIZER, PROJECTIONS } module_type;
extern module_type current_module;
extern int viewport_width, viewport_height, viewport_x, viewport_y;

void display_callback();
void reshape_callback(int width, int height);
void mouse_callback(int button, int state, int x, int y);
void motion_callback(int x, int y);
void passive_motion_callback(int x, int y);
void keyboard_callback(unsigned char key, int x, int y);
void keyboard_up_callback(unsigned char key, int x, int y);
void special_callback(int key, int x, int y);
void timer_callback(int value);

#endif
