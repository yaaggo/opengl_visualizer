#include <GL/freeglut.h>
#include <string>
#include <cmath>
#include "visualizer.h"
#include "modules.h"
#include "utils.h"

extern module_type current_module;
extern int viewport_width, viewport_height, viewport_x, viewport_y;

struct Point2D {
    float x, y;
};

static bool current_mode_3d = false;
static int selected_shape = 0;
static bool is_2d_expanded = true;
static bool is_3d_expanded = false;

static int selected_point_index = -1;
static bool right_button_down = false;
static int last_mouse_x = 0;
static int last_mouse_y = 0;

static float shape_x = 790.0f;
static float shape_y = 600.0f;

static float point_size = 10.0f;
static float line_dx = 150.0f;
static float line_dy = 100.0f;
static float rect_w = 200.0f;
static float rect_h = 150.0f;
static float circle_r = 100.0f;
static float circle_seg = 32.0f;

static bool is_lighting_enabled = true;
static int current_page_idx = 1;

static float obj_3d_x = 0.0f;
static float obj_3d_y = 0.0f;
static float obj_3d_z = 0.0f;
static float obj_rot_x = 0.0f;
static float obj_rot_y = 0.0f;

static float light_3d_x = 1.0f;
static float light_3d_y = 1.5f;
static float light_3d_z = 2.0f;
static float obj_3d_size = 1.2f;

static float camera_rot_x = 20.0f;
static float camera_rot_y = -30.0f;
static float camera_pos_x = 0.0f;
static float camera_pos_y = 0.0f;
static float camera_pos_z = -5.0f;

static float camera_2d_pos_x = 0.0f;
static float camera_2d_pos_y = 0.0f;

static float hover_mouse_x = 0.0f;
static float hover_mouse_y = 0.0f;

static bool keys_state[256] = { false };

static Point2D map_mouse_to_ortho(int x, int y) {
    int local_x = x - viewport_x;
    int local_y = y - viewport_y;
    Point2D mapped;
    mapped.x = ((float)local_x / viewport_width) * 1600.0f;
    mapped.y = (1.0f - (float)local_y / viewport_height) * 900.0f;
    return mapped;
}

static bool is_inside(float px, float py, float x1, float y1, float x2, float y2) {
    return (px >= x1 && px <= x2 && py >= y1 && py <= y2);
}

