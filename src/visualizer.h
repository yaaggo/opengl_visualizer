#ifndef VISUALIZER_H
#define VISUALIZER_H

void visualizer_display();
void visualizer_mouse(int button, int state, int x, int y);
void visualizer_motion(int x, int y);
void visualizer_passive_motion(int x, int y);
void visualizer_keyboard(unsigned char key);

#endif
