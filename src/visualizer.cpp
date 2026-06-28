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

static float camera_rot_x = 20.0f;
static float camera_rot_y = -30.0f;

static float hover_mouse_x = 0.0f;
static float hover_mouse_y = 0.0f;

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

void visualizer_display() {
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
    draw_container(50.0f, 40.0f, 1550.0f, 310.0f);

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

    float current_y_3d_btn = 695.0f;
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
        current_y_3d_btn = 480.0f;
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
    draw_stroke_text_centered(175.0f, current_y_3d_btn + 22.5f, 0.15f, 2.0f, "3D");

    if (!current_mode_3d) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(viewport_x + (int)(330.0f / 1600.0f * viewport_width),
                  viewport_y + (int)(350.0f / 900.0f * viewport_height),
                  (int)((1250.0f - 330.0f) / 1600.0f * viewport_width),
                  (int)((850.0f - 350.0f) / 900.0f * viewport_height));

        glColor3f(0.25f, 0.17f, 0.33f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        for (float gx = 390.0f; gx <= 1190.0f; gx += 100.0f) {
            glVertex2f(gx, 350.0f);
            glVertex2f(gx, 850.0f);
        }
        for (float gy = 400.0f; gy <= 800.0f; gy += 100.0f) {
            glVertex2f(330.0f, gy);
            glVertex2f(1250.0f, gy);
        }
        glEnd();

        glLineWidth(2.0f);
        glColor3f(0.9f, 0.5f, 0.5f);
        glBegin(GL_LINES);
        glVertex2f(340.0f, 600.0f);
        glVertex2f(1230.0f, 600.0f);
        glVertex2f(1230.0f, 600.0f);
        glVertex2f(1220.0f, 595.0f);
        glVertex2f(1230.0f, 600.0f);
        glVertex2f(1220.0f, 605.0f);
        glEnd();

        glColor3f(0.5f, 0.8f, 0.5f);
        glBegin(GL_LINES);
        glVertex2f(790.0f, 360.0f);
        glVertex2f(790.0f, 840.0f);
        glVertex2f(790.0f, 840.0f);
        glVertex2f(785.0f, 830.0f);
        glVertex2f(790.0f, 840.0f);
        glVertex2f(795.0f, 830.0f);
        glEnd();
        glLineWidth(1.0f);

        glColor3f(0.918f, 0.804f, 0.761f);
        if (selected_shape == 0) {
            glPointSize(point_size);
            glBegin(GL_POINTS);
            glVertex2f(shape_x, shape_y);
            glEnd();
            glPointSize(1.0f);
        } else if (selected_shape == 1) {
            glLineWidth(3.0f);
            glBegin(GL_LINES);
            glVertex2f(shape_x, shape_y);
            glVertex2f(shape_x + line_dx, shape_y - line_dy);
            glEnd();
            glLineWidth(1.0f);
        } else if (selected_shape == 2) {
            glLineWidth(3.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(shape_x - rect_w / 2.0f, shape_y - rect_h / 2.0f);
            glVertex2f(shape_x + rect_w / 2.0f, shape_y - rect_h / 2.0f);
            glVertex2f(shape_x + rect_w / 2.0f, shape_y + rect_h / 2.0f);
            glVertex2f(shape_x - rect_w / 2.0f, shape_y + rect_h / 2.0f);
            glEnd();
            glLineWidth(1.0f);
        } else if (selected_shape == 3) {
            glLineWidth(3.0f);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < (int)circle_seg; i++) {
                float theta = 2.0f * 3.14159265f * (float)i / circle_seg;
                glVertex2f(shape_x + circle_r * cosf(theta), shape_y + circle_r * sinf(theta));
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
        glVertex2f(shape_x - 6.0f, shape_y - 6.0f);
        glVertex2f(shape_x + 6.0f, shape_y - 6.0f);
        glVertex2f(shape_x + 6.0f, shape_y + 6.0f);
        glVertex2f(shape_x - 6.0f, shape_y + 6.0f);
        glEnd();

        glDisable(GL_SCISSOR_TEST);
    } else {
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

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(45.0, (double)pw / ph, 0.1, 100.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_DEPTH_TEST);

        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(camera_rot_x, 1.0f, 0.0f, 0.0f);
        glRotatef(camera_rot_y, 0.0f, 1.0f, 0.0f);

        glLineWidth(3.0f);
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

        glColor3f(0.918f, 0.804f, 0.761f);
        glLineWidth(1.5f);
        glutWireCube(1.2);
        glLineWidth(1.0f);

        glDisable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glDisable(GL_SCISSOR_TEST);
        glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    }

    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(1300.0f, 815.0f, GLUT_BITMAP_HELVETICA_18, "PARAMETROS");

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
            draw_text(1300.0f, 700.0f, GLUT_BITMAP_HELVETICA_18, label1);
            glColor3f(0.5f, 0.5f, 0.6f);
            glBegin(GL_LINES);
            glVertex2f(1350.0f, 670.0f);
            glVertex2f(1500.0f, 670.0f);
            glEnd();

            float t1 = (val1 - min1) / (max1 - min1);
            float sx1 = 1350.0f + t1 * 150.0f;

            if (selected_point_index == 1) {
                glColor3f(1.0f, 0.5f, 0.5f);
            } else {
                glColor3f(0.918f, 0.804f, 0.761f);
            }
            glBegin(GL_QUADS);
            glVertex2f(sx1 - 6.0f, 670.0f - 6.0f);
            glVertex2f(sx1 + 6.0f, 670.0f - 6.0f);
            glVertex2f(sx1 + 6.0f, 670.0f + 6.0f);
            glVertex2f(sx1 - 6.0f, 670.0f + 6.0f);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), "%.1f", val1);
            draw_text(1515.0f, 675.0f, GLUT_BITMAP_HELVETICA_18, val_buf);
        }

        if (!label2.empty()) {
            draw_text(1300.0f, 620.0f, GLUT_BITMAP_HELVETICA_18, label2);
            glColor3f(0.5f, 0.5f, 0.6f);
            glBegin(GL_LINES);
            glVertex2f(1350.0f, 590.0f);
            glVertex2f(1500.0f, 590.0f);
            glEnd();

            float t2 = (val2 - min2) / (max2 - min2);
            float sx2 = 1350.0f + t2 * 150.0f;

            if (selected_point_index == 2) {
                glColor3f(1.0f, 0.5f, 0.5f);
            } else {
                glColor3f(0.918f, 0.804f, 0.761f);
            }
            glBegin(GL_QUADS);
            glVertex2f(sx2 - 6.0f, 590.0f - 6.0f);
            glVertex2f(sx2 + 6.0f, 590.0f - 6.0f);
            glVertex2f(sx2 + 6.0f, 590.0f + 6.0f);
            glVertex2f(sx2 - 6.0f, 590.0f + 6.0f);
            glEnd();

            glColor3f(0.918f, 0.804f, 0.761f);
            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), "%.1f", val2);
            draw_text(1515.0f, 595.0f, GLUT_BITMAP_HELVETICA_18, val_buf);
        }
    } else {
        draw_text(1300.0f, 780.0f, GLUT_BITMAP_HELVETICA_18, "Modo 3D Ativo");
        draw_text(1300.0f, 740.0f, GLUT_BITMAP_HELVETICA_18, "Rotacione a camera");
        draw_text(1300.0f, 710.0f, GLUT_BITMAP_HELVETICA_18, "arrastando com o");
        draw_text(1300.0f, 680.0f, GLUT_BITMAP_HELVETICA_18, "botao direito.");
    }

    glColor3f(0.918f, 0.804f, 0.761f);
    if (!current_mode_3d) {
        float rel_x = shape_x - 790.0f;
        float rel_y = shape_y - 600.0f;

        if (selected_shape == 0) {
            char code_buf[128];
            snprintf(code_buf, sizeof(code_buf), "glPointSize(%.1ff);", point_size);
            draw_text(80.0f, 270.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(80.0f, 230.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_POINTS);");
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x, rel_y);
            draw_text(80.0f, 190.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(80.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
        } else if (selected_shape == 1) {
            char code_buf[128];
            draw_text(80.0f, 270.0f, GLUT_BITMAP_HELVETICA_18, "glLineWidth(3.0f);");
            draw_text(80.0f, 230.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_LINES);");
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x, rel_y);
            draw_text(80.0f, 190.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x + line_dx, rel_y - line_dy);
            draw_text(80.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(80.0f, 110.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
        } else if (selected_shape == 2) {
            char code_buf[128];
            draw_text(80.0f, 270.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_LINE_LOOP);");
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x - rect_w / 2.0f, rel_y - rect_h / 2.0f);
            draw_text(80.0f, 230.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x + rect_w / 2.0f, rel_y - rect_h / 2.0f);
            draw_text(80.0f, 190.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x + rect_w / 2.0f, rel_y + rect_h / 2.0f);
            draw_text(80.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff, %.1ff);", rel_x - rect_w / 2.0f, rel_y + rect_h / 2.0f);
            draw_text(80.0f, 110.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(80.0f, 70.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
        } else if (selected_shape == 3) {
            char code_buf[128];
            draw_text(80.0f, 270.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_LINE_LOOP);");
            snprintf(code_buf, sizeof(code_buf), "for (int i = 0; i < %d; i++) {", (int)circle_seg);
            draw_text(80.0f, 230.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    float theta = 2.0f * 3.14159f * i / %d;", (int)circle_seg);
            draw_text(80.0f, 190.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            snprintf(code_buf, sizeof(code_buf), "    glVertex2f(%.1ff + %.1ff * cos(theta), %.1ff + %.1ff * sin(theta));", rel_x, circle_r, rel_y, circle_r);
            draw_text(80.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
            draw_text(80.0f, 110.0f, GLUT_BITMAP_HELVETICA_18, "}");
            draw_text(80.0f, 70.0f, GLUT_BITMAP_HELVETICA_18, "glEnd();");
        }
    } else {
        char rot_buf[128];
        draw_text(80.0f, 270.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_PROJECTION);");
        draw_text(80.0f, 230.0f, GLUT_BITMAP_HELVETICA_18, "glLoadIdentity();");
        draw_text(80.0f, 190.0f, GLUT_BITMAP_HELVETICA_18, "gluPerspective(45.0, aspect_ratio, 0.1, 100.0);");
        draw_text(80.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_MODELVIEW);");
        draw_text(80.0f, 110.0f, GLUT_BITMAP_HELVETICA_18, "glLoadIdentity();");
        snprintf(rot_buf, sizeof(rot_buf), "glRotatef(%.1ff, 1.0f, 0.0f, 0.0f); glRotatef(%.1ff, 0.0f, 1.0f, 0.0f);", camera_rot_x, camera_rot_y);
        draw_text(80.0f, 70.0f, GLUT_BITMAP_HELVETICA_18, rot_buf);
    }

    glutSwapBuffers();
}

void visualizer_mouse(int button, int state, int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            if (is_inside(mouse_pos.x, mouse_pos.y, 70.0f, 745.0f, 280.0f, 790.0f)) {
                current_mode_3d = false;
                is_2d_expanded = !is_2d_expanded;
            } else {
                float current_y_3d_btn = is_2d_expanded ? 480.0f : 695.0f;
                if (is_inside(mouse_pos.x, mouse_pos.y, 70.0f, current_y_3d_btn, 280.0f, current_y_3d_btn + 45.0f)) {
                    current_mode_3d = true;
                } else if (is_2d_expanded && !current_mode_3d) {
                    for (int i = 0; i < 4; i++) {
                        float y1 = 695.0f - i * 50.0f;
                        float y2 = y1 + 40.0f;
                        if (is_inside(mouse_pos.x, mouse_pos.y, 90.0f, y1, 280.0f, y2)) {
                            selected_shape = i;
                            break;
                        }
                    }
                }
            }

            if (!current_mode_3d) {
                float dx = mouse_pos.x - shape_x;
                float dy = mouse_pos.y - shape_y;
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
                        float sx1 = 1350.0f + t1 * 150.0f;
                        float sdx = mouse_pos.x - sx1;
                        float sdy = mouse_pos.y - 670.0f;
                        if (std::sqrt(sdx*sdx + sdy*sdy) <= 15.0f) {
                            selected_point_index = 1;
                        }
                    }

                    if (!label2.empty() && selected_point_index == -1) {
                        float t2 = (val2 - min2) / (max2 - min2);
                        float sx2 = 1350.0f + t2 * 150.0f;
                        float sdx = mouse_pos.x - sx2;
                        float sdy = mouse_pos.y - 590.0f;
                        if (std::sqrt(sdx*sdx + sdy*sdy) <= 15.0f) {
                            selected_point_index = 2;
                        }
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
    if (right_button_down && current_mode_3d) {
        int dx = x - last_mouse_x;
        int dy = y - last_mouse_y;
        camera_rot_y += (float)dx * 0.5f;
        camera_rot_x += (float)dy * 0.5f;
        last_mouse_x = x;
        last_mouse_y = y;
        glutPostRedisplay();
        return;
    }

    if (!current_mode_3d) {
        Point2D mouse_pos = map_mouse_to_ortho(x, y);
        hover_mouse_x = mouse_pos.x;
        hover_mouse_y = mouse_pos.y;

        if (selected_point_index == 0) {
            shape_x = mouse_pos.x;
            shape_y = mouse_pos.y;
            if (shape_x < 340.0f) shape_x = 340.0f;
            if (shape_x > 1240.0f) shape_x = 1240.0f;
            if (shape_y < 360.0f) shape_y = 360.0f;
            if (shape_y > 840.0f) shape_y = 840.0f;
        } else if (selected_point_index == 1) {
            float t = (mouse_pos.x - 1350.0f) / 150.0f;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float min1, max1, min2, max2;
            std::string label1, label2;
            get_slider_limits(selected_shape, &min1, &max1, &min2, &max2, label1, label2);
            set_param_1(min1 + t * (max1 - min1));
        } else if (selected_point_index == 2) {
            float t = (mouse_pos.x - 1350.0f) / 150.0f;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float min1, max1, min2, max2;
            std::string label1, label2;
            get_slider_limits(selected_shape, &min1, &max1, &min2, &max2, label1, label2);
            set_param_2(min2 + t * (max2 - min2));
        }
        glutPostRedisplay();
    }
}

void visualizer_passive_motion(int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);
    hover_mouse_x = mouse_pos.x;
    hover_mouse_y = mouse_pos.y;
    glutPostRedisplay();
}

void visualizer_keyboard(unsigned char key) {
    if (key == 27) {
        current_module = MENU;
    }
    glutPostRedisplay();
}
