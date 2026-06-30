#include <GL/freeglut.h>
#include <string>
#include <cmath>
#include "texture.h"
#include "modules.h"
#include "utils.h"

extern module_type current_module;
extern int viewport_width, viewport_height, viewport_x, viewport_y;

struct Point2D {
    float x, y;
};

// State variables
static int selected_texture_idx = 0;
static int selected_point_index = -1;
static bool right_button_down = false;
static int last_mouse_x = 0;
static int last_mouse_y = 0;

static float light_pos_x = 1.5f;
static float light_pos_y = 2.5f;
static float light_pos_z = 2.0f;
static float light_intensity = 1.0f;
static float light_ambient_val = 0.3f;
static float light_diffuse_val = 0.8f;

static float camera_rot_x = 20.0f;
static float camera_rot_y = -30.0f;
static float camera_pos_x = 0.0f;
static float camera_pos_y = 0.0f;
static float camera_pos_z = -5.0f;

static float hover_mouse_x = 0.0f;
static float hover_mouse_y = 0.0f;

static float cube_rot_y = 0.0f;
static bool auto_rotate = true;

static GLuint texture_ids[4] = { 0, 0, 0, 0 };
static bool textures_loaded = false;

// Procedural texture generators using theme colors:
// Beige: R=234, G=205, B=194 (0.918f, 0.804f, 0.761f)
// Purple: R=55, G=37, B=73 (0.216f, 0.145f, 0.286f)

static GLuint create_checkerboard_texture() {
    const int width = 64;
    const int height = 64;
    GLubyte image[width][height][4];
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            bool is_beige = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));
            image[i][j][0] = is_beige ? 234 : 55;
            image[i][j][1] = is_beige ? 205 : 37;
            image[i][j][2] = is_beige ? 194 : 73;
            image[i][j][3] = 255;
        }
    }
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return tex_id;
}


static void init_textures() {
    if (textures_loaded) return;
    texture_ids[0] = load_texture("assets/crate.png");
    if (texture_ids[0] == 0) {
        texture_ids[0] = create_checkerboard_texture();
    }
    texture_ids[1] = load_texture("assets/crate_metal.jpg");
    if (texture_ids[1] == 0) {
        texture_ids[1] = create_checkerboard_texture();
    }
    texture_ids[2] = load_texture("assets/crate_scifi.jpg");
    if (texture_ids[2] == 0) {
        texture_ids[2] = create_checkerboard_texture();
    }
    texture_ids[3] = load_texture("assets/crate_stone.jpg");
    if (texture_ids[3] == 0) {
        texture_ids[3] = create_checkerboard_texture();
    }
    textures_loaded = true;
}

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

static void draw_textured_cube(float size) {
    float h = size / 2.0f;
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h,  h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h,  h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h,  h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h,  h);
    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h, -h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-h,  h, -h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( h,  h, -h);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( h, -h, -h);
    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h, -h);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h,  h,  h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h,  h,  h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h, -h);
    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-h, -h, -h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( h, -h, -h);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( h, -h,  h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h,  h);
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( h, -h, -h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( h,  h, -h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( h,  h,  h);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( h, -h,  h);
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, -h);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h,  h);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-h,  h,  h);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-h,  h, -h);
    glEnd();
}

static void get_slider_limits_texture(int slider_idx, float* min_val, float* max_val, std::string& label, float* val) {
    if (slider_idx == 1) {
        *min_val = -5.0f; *max_val = 5.0f; label = "Luz X"; *val = light_pos_x;
    } else if (slider_idx == 2) {
        *min_val = -5.0f; *max_val = 5.0f; label = "Luz Y"; *val = light_pos_y;
    } else if (slider_idx == 3) {
        *min_val = -5.0f; *max_val = 5.0f; label = "Luz Z"; *val = light_pos_z;
    } else if (slider_idx == 4) {
        *min_val = 0.0f; *max_val = 2.0f; label = "Intensidade"; *val = light_intensity;
    } else if (slider_idx == 5) {
        *min_val = 0.0f; *max_val = 1.0f; label = "Ambiente"; *val = light_ambient_val;
    } else if (slider_idx == 6) {
        *min_val = 0.0f; *max_val = 1.0f; label = "Difusa"; *val = light_diffuse_val;
    }
}