static void draw_container(float x1, float y1, float x2, float y2) {
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

static void get_slider_limits(int shape, float* min1, float* max1, float* min2, float* max2, std::string& label1, std::string& label2) {
    if (shape == 0) {
        *min1 = 1.0f; *max1 = 50.0f;
        *min2 = 0.0f; *max2 = 0.0f;
        label1 = "Tamanho";
        label2 = "";
    } else if (shape == 1) {
        *min1 = -300.0f; *max1 = 300.0f;
        *min2 = -300.0f; *max2 = 300.0f;
        label1 = "Comprimento X";
        label2 = "Comprimento Y";
    } else if (shape == 2) {
        *min1 = 10.0f; *max1 = 400.0f;
        *min2 = 10.0f; *max2 = 300.0f;
        label1 = "Largura";
        label2 = "Altura";
    } else if (shape == 3) {
        *min1 = 10.0f; *max1 = 200.0f;
        *min2 = 3.0f; *max2 = 64.0f;
        label1 = "Raio";
        label2 = "Segmentos";
    }
}

static float get_param_1() {
    if (selected_shape == 0) return point_size;
    if (selected_shape == 1) return line_dx;
    if (selected_shape == 2) return rect_w;
    return circle_r;
}

static float get_param_2() {
    if (selected_shape == 0) return 0.0f;
    if (selected_shape == 1) return line_dy;
    if (selected_shape == 2) return rect_h;
    return circle_seg;
}

static void set_param_1(float val) {
    if (selected_shape == 0) point_size = val;
    else if (selected_shape == 1) line_dx = val;
    else if (selected_shape == 2) rect_w = val;
    else circle_r = val;
}

static void set_param_2(float val) {
    if (selected_shape == 1) line_dy = val;
    else if (selected_shape == 2) rect_h = val;
    else if (selected_shape == 3) circle_seg = val;
}

static void get_slider_limits_3d(int slider_idx, float* min_val, float* max_val, std::string& label, float* val) {
    if (current_page_idx == 1) {
        if (slider_idx == 1) {
            *min_val = 0.1f; *max_val = 3.0f; label = "Tamanho"; *val = obj_3d_size;
        } else if (slider_idx == 2) {
            *min_val = -2.0f; *max_val = 2.0f; label = "Objeto X"; *val = obj_3d_x;
        } else if (slider_idx == 3) {
            *min_val = -2.0f; *max_val = 2.0f; label = "Objeto Y"; *val = obj_3d_y;
        } else if (slider_idx == 4) {
            *min_val = -2.0f; *max_val = 2.0f; label = "Objeto Z"; *val = obj_3d_z;
        } else if (slider_idx == 5) {
            *min_val = -180.0f; *max_val = 180.0f; label = "Rotacao X"; *val = obj_rot_x;
        } else if (slider_idx == 6) {
            *min_val = -180.0f; *max_val = 180.0f; label = "Rotacao Y"; *val = obj_rot_y;
        }
    } else {
        if (slider_idx == 1) {
            *min_val = -5.0f; *max_val = 5.0f; label = "Luz X"; *val = light_3d_x;
        } else if (slider_idx == 2) {
            *min_val = -5.0f; *max_val = 5.0f; label = "Luz Y"; *val = light_3d_y;
        } else if (slider_idx == 3) {
            *min_val = -5.0f; *max_val = 5.0f; label = "Luz Z"; *val = light_3d_z;
        }
    }
}

static void set_slider_val_3d(int slider_idx, float val) {
    if (current_page_idx == 1) {
        if (slider_idx == 1) obj_3d_size = val;
        else if (slider_idx == 2) obj_3d_x = val;
        else if (slider_idx == 3) obj_3d_y = val;
        else if (slider_idx == 4) obj_3d_z = val;
        else if (slider_idx == 5) obj_rot_x = val;
        else if (slider_idx == 6) obj_rot_y = val;
    } else {
        if (slider_idx == 1) light_3d_x = val;
        else if (slider_idx == 2) light_3d_y = val;
        else if (slider_idx == 3) light_3d_z = val;
    }
}

static int get_max_sliders_3d() {
    if (current_page_idx == 1) {
        return 6;
    } else {
        return 3;
    }
}

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

static void draw_bitmap_text_centered(float x1, float x2, float y, void* font, const std::string& text) {
    float text_w = 0.0f;
    for (char c : text) {
        text_w += glutBitmapWidth(font, c);
    }
    float start_x = (x1 + x2) / 2.0f - text_w / 2.0f;
    draw_text(start_x, y, font, text);
}

void visualizer_display() {
    if (!is_lighting_enabled) {
        current_page_idx = 1;
    }

    glClearColor(0.216f, 0.145f, 0.286f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1600, 0, 900);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(50.0f, 865.0f, GLUT_BITMAP_HELVETICA_18, "Visualizador OpenGL - Selecione formas e altere parametros");
    draw_text(1420.0f, 865.0f, GLUT_BITMAP_HELVETICA_18, "ESC para voltar");

    draw_container(50.0f, 350.0f, 300.0f, 850.0f);
    draw_container(330.0f, 350.0f, 1250.0f, 850.0f);
    draw_container(1280.0f, 350.0f, 1550.0f, 850.0f);

    if (!current_mode_3d) {
        draw_container(50.0f, 40.0f, 790.0f, 310.0f);
        draw_container(810.0f, 40.0f, 1550.0f, 310.0f);

        glColor3f(0.918f, 0.804f, 0.761f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(50.0f, 265.0f);
        glVertex2f(790.0f, 265.0f);
        glVertex2f(810.0f, 265.0f);
        glVertex2f(1550.0f, 265.0f);
        glEnd();
        glLineWidth(1.0f);
    } else {
        draw_container(50.0f, 40.0f, 530.0f, 310.0f);
        draw_container(550.0f, 40.0f, 1040.0f, 310.0f);
        draw_container(1060.0f, 40.0f, 1550.0f, 310.0f);

        glColor3f(0.918f, 0.804f, 0.761f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(50.0f, 265.0f);
        glVertex2f(530.0f, 265.0f);
        glVertex2f(550.0f, 265.0f);
        glVertex2f(1040.0f, 265.0f);
        glVertex2f(1060.0f, 265.0f);
        glVertex2f(1550.0f, 265.0f);
        glEnd();
        glLineWidth(1.0f);
    }

    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(85.0f, 815.0f, GLUT_BITMAP_HELVETICA_18, "SELETOR");

    bool hover_2d = is_inside(hover_mouse_x, hover_mouse_y, 70.0f, 745.0f, 280.0f, 790.0f);
    if (!current_mode_3d) {
        glColor3f(0.353f, 0.243f, 0.459f);
    } else if (hover_2d) {
        glColor3f(0.282f, 0.192f, 0.369f);
    } else {
        glColor3f(0.231f, 0.153f, 0.306f);
    }
    glBegin(GL_QUADS);
    glVertex2f(70.0f, 745.0f);
    glVertex2f(280.0f, 745.0f);
    glVertex2f(280.0f, 790.0f);
    glVertex2f(70.0f, 790.0f);
    glEnd();

    glColor3f(0.918f, 0.804f, 0.761f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(70.0f, 745.0f);
    glVertex2f(280.0f, 745.0f);
    glVertex2f(280.0f, 790.0f);
    glVertex2f(70.0f, 790.0f);
    glEnd();
    draw_stroke_text_centered(175.0f, 767.5f, 0.15f, 2.0f, is_2d_expanded ? "2D [-]" : "2D [+]");

    float current_y_3d_btn = 0.0f;
    if (!current_mode_3d) {
        current_y_3d_btn = 480.0f;
    } else {
        current_y_3d_btn = 685.0f;
    }

    if (is_2d_expanded) {
        std::string shapes_labels[4] = { "PONTO", "LINHA", "RETANGULO", "CIRCULO" };
        for (int i = 0; i < 4; i++) {
            float y1 = 695.0f - i * 50.0f;
            float y2 = y1 + 40.0f;
            bool hover_shape = is_inside(hover_mouse_x, hover_mouse_y, 90.0f, y1, 280.0f, y2);
            if (!current_mode_3d && selected_shape == i) {
                glColor3f(0.353f, 0.243f, 0.459f);
            } else if (hover_shape) {
                glColor3f(0.282f, 0.192f, 0.369f);
            } else {
                glColor3f(0.231f, 0.153f, 0.306f);
            }
            glBegin(GL_QUADS);
            glVertex2f(90.0f, y1);
            glVertex2f(280.0f, y1);
            glVertex2f(280.0f, y2);
            glVertex2f(90.0f, y2);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(90.0f, y1);
            glVertex2f(280.0f, y1);
            glVertex2f(280.0f, y2);
            glVertex2f(90.0f, y2);
            glEnd();
            draw_stroke_text_centered(185.0f, (y1 + y2) / 2.0f, 0.12f, 1.5f, shapes_labels[i]);
        }
    }

    bool hover_3d = is_inside(hover_mouse_x, hover_mouse_y, 70.0f, current_y_3d_btn, 280.0f, current_y_3d_btn + 45.0f);
    if (current_mode_3d) {
        glColor3f(0.353f, 0.243f, 0.459f);
    } else if (hover_3d) {
        glColor3f(0.282f, 0.192f, 0.369f);
    } else {
        glColor3f(0.231f, 0.153f, 0.306f);
    }
    glBegin(GL_QUADS);
    glVertex2f(70.0f, current_y_3d_btn);
    glVertex2f(280.0f, current_y_3d_btn);
    glVertex2f(280.0f, current_y_3d_btn + 45.0f);
    glVertex2f(70.0f, current_y_3d_btn + 45.0f);
    glEnd();

    glColor3f(0.918f, 0.804f, 0.761f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(70.0f, current_y_3d_btn);
    glVertex2f(280.0f, current_y_3d_btn);
    glVertex2f(280.0f, current_y_3d_btn + 45.0f);
    glVertex2f(70.0f, current_y_3d_btn + 45.0f);
    glEnd();
    draw_stroke_text_centered(175.0f, current_y_3d_btn + 22.5f, 0.15f, 2.0f, is_3d_expanded ? "3D [-]" : "3D [+]");

    if (is_3d_expanded) {
        std::string shapes_labels_3d[5] = { "CUBO", "ESFERA", "CONE", "TORO", "BULE" };
        for (int i = 0; i < 5; i++) {
            float y1 = 635.0f - i * 50.0f;
            float y2 = y1 + 40.0f;
            bool hover_shape = is_inside(hover_mouse_x, hover_mouse_y, 90.0f, y1, 280.0f, y2);
            if (current_mode_3d && selected_shape == i) {
                glColor3f(0.353f, 0.243f, 0.459f);
            } else if (hover_shape) {
                glColor3f(0.282f, 0.192f, 0.369f);
            } else {
                glColor3f(0.231f, 0.153f, 0.306f);
            }
            glBegin(GL_QUADS);
            glVertex2f(90.0f, y1);
            glVertex2f(280.0f, y1);
            glVertex2f(280.0f, y2);
            glVertex2f(90.0f, y2);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(90.0f, y1);
            glVertex2f(280.0f, y1);
            glVertex2f(280.0f, y2);
            glVertex2f(90.0f, y2);
            glEnd();
            draw_stroke_text_centered(185.0f, (y1 + y2) / 2.0f, 0.12f, 1.5f, shapes_labels_3d[i]);
        }
    }

    float rx = 330.0f / 1600.0f;
    float ry = 350.0f / 900.0f;
    float rw = (1250.0f - 330.0f) / 1600.0f;
    float rh = (850.0f - 350.0f) / 900.0f;

    int px = viewport_x + (int)(rx * viewport_width);
    int py = viewport_y + (int)(ry * viewport_height);
    int pw = (int)(rw * viewport_width);
    int ph = (int)(rh * viewport_height);

    glViewport(px, py, pw, ph);
    glEnable(GL_SCISSOR_TEST);
    glScissor(px, py, pw, ph);

    glClearColor(0.169f, 0.106f, 0.231f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!current_mode_3d) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-460.0, 460.0, -250.0, 250.0, -1000.0, 1000.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glTranslatef(camera_2d_pos_x, camera_2d_pos_y, 0.0f);

        glColor3f(0.25f, 0.17f, 0.33f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        for (float gx = -10000.0f; gx <= 10000.0f; gx += 100.0f) {
            glVertex2f(gx, -10000.0f);
            glVertex2f(gx, 10000.0f);
        }
        for (float gy = -10000.0f; gy <= 10000.0f; gy += 100.0f) {
            glVertex2f(-10000.0f, gy);
            glVertex2f(10000.0f, gy);
        }
        glEnd();

        glLineWidth(4.0f);
        glColor3f(0.9f, 0.5f, 0.5f);
        glBegin(GL_LINES);
        glVertex2f(-10000.0f, 0.0f);
        glVertex2f(10000.0f, 0.0f);
        glEnd();

        glColor3f(0.5f, 0.8f, 0.5f);
        glBegin(GL_LINES);
        glVertex2f(0.0f, -10000.0f);
        glVertex2f(0.0f, 10000.0f);
        glEnd();
        glLineWidth(1.0f);

        float rel_x = shape_x - 790.0f;
        float rel_y = shape_y - 600.0f;

        glColor3f(0.851f, 0.753f, 0.949f);
        if (selected_shape == 0) {
            glPointSize(point_size);
            glBegin(GL_POINTS);
            glVertex2f(rel_x, rel_y);
            glEnd();
            glPointSize(1.0f);
        } else if (selected_shape == 1) {
            glLineWidth(4.0f);
            glBegin(GL_LINES);
            glVertex2f(rel_x, rel_y);
            glVertex2f(rel_x + line_dx, rel_y + line_dy);
            glEnd();
            glLineWidth(1.0f);
        } else if (selected_shape == 2) {
            glLineWidth(4.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(rel_x - rect_w / 2.0f, rel_y - rect_h / 2.0f);
            glVertex2f(rel_x + rect_w / 2.0f, rel_y - rect_h / 2.0f);
            glVertex2f(rel_x + rect_w / 2.0f, rel_y + rect_h / 2.0f);
            glVertex2f(rel_x - rect_w / 2.0f, rel_y + rect_h / 2.0f);
            glEnd();
            glLineWidth(1.0f);
        } else if (selected_shape == 3) {
            glLineWidth(4.0f);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < (int)circle_seg; i++) {
                float theta = 2.0f * 3.14159265f * (float)i / circle_seg;
                glVertex2f(rel_x + circle_r * cosf(theta), rel_y + circle_r * sinf(theta));
            }
            glEnd();
            glLineWidth(1.0f);
        }

        if (selected_point_index == 0) {
            glColor3f(1.0f, 0.5f, 0.5f);
        } else {
            glColor3f(0.918f, 0.804f, 0.761f);
        }
        glBegin(GL_QUADS);
        glVertex2f(rel_x - 6.0f, rel_y - 6.0f);
        glVertex2f(rel_x + 6.0f, rel_y - 6.0f);
        glVertex2f(rel_x + 6.0f, rel_y + 6.0f);
        glVertex2f(rel_x - 6.0f, rel_y + 6.0f);
        glEnd();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    } else {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(45.0, (double)pw / ph, 0.1, 100.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_DEPTH_TEST);

        glTranslatef(camera_pos_x, camera_pos_y, camera_pos_z);
        glRotatef(camera_rot_x, 1.0f, 0.0f, 0.0f);
        glRotatef(camera_rot_y, 0.0f, 1.0f, 0.0f);

        glLineWidth(2.0f);
        glColor3f(0.25f, 0.17f, 0.33f);
        glBegin(GL_LINES);
        for (float g = -25.0f; g <= 25.0f; g += 1.0f) {
            glVertex3f(g, -1.0f, -25.0f);
            glVertex3f(g, -1.0f, 25.0f);
            glVertex3f(-25.0f, -1.0f, g);
            glVertex3f(25.0f, -1.0f, g);
        }
        glEnd();
        glLineWidth(1.0f);

        if (is_lighting_enabled) {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

            GLfloat light_pos[4] = { light_3d_x, light_3d_y, light_3d_z, 1.0f };
            GLfloat light_diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            GLfloat light_ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

            glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
            glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        } else {
            glDisable(GL_LIGHTING);
        }

        glLineWidth(4.0f);
        glColor3f(0.9f, 0.5f, 0.5f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.5f, 0.0f, 0.0f);
        glEnd();

        glColor3f(0.5f, 0.8f, 0.5f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.5f, 0.0f);
        glEnd();

        glColor3f(0.5f, 0.6f, 0.9f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.5f);
        glEnd();
        glLineWidth(1.0f);

        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslatef(light_3d_x, light_3d_y, light_3d_z);
        glColor3f(0.9f, 0.9f, 0.5f);
        glBegin(GL_LINES);
        glVertex3f(-0.15f, 0.0f, 0.0f); glVertex3f(0.15f, 0.0f, 0.0f);
        glVertex3f(0.0f, -0.15f, 0.0f); glVertex3f(0.0f, 0.15f, 0.0f);
        glVertex3f(0.0f, 0.0f, -0.15f); glVertex3f(0.0f, 0.0f, 0.15f);
        glVertex3f(-0.1f, -0.1f, 0.0f); glVertex3f(0.1f, 0.1f, 0.0f);
        glVertex3f(-0.1f, 0.1f, 0.0f); glVertex3f(0.1f, -0.1f, 0.0f);
        glEnd();
        glPopMatrix();

        if (is_lighting_enabled) {
            glEnable(GL_LIGHTING);
        }

        glPushMatrix();
        glTranslatef(obj_3d_x, obj_3d_y, obj_3d_z);
        glRotatef(obj_rot_x, 1.0f, 0.0f, 0.0f);
        glRotatef(obj_rot_y, 0.0f, 1.0f, 0.0f);
        glColor3f(0.851f, 0.753f, 0.949f);

        if (is_lighting_enabled) {
            if (selected_shape == 0) {
                glutSolidCube(obj_3d_size);
            } else if (selected_shape == 1) {
                glutSolidSphere(obj_3d_size * 0.6f, 20, 20);
            } else if (selected_shape == 2) {
                glutSolidCone(obj_3d_size * 0.6f, obj_3d_size * 1.2f, 20, 20);
            } else if (selected_shape == 3) {
                glutSolidTorus(obj_3d_size * 0.25f, obj_3d_size * 0.55f, 20, 20);
            } else if (selected_shape == 4) {
                glutSolidTeapot(obj_3d_size * 0.7f);
            }
        } else {
            glLineWidth(1.5f);
            if (selected_shape == 0) {
                glutWireCube(obj_3d_size);
            } else if (selected_shape == 1) {
                glutWireSphere(obj_3d_size * 0.6f, 20, 20);
            } else if (selected_shape == 2) {
                glutWireCone(obj_3d_size * 0.6f, obj_3d_size * 1.2f, 20, 20);
            } else if (selected_shape == 3) {
                glutWireTorus(obj_3d_size * 0.25f, obj_3d_size * 0.55f, 20, 20);
            } else if (selected_shape == 4) {
                glutWireTeapot(obj_3d_size * 0.7f);
            }
            glLineWidth(1.0f);
        }

        glPopMatrix();

        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    glDisable(GL_SCISSOR_TEST);
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1600, 0, 900);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(1115.0f, 820.0f, GLUT_BITMAP_HELVETICA_18, "R - Reset Cam");
    draw_text(1115.0f, 795.0f, GLUT_BITMAP_HELVETICA_18, "Z - Reset Pos");

    if (current_mode_3d) {
        glColor3f(0.918f, 0.804f, 0.761f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(350.0f, 805.0f);
        glVertex2f(370.0f, 805.0f);
        glVertex2f(370.0f, 825.0f);
        glVertex2f(350.0f, 825.0f);
        glEnd();
        glLineWidth(1.0f);

        if (is_lighting_enabled) {
            glBegin(GL_QUADS);
            glVertex2f(355.0f, 810.0f);
            glVertex2f(365.0f, 810.0f);
            glVertex2f(365.0f, 820.0f);
            glVertex2f(355.0f, 820.0f);
            glEnd();
        }
        draw_text(380.0f, 810.0f, GLUT_BITMAP_HELVETICA_18, "Iluminacao + Solido");
    }

    glColor3f(0.918f, 0.804f, 0.761f);
    draw_bitmap_text_centered(1280.0f, 1550.0f, 815.0f, GLUT_BITMAP_HELVETICA_18, "PARAMETROS");

    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(1280.0f, 805.0f);
    glVertex2f(1550.0f, 805.0f);
    glEnd();
    glLineWidth(1.0f);

    if (!current_mode_3d) {
        float min1, max1, min2, max2;
        std::string label1, label2;
        get_slider_limits(selected_shape, &min1, &max1, &min2, &max2, label1, label2);

        float val1 = get_param_1();
        float val2 = get_param_2();

        float rel_x = shape_x - 790.0f;
        float rel_y = shape_y - 600.0f;

        char pos_buf[64];
        snprintf(pos_buf, sizeof(pos_buf), "Posicao X: %.1f", rel_x);
        draw_text(1300.0f, 780.0f, GLUT_BITMAP_HELVETICA_18, pos_buf);
        snprintf(pos_buf, sizeof(pos_buf), "Posicao Y: %.1f", rel_y);
        draw_text(1300.0f, 750.0f, GLUT_BITMAP_HELVETICA_18, pos_buf);

        if (!label1.empty()) {
            draw_text(1300.0f, 712.0f, GLUT_BITMAP_HELVETICA_18, label1);
            glColor3f(0.5f, 0.5f, 0.6f);
            glBegin(GL_LINES);
            glVertex2f(1330.0f, 700.0f);
            glVertex2f(1470.0f, 700.0f);
            glEnd();

            float t1 = (val1 - min1) / (max1 - min1);
            float sx1 = 1330.0f + t1 * 140.0f;

            if (selected_point_index == 1) {
                glColor3f(1.0f, 0.5f, 0.5f);
            } else {
                glColor3f(0.918f, 0.804f, 0.761f);
            }
            glBegin(GL_QUADS);
            glVertex2f(sx1 - 6.0f, 700.0f - 6.0f);
            glVertex2f(sx1 + 6.0f, 700.0f - 6.0f);
            glVertex2f(sx1 + 6.0f, 700.0f + 6.0f);
            glVertex2f(sx1 - 6.0f, 700.0f + 6.0f);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), "%.1f", val1);
            draw_text(1485.0f, 705.0f, GLUT_BITMAP_HELVETICA_18, val_buf);
        }

        if (!label2.empty()) {
            draw_text(1300.0f, 657.0f, GLUT_BITMAP_HELVETICA_18, label2);
            glColor3f(0.5f, 0.5f, 0.6f);
            glBegin(GL_LINES);
            glVertex2f(1330.0f, 645.0f);
            glVertex2f(1470.0f, 645.0f);
            glEnd();

            float t2 = (val2 - min2) / (max2 - min2);
            float sx2 = 1330.0f + t2 * 140.0f;

            if (selected_point_index == 2) {
                glColor3f(1.0f, 0.5f, 0.5f);
            } else {
                glColor3f(0.918f, 0.804f, 0.761f);
            }
            glBegin(GL_QUADS);
            glVertex2f(sx2 - 6.0f, 645.0f - 6.0f);
            glVertex2f(sx2 + 6.0f, 645.0f - 6.0f);
            glVertex2f(sx2 + 6.0f, 645.0f + 6.0f);
            glVertex2f(sx2 - 6.0f, 645.0f + 6.0f);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), "%.1f", val2);
            draw_text(1485.0f, 650.0f, GLUT_BITMAP_HELVETICA_18, val_buf);
        }
    } else {
        int max_sliders = get_max_sliders_3d();
        for (int idx = 1; idx <= max_sliders; idx++) {
            float min_val, max_val;
            std::string label;
            float val;
            get_slider_limits_3d(idx, &min_val, &max_val, label, &val);

            float rail_y = 750.0f - (idx - 1) * 55.0f;
            draw_text(1300.0f, rail_y + 12.0f, GLUT_BITMAP_HELVETICA_18, label);

            glColor3f(0.5f, 0.5f, 0.6f);
            glBegin(GL_LINES);
            glVertex2f(1330.0f, rail_y);
            glVertex2f(1470.0f, rail_y);
            glEnd();

            float t = (val - min_val) / (max_val - min_val);
            float sx = 1330.0f + t * 140.0f;

            if (selected_point_index == idx) {
                glColor3f(1.0f, 0.5f, 0.5f);
            } else {
                glColor3f(0.918f, 0.804f, 0.761f);
            }
            glBegin(GL_QUADS);
            glVertex2f(sx - 6.0f, rail_y - 6.0f);
            glVertex2f(sx + 6.0f, rail_y - 6.0f);
            glVertex2f(sx + 6.0f, rail_y + 6.0f);
            glVertex2f(sx - 6.0f, rail_y + 6.0f);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), "%.1f", val);
            draw_text(1485.0f, rail_y + 5.0f, GLUT_BITMAP_HELVETICA_18, val_buf);
        }

        if (is_lighting_enabled) {
            bool hover_left = is_inside(hover_mouse_x, hover_mouse_y, 1336.0f, 390.0f, 1360.0f, 420.0f);
            if (hover_left) {
                glColor3f(0.353f, 0.243f, 0.459f);
            } else {
                glColor3f(0.231f, 0.153f, 0.306f);
            }
            glBegin(GL_QUADS);
            glVertex2f(1336.0f, 390.0f);
            glVertex2f(1360.0f, 390.0f);
            glVertex2f(1360.0f, 420.0f);
            glVertex2f(1336.0f, 420.0f);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(1336.0f, 390.0f);
            glVertex2f(1360.0f, 390.0f);
            glVertex2f(1360.0f, 420.0f);
            glVertex2f(1336.0f, 420.0f);
            glEnd();
            draw_text(1343.0f, 400.0f, GLUT_BITMAP_HELVETICA_18, "<");

            char pag_buf[32];
            snprintf(pag_buf, sizeof(pag_buf), "Pagina %d/2", current_page_idx);
            draw_bitmap_text_centered(1360.0f, 1470.0f, 400.0f, GLUT_BITMAP_HELVETICA_18, pag_buf);

            bool hover_right = is_inside(hover_mouse_x, hover_mouse_y, 1470.0f, 390.0f, 1494.0f, 420.0f);
            if (hover_right) {
                glColor3f(0.353f, 0.243f, 0.459f);
            } else {
                glColor3f(0.231f, 0.153f, 0.306f);
            }
            glBegin(GL_QUADS);
            glVertex2f(1470.0f, 390.0f);
            glVertex2f(1494.0f, 390.0f);
            glVertex2f(1494.0f, 420.0f);
            glVertex2f(1470.0f, 420.0f);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(1470.0f, 390.0f);
            glVertex2f(1494.0f, 390.0f);
            glVertex2f(1494.0f, 420.0f);
            glVertex2f(1470.0f, 420.0f);
            glEnd();
            draw_text(1478.0f, 400.0f, GLUT_BITMAP_HELVETICA_18, ">");
        } else {
            draw_bitmap_text_centered(1280.0f, 1550.0f, 400.0f, GLUT_BITMAP_HELVETICA_18, "Pagina 1/1");
        }
    }

    glColor3f(0.918f, 0.804f, 0.761f);
    if (!current_mode_3d) {
        float rel_x = shape_x - 790.0f;
        float rel_y = shape_y - 600.0f;

        draw_bitmap_text_centered(50.0f, 790.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "PROJECAO E VISUALIZACAO 2D");
        draw_text(70.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_PROJECTION);");
        draw_text(70.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, "glLoadIdentity();");
        draw_text(70.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, "glOrtho(-460, 460, -250, 250, -1000, 1000);");
        draw_text(70.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_MODELVIEW);");
        draw_text(70.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, "glLoadIdentity();");
        char trans_buf[64];
        snprintf(trans_buf, sizeof(trans_buf), "glTranslatef(%.1ff, %.1ff, 0.0f);", camera_2d_pos_x, camera_2d_pos_y);
        draw_text(70.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, trans_buf);

        draw_bitmap_text_centered(810.0f, 1550.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "DESENHO DA FIGURA 2D");
        if (selected_shape == 0) {
            char code_buf[128];
            snprintf(code_buf, sizeof(code_buf), "glPointSize(%.1ff);", point_size);
            draw_text(830.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(830.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_POINTS);");
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x, rel_y);
            draw_text(830.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(830.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
        } else if (selected_shape == 1) {
            char code_buf[128];
            draw_text(830.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glLineWidth(4.0f);");
            draw_text(830.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_LINES);");
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x, rel_y);
            draw_text(830.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x + line_dx, rel_y + line_dy);
            draw_text(830.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(830.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
        } else if (selected_shape == 2) {
            char code_buf[128];
            draw_text(830.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_LINE_LOOP);");
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x - rect_w / 2.0f, rel_y - rect_h / 2.0f);
            draw_text(830.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x + rect_w / 2.0f, rel_y - rect_h / 2.0f);
            draw_text(830.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x + rect_w / 2.0f, rel_y + rect_h / 2.0f);
            draw_text(830.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x - rect_w / 2.0f, rel_y + rect_h / 2.0f);
            draw_text(830.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(830.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
        } else if (selected_shape == 3) {
            char code_buf[128];
            draw_text(830.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_LINE_LOOP);");
            snprintf(code_buf, sizeof(code_buf), "for (int i = 0; i < %d; i++) {", (int)circle_seg);
            draw_text(830.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    float theta = 2.0f * 3.14159f * i / %d;", (int)circle_seg);
            draw_text(830.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff + %.1ff * cos(theta), %.1ff + %.1ff * sin(theta));", rel_x, circle_r, rel_y, circle_r);
            draw_text(830.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(830.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, "}");
            draw_text(830.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
        }
    } else {
        char rot_buf[128];

        draw_bitmap_text_centered(50.0f, 530.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "PROJECAO E CAMERA");
        draw_text(70.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_PROJECTION);");
        draw_text(70.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, "glLoadIdentity();");
        draw_text(70.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, "gluPerspective(45.0, aspect_ratio, 0.1, 100.0);");
        draw_text(70.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_MODELVIEW);");
        draw_text(70.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, "glLoadIdentity();");
        snprintf(rot_buf, sizeof(rot_buf), "glTranslatef(%.2ff, %.2ff, %.2ff);", camera_pos_x, camera_pos_y, camera_pos_z);
        draw_text(70.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);
        snprintf(rot_buf, sizeof(rot_buf), "glRotatef(%.1ff, 1.0f, 0.0f, 0.0f); glRotatef(%.1ff, 0.0f, 1.0f, 0.0f);", camera_rot_x, camera_rot_y);
        draw_text(70.0f, 60.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);

        draw_bitmap_text_centered(550.0f, 1040.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "ROTACAO & TRANSLACAO SOLIDO");
        draw_text(570.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glPushMatrix();");
        snprintf(rot_buf, sizeof(rot_buf), "glTranslatef(%.2ff, %.2ff, %.2ff);", obj_3d_x, obj_3d_y, obj_3d_z);
        draw_text(570.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);
        snprintf(rot_buf, sizeof(rot_buf), "glRotatef(%.1ff, 1.0f, 0.0f, 0.0f);", obj_rot_x);
        draw_text(570.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);
        snprintf(rot_buf, sizeof(rot_buf), "glRotatef(%.1ff, 0.0f, 1.0f, 0.0f);", obj_rot_y);
        draw_text(570.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);
        draw_text(570.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, "// transformacao local concluida");
        draw_text(570.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, "glPopMatrix();");

        draw_bitmap_text_centered(1060.0f, 1550.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "ILUMINACAO & RENDER");
        if (is_lighting_enabled) {
            draw_text(1080.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);");
            snprintf(rot_buf, sizeof(rot_buf), "GLfloat pos[4] = { %.1ff, %.1ff, %.1ff, 1.0f };", light_3d_x, light_3d_y, light_3d_z);
            draw_text(1080.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);
            draw_text(1080.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, "glLightfv(GL_LIGHT0, GL_POSITION, pos);");
            draw_text(1080.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, "glColor3f(0.851f, 0.753f, 0.949f);");
            if (selected_shape == 0) {
                snprintf(rot_buf, sizeof(rot_buf), "glutSolidCube(%.2ff);", obj_3d_size);
            } else if (selected_shape == 1) {
                snprintf(rot_buf, sizeof(rot_buf), "glutSolidSphere(%.2ff, 20, 20);", obj_3d_size * 0.6f);
            } else if (selected_shape == 2) {
                snprintf(rot_buf, sizeof(rot_buf), "glutSolidCone(%.2ff, %.2ff, 20, 20);", obj_3d_size * 0.6f, obj_3d_size * 1.2f);
            } else if (selected_shape == 3) {
                snprintf(rot_buf, sizeof(rot_buf), "glutSolidTorus(%.2ff, %.2ff, 20, 20);", obj_3d_size * 0.25f, obj_3d_size * 0.55f);
            } else if (selected_shape == 4) {
                snprintf(rot_buf, sizeof(rot_buf), "glutSolidTeapot(%.2ff);", obj_3d_size * 0.7f);
            }
            draw_text(1080.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);
        } else {
            draw_text(1080.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glDisable(GL_LIGHTING);");
            draw_text(1080.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, "glColor3f(0.851f, 0.753f, 0.949f);");
            if (selected_shape == 0) {
                snprintf(rot_buf, sizeof(rot_buf), "glutWireCube(%.2ff);", obj_3d_size);
            } else if (selected_shape == 1) {
                snprintf(rot_buf, sizeof(rot_buf), "glutWireSphere(%.2ff, 20, 20);", obj_3d_size * 0.6f);
            } else if (selected_shape == 2) {
                snprintf(rot_buf, sizeof(rot_buf), "glutWireCone(%.2ff, %.2ff, 20, 20);", obj_3d_size * 0.6f, obj_3d_size * 1.2f);
            } else if (selected_shape == 3) {
                snprintf(rot_buf, sizeof(rot_buf), "glutWireTorus(%.2ff, %.2ff, 20, 20);", obj_3d_size * 0.25f, obj_3d_size * 0.55f);
            } else if (selected_shape == 4) {
                snprintf(rot_buf, sizeof(rot_buf), "glutWireTeapot(%.2ff);", obj_3d_size * 0.7f);
            }
            draw_text(1080.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);
        }
    }

    glutSwapBuffers();
}

void visualizer_mouse(int button, int state, int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            if (is_inside(mouse_pos.x, mouse_pos.y, 70.0f, 745.0f, 280.0f, 790.0f)) {
                current_mode_3d = false;
                is_2d_expanded = true;
                is_3d_expanded = false;
                selected_shape = 0;
            } else {
                float current_y_3d_btn = !current_mode_3d ? 480.0f : 685.0f;
                if (is_inside(mouse_pos.x, mouse_pos.y, 70.0f, current_y_3d_btn, 280.0f, current_y_3d_btn + 45.0f)) {
                    current_mode_3d = true;
                    is_3d_expanded = true;
                    is_2d_expanded = false;
                    selected_shape = 0;
                } else if (is_2d_expanded && !current_mode_3d) {
                    for (int i = 0; i < 4; i++) {
                        float y1 = 695.0f - i * 50.0f;
                        float y2 = y1 + 40.0f;
                        if (is_inside(mouse_pos.x, mouse_pos.y, 90.0f, y1, 280.0f, y2)) {
                            selected_shape = i;
                            break;
                        }
                    }
                } else if (is_3d_expanded && current_mode_3d) {
                    for (int i = 0; i < 5; i++) {
                        float y1 = 635.0f - i * 50.0f;
                        float y2 = y1 + 40.0f;
                        if (is_inside(mouse_pos.x, mouse_pos.y, 90.0f, y1, 280.0f, y2)) {
                            selected_shape = i;
                            break;
                        }
                    }
                }
            }

            if (current_mode_3d && is_inside(mouse_pos.x, mouse_pos.y, 350.0f, 805.0f, 520.0f, 825.0f)) {
                is_lighting_enabled = !is_lighting_enabled;
                if (!is_lighting_enabled) {
                    current_page_idx = 1;
                }
            }

            if (current_mode_3d && is_lighting_enabled) {
                if (is_inside(mouse_pos.x, mouse_pos.y, 1336.0f, 390.0f, 1360.0f, 420.0f)) {
                    current_page_idx = 1;
                } else if (is_inside(mouse_pos.x, mouse_pos.y, 1470.0f, 390.0f, 1494.0f, 420.0f)) {
                    current_page_idx = 2;
                }
            }

            if (!current_mode_3d) {
                float dx = mouse_pos.x - (shape_x + camera_2d_pos_x);
                float dy = mouse_pos.y - (shape_y + camera_2d_pos_y);
                if (std::sqrt(dx*dx + dy*dy) <= 15.0f) {
                    selected_point_index = 0;
                } else {
                    float min1, max1, min2, max2;
                    std::string label1, label2;
                    get_slider_limits(selected_shape, &min1, &max1, &min2, &max2, label1, label2);

                    float val1 = get_param_1();
                    float val2 = get_param_2();

                    if (!label1.empty()) {
                        float t1 = (val1 - min1) / (max1 - min1);
                        float sx1 = 1330.0f + t1 * 140.0f;
                        float sdx = mouse_pos.x - sx1;
                        float sdy = mouse_pos.y - 700.0f;
                        if (std::sqrt(sdx*sdx + sdy*sdy) <= 15.0f) {
                            selected_point_index = 1;
                        }
                    }

                    if (!label2.empty() && selected_point_index == -1) {
                        float t2 = (val2 - min2) / (max2 - min2);
                        float sx2 = 1330.0f + t2 * 140.0f;
                        float sdx = mouse_pos.x - sx2;
                        float sdy = mouse_pos.y - 645.0f;
                        if (std::sqrt(sdx*sdx + sdy*sdy) <= 15.0f) {
                            selected_point_index = 2;
                        }
                    }
                }
            } else {
                int max_sliders = get_max_sliders_3d();
                for (int idx = 1; idx <= max_sliders; idx++) {
                    float min_val, max_val;
                    std::string label;
                    float val;
                    get_slider_limits_3d(idx, &min_val, &max_val, label, &val);
                    float t = (val - min_val) / (max_val - min_val);
                    float sx = 1330.0f + t * 140.0f;
                    float rail_y = 750.0f - (idx - 1) * 55.0f;
                    float sdx = mouse_pos.x - sx;
                    float sdy = mouse_pos.y - rail_y;
                    if (std::sqrt(sdx*sdx + sdy*sdy) <= 15.0f) {
                        selected_point_index = idx;
                        break;
                    }
                }
            }
        } else if (state == GLUT_UP) {
            selected_point_index = -1;
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            right_button_down = true;
            last_mouse_x = x;
            last_mouse_y = y;
        } else if (state == GLUT_UP) {
            right_button_down = false;
        }
    }
    glutPostRedisplay();
}

void visualizer_motion(int x, int y) {
    if (right_button_down) {
        int dx = x - last_mouse_x;
        int dy = y - last_mouse_y;
        if (current_mode_3d) {
            camera_rot_y += (float)dx * 0.5f;
            camera_rot_x += (float)dy * 0.5f;
        } else {
            camera_2d_pos_x += (float)dx * 1.25f;
            camera_2d_pos_y -= (float)dy * 1.25f;

            if (camera_2d_pos_x < -800.0f) camera_2d_pos_x = -800.0f;
            if (camera_2d_pos_x > 800.0f) camera_2d_pos_x = 800.0f;
            if (camera_2d_pos_y < -450.0f) camera_2d_pos_y = -450.0f;
            if (camera_2d_pos_y > 450.0f) camera_2d_pos_y = 450.0f;
        }
        last_mouse_x = x;
        last_mouse_y = y;
        glutPostRedisplay();
        return;
    }

    Point2D mouse_pos = map_mouse_to_ortho(x, y);
    hover_mouse_x = mouse_pos.x;
    hover_mouse_y = mouse_pos.y;

    if (!current_mode_3d) {
        if (selected_point_index == 0) {
            shape_x = mouse_pos.x - camera_2d_pos_x;
            shape_y = mouse_pos.y - camera_2d_pos_y;
            if (shape_x < 390.0f) shape_x = 390.0f;
            if (shape_x > 1190.0f) shape_x = 1190.0f;
            if (shape_y < 400.0f) shape_y = 400.0f;
            if (shape_y > 800.0f) shape_y = 800.0f;
        } else if (selected_point_index == 1) {
            float t = (mouse_pos.x - 1330.0f) / 140.0f;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float min1, max1, min2, max2;
            std::string label1, label2;
            get_slider_limits(selected_shape, &min1, &max1, &min2, &max2, label1, label2);
            set_param_1(min1 + t * (max1 - min1));
        } else if (selected_point_index == 2) {
            float t = (mouse_pos.x - 1330.0f) / 140.0f;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float min1, max1, min2, max2;
            std::string label1, label2;
            get_slider_limits(selected_shape, &min1, &max1, &min2, &max2, label1, label2);
            set_param_2(min2 + t * (max2 - min2));
        }
    } else {
        int max_sliders = get_max_sliders_3d();
        if (selected_point_index >= 1 && selected_point_index <= max_sliders) {
            float t = (mouse_pos.x - 1330.0f) / 140.0f;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float min_val, max_val;
            std::string label;
            float val;
            get_slider_limits_3d(selected_point_index, &min_val, &max_val, label, &val);
            set_slider_val_3d(selected_point_index, min_val + t * (max_val - min_val));
        }
    }
    glutPostRedisplay();
}

void visualizer_passive_motion(int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);
    hover_mouse_x = mouse_pos.x;
    hover_mouse_y = mouse_pos.y;
    glutPostRedisplay();
}

void visualizer_keyboard(unsigned char key) {
    unsigned char lower_key = tolower(key);
    if (key == 27) {
        current_module = MENU;
    } else if (lower_key == 'r') {
        camera_rot_x = 20.0f;
        camera_rot_y = -30.0f;
        camera_pos_x = 0.0f;
        camera_pos_y = 0.0f;
        camera_pos_z = -5.0f;
        camera_2d_pos_x = 0.0f;
        camera_2d_pos_y = 0.0f;
    } else if (lower_key == 'z') {
        shape_x = 790.0f;
        shape_y = 600.0f;
        point_size = 10.0f;
        line_dx = 150.0f;
        line_dy = 100.0f;
        rect_w = 200.0f;
        rect_h = 150.0f;
        circle_r = 100.0f;
        circle_seg = 32.0f;
        obj_3d_size = 1.2f;
        obj_3d_x = 0.0f;
        obj_3d_y = 0.0f;
        obj_3d_z = 0.0f;
        obj_rot_x = 0.0f;
        obj_rot_y = 0.0f;
        light_3d_x = 1.0f;
        light_3d_y = 1.5f;
        light_3d_z = 2.0f;
        current_page_idx = 1;
    } else if (lower_key == 'w' || lower_key == 's' || lower_key == 'a' || lower_key == 'd') {
        keys_state[lower_key] = true;
    }
    glutPostRedisplay();
}

void visualizer_keyboard_up(unsigned char key) {
    keys_state[tolower(key)] = false;
}

void visualizer_update() {
    if (current_mode_3d) {
        if (keys_state['w']) {
            camera_pos_z += 0.05f;
        }
        if (keys_state['s']) {
            camera_pos_z -= 0.05f;
        }
        if (keys_state['a']) {
            camera_pos_x += 0.05f;
        }
        if (keys_state['d']) {
            camera_pos_x -= 0.05f;
        }

        if (camera_pos_x < -5.0f) camera_pos_x = -5.0f;
        if (camera_pos_x > 5.0f) camera_pos_x = 5.0f;
        if (camera_pos_z < -15.0f) camera_pos_z = -15.0f;
        if (camera_pos_z > -1.0f) camera_pos_z = -1.0f;
    }
}
