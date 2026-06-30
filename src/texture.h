#ifndef TEXTURE_H
#define TEXTURE_H

void texture_display();
void texture_mouse(int button, int state, int x, int y);
void texture_motion(int x, int y);
void texture_passive_motion(int x, int y);
void texture_keyboard(unsigned char key);
void texture_keyboard_up(unsigned char key);
void texture_update();

#endif