static void set_slider_val_texture(int slider_idx, float val) {
    if (slider_idx == 1) light_pos_x = val;
    else if (slider_idx == 2) light_pos_y = val;
    else if (slider_idx == 3) light_pos_z = val;
    else if (slider_idx == 4) light_intensity = val;
    else if (slider_idx == 5) light_ambient_val = val;
    else if (slider_idx == 6) light_diffuse_val = val;
}

void texture_display() {
    init_textures();

    glClearColor(0.216f, 0.145f, 0.286f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set 2D Overlay Projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1600, 0, 900);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Headers
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(50.0f, 865.0f, GLUT_BITMAP_HELVETICA_18, "Mapeamento de Texturas - Escolha texturas e configure a iluminacao");
    draw_text(1420.0f, 865.0f, GLUT_BITMAP_HELVETICA_18, "ESC para voltar");

    // Layout Containers
    draw_container(50.0f, 350.0f, 300.0f, 850.0f);     // Left selector box
    draw_container(330.0f, 350.0f, 1250.0f, 850.0f);   // Viewport container
    draw_container(1280.0f, 350.0f, 1550.0f, 850.0f);  // Right configurations

    // 3 bottom code containers
    draw_container(50.0f, 40.0f, 530.0f, 310.0f);
    draw_container(550.0f, 40.0f, 1040.0f, 310.0f);
    draw_container(1060.0f, 40.0f, 1550.0f, 310.0f);

    glColor3f(0.918f, 0.804f, 0.761f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // Draw section lines in bottom containers
    glVertex2f(50.0f, 265.0f); glVertex2f(530.0f, 265.0f);
    glVertex2f(550.0f, 265.0f); glVertex2f(1040.0f, 265.0f);
    glVertex2f(1060.0f, 265.0f); glVertex2f(1550.0f, 265.0f);
    glEnd();
    glLineWidth(1.0f);

    // Left Panel Contents: Texture Selector
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(85.0f, 815.0f, GLUT_BITMAP_HELVETICA_18, "TEXTURAS");

    std::string text_labels[4] = { "1. MADEIRA", "2. METAL", "3. SCI-FI", "4. PEDRA" };
    for (int i = 0; i < 4; i++) {
        float y1 = 695.0f - i * 50.0f;
        float y2 = y1 + 40.0f;
        bool hover_btn = is_inside(hover_mouse_x, hover_mouse_y, 90.0f, y1, 280.0f, y2);
        
        if (selected_texture_idx == i) {
            glColor3f(0.353f, 0.243f, 0.459f);
        } else if (hover_btn) {
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

        draw_stroke_text_centered(185.0f, (y1 + y2) / 2.0f, 0.12f, 1.5f, text_labels[i]);
    }

    // Viewport coordinates math
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

    // Viewport background
    glClearColor(0.169f, 0.106f, 0.231f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3D Rendering setup
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(45.0, (double)pw / ph, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);

    gluLookAt(-camera_pos_x, -camera_pos_y, -camera_pos_z,
              -camera_pos_x, -camera_pos_y, 0.0f,
              0.0f, 1.0f, 0.0f);
    glRotatef(camera_rot_x, 1.0f, 0.0f, 0.0f);
    glRotatef(camera_rot_y, 0.0f, 1.0f, 0.0f);

    // Grid Floor
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

    // Lighting setup
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat light_pos[4] = { light_pos_x, light_pos_y, light_pos_z, 1.0f };
    GLfloat light_ambient[4] = { light_ambient_val * light_intensity, light_ambient_val * light_intensity, light_ambient_val * light_intensity, 1.0f };
    GLfloat light_diffuse[4] = { light_diffuse_val * light_intensity, light_diffuse_val * light_intensity, light_diffuse_val * light_intensity, 1.0f };
    GLfloat light_specular[4] = { light_intensity, light_intensity, light_intensity, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Render Light Source (crosshair)
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(light_pos_x, light_pos_y, light_pos_z);
    glColor3f(0.9f, 0.9f, 0.5f);
    glBegin(GL_LINES);
    glVertex3f(-0.15f, 0.0f, 0.0f); glVertex3f(0.15f, 0.0f, 0.0f);
    glVertex3f(0.0f, -0.15f, 0.0f); glVertex3f(0.0f, 0.15f, 0.0f);
    glVertex3f(0.0f, 0.0f, -0.15f); glVertex3f(0.0f, 0.0f, 0.15f);
    glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING);

    // Render Cube with Texturing
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glRotatef(cube_rot_y, 0.0f, 1.0f, 0.0f);

    GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_ids[selected_texture_idx]);

    draw_textured_cube(2.0f);

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_SCISSOR_TEST);
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);

    // Back to 2D Overlay
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1600, 0, 900);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Hotkeys / UI labels on center view
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(1085.0f, 820.0f, GLUT_BITMAP_HELVETICA_18, "R - Reset Cam");
    draw_text(1085.0f, 795.0f, GLUT_BITMAP_HELVETICA_18, "Z - Reset Pos");
    draw_text(1085.0f, 770.0f, GLUT_BITMAP_HELVETICA_18, auto_rotate ? "Espaco - Rotacao [ON]" : "Espaco - Rotacao [OFF]");

    // Right Panel Contents: Sliders
    draw_bitmap_text_centered(1280.0f, 1550.0f, 815.0f, GLUT_BITMAP_HELVETICA_18, "ILUMINACAO");
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(1280.0f, 805.0f); glVertex2f(1550.0f, 805.0f);
    glEnd();
    glLineWidth(1.0f);

    // Render 6 Sliders
    for (int idx = 1; idx <= 6; idx++) {
        float min_val, max_val;
        std::string label;
        float val;
        get_slider_limits_texture(idx, &min_val, &max_val, label, &val);

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

    // Bottom Code Panels
    char code_buf[128];
    // Panel 1
    draw_bitmap_text_centered(50.0f, 530.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "CENA & CAMERA (3D)");
    draw_text(70.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_PROJECTION);");
    draw_text(70.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, "gluPerspective(45.0, aspect, 0.1, 100.0);");
    draw_text(70.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_MODELVIEW);");
    snprintf(code_buf, sizeof(code_buf), "gluLookAt(%.1f, %.1f, %.1f, %.1f, %.1f, 0, 0, 1, 0);", -camera_pos_x, -camera_pos_y, -camera_pos_z, -camera_pos_x, -camera_pos_y);
    draw_text(70.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
    snprintf(code_buf, sizeof(code_buf), "glRotatef(%.1ff, 1, 0, 0); glRotatef(%.1ff, 0, 1, 0);", camera_rot_x, camera_rot_y);
    draw_text(70.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
    snprintf(code_buf, sizeof(code_buf), "glRotatef(%.1ff, 0, 1, 0); draw_cube(2.0f);", cube_rot_y);
    draw_text(70.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, code_buf);

    // Panel 2
    draw_bitmap_text_centered(550.0f, 1040.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "ILUMINACAO GLOBAL");
    draw_text(570.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);");
    snprintf(code_buf, sizeof(code_buf), "GLfloat pos[4] = { %.1ff, %.1ff, %.1ff, 1.0f };", light_pos_x, light_pos_y, light_pos_z);
    draw_text(570.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
    snprintf(code_buf, sizeof(code_buf), "GLfloat amb[4] = { %.2ff, %.2ff, %.2ff, 1.0f };", light_ambient_val * light_intensity, light_ambient_val * light_intensity, light_ambient_val * light_intensity);
    draw_text(570.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
    snprintf(code_buf, sizeof(code_buf), "GLfloat diff[4] = { %.2ff, %.2ff, %.2ff, 1.0f };", light_diffuse_val * light_intensity, light_diffuse_val * light_intensity, light_diffuse_val * light_intensity);
    draw_text(570.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
    draw_text(570.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, "glLightfv(GL_LIGHT0, GL_POSITION, pos);");
    draw_text(570.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, "glLightfv(GL_LIGHT0, GL_AMBIENT, amb);");
    draw_text(570.0f, 60.0f, GLUT_BITMAP_HELVETICA_18, "glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);");

    // Panel 3
    draw_bitmap_text_centered(1060.0f, 1550.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "MAPEAMENTO DE TEXTURA");
    std::string tex_path = "assets/crate.png";
    if (selected_texture_idx == 1) tex_path = "assets/crate_metal.jpg";
    else if (selected_texture_idx == 2) tex_path = "assets/crate_scifi.jpg";
    else if (selected_texture_idx == 3) tex_path = "assets/crate_stone.jpg";

    snprintf(code_buf, sizeof(code_buf), "tex_id = load_texture(\"%s\");", tex_path.c_str());
    draw_text(1080.0f, 240.0f, GLUT_BITMAP_HELVETICA_18, "glEnable(GL_TEXTURE_2D);");
    draw_text(1080.0f, 210.0f, GLUT_BITMAP_HELVETICA_18, code_buf);
    draw_text(1080.0f, 180.0f, GLUT_BITMAP_HELVETICA_18, "glBindTexture(GL_TEXTURE_2D, tex_id);");
    draw_text(1080.0f, 150.0f, GLUT_BITMAP_HELVETICA_18, "glBegin(GL_QUADS);");
    draw_text(1080.0f, 120.0f, GLUT_BITMAP_HELVETICA_18, "  glNormal3f(0.0f, 0.0f, 1.0f);");
    draw_text(1080.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, "  glTexCoord2f(0.0f, 0.0f); glVertex3f(-1,-1, 1);");
    draw_text(1080.0f, 60.0f, GLUT_BITMAP_HELVETICA_18, "  // ... outras faces do cubo ...");
    draw_text(1080.0f, 30.0f, GLUT_BITMAP_HELVETICA_18, "glEnd(); glDisable(GL_TEXTURE_2D);");

    glutSwapBuffers();
}

void texture_mouse(int button, int state, int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // Check texture selector buttons
            for (int i = 0; i < 4; i++) {
                float y1 = 695.0f - i * 50.0f;
                float y2 = y1 + 40.0f;
                if (is_inside(mouse_pos.x, mouse_pos.y, 90.0f, y1, 280.0f, y2)) {
                    selected_texture_idx = i;
                    break;
                }
            }

            // Check sliders
            for (int idx = 1; idx <= 6; idx++) {
                float min_val, max_val;
                std::string label;
                float val;
                get_slider_limits_texture(idx, &min_val, &max_val, label, &val);
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
    } else if (state == GLUT_UP) {
        if (button == 3) {
            camera_pos_z += 0.25f;
            if (camera_pos_z > -1.0f) camera_pos_z = -1.0f;
        } else if (button == 4) {
            camera_pos_z -= 0.25f;
            if (camera_pos_z < -15.0f) camera_pos_z = -15.0f;
        }
    }
    glutPostRedisplay();
}

void texture_motion(int x, int y) {
    if (right_button_down) {
        int dx = x - last_mouse_x;
        int dy = y - last_mouse_y;
        camera_rot_y += (float)dx * 0.5f;
        camera_rot_x += (float)dy * 0.5f;
        last_mouse_x = x;
        last_mouse_y = y;
        glutPostRedisplay();
        return;
    }

    Point2D mouse_pos = map_mouse_to_ortho(x, y);
    hover_mouse_x = mouse_pos.x;
    hover_mouse_y = mouse_pos.y;

    if (selected_point_index >= 1 && selected_point_index <= 6) {
        float t = (mouse_pos.x - 1330.0f) / 140.0f;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        float min_val, max_val;
        std::string label;
        float val;
        get_slider_limits_texture(selected_point_index, &min_val, &max_val, label, &val);
        set_slider_val_texture(selected_point_index, min_val + t * (max_val - min_val));
    }
    glutPostRedisplay();
}

void texture_passive_motion(int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);
    hover_mouse_x = mouse_pos.x;
    hover_mouse_y = mouse_pos.y;
    glutPostRedisplay();
}

void texture_keyboard(unsigned char key) {
    unsigned char lower_key = tolower(key);
    if (key == 27) {
        current_module = MENU;
    } else if (lower_key == 'r') {
        camera_rot_x = 20.0f;
        camera_rot_y = -30.0f;
        camera_pos_x = 0.0f;
        camera_pos_y = 0.0f;
        camera_pos_z = -5.0f;
    } else if (lower_key == ' ') {
        auto_rotate = !auto_rotate;
    } else if (lower_key == 'z') {
        light_pos_x = 1.5f;
        light_pos_y = 2.5f;
        light_pos_z = 2.0f;
        light_intensity = 1.0f;
        light_ambient_val = 0.3f;
        light_diffuse_val = 0.8f;
        camera_rot_x = 20.0f;
        camera_rot_y = -30.0f;
        camera_pos_x = 0.0f;
        camera_pos_y = 0.0f;
        camera_pos_z = -5.0f;
        cube_rot_y = 0.0f;
    }
    glutPostRedisplay();
}

void texture_keyboard_up(unsigned char key) {
    (void)key;
}

void texture_update() {
    if (auto_rotate) {
        cube_rot_y += 0.5f;
        if (cube_rot_y >= 360.0f) {
            cube_rot_y -= 360.0f;
        }
    }
}
