#include <GL/freeglut.h>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cctype>
#include "utils.h"
#include "modules.h"

enum ProjType { PERSP = 0, ORTHO = 1 };
ProjType current_proj = PERSP;

float cam_eyeX = 0.0f, cam_eyeY = 0.0f, cam_eyeZ = 8.0f;
float cam_cX = 0.0f, cam_cY = 0.0f, cam_cZ = 0.0f;
float cam_upX = 0.0f, cam_upY = 1.0f, cam_upZ = 0.0f;

float custom_aspect = 1.0f;
float p_fovy = 60.0f, p_zNear = 1.0f, p_zFar = 15.0f;
float o_size = 5.0f, o_zNear = 1.0f, o_zFar = 15.0f;

int selected_var = 0; 
const int MAX_VARS = 13;

struct Vec3 {
    float x, y, z;
};

static float debug_orbit_yaw = 39.0f;
static float debug_orbit_pitch = 32.0f;
static float debug_distance = 22.0f;
static float debug_target_distance = 22.0f;
static float debug_manual_zoom = 0.0f;
static Vec3 debug_focus = { 0.0f, 0.0f, 0.0f };
static Vec3 debug_target_focus = { 0.0f, 0.0f, 0.0f };
static bool debug_orbiting = false;
static int debug_last_mouse_x = 0;
static int debug_last_mouse_y = 0;

static Vec3 make_vec3(float x, float y, float z) {
    Vec3 v = { x, y, z };
    return v;
}

static Vec3 add_vec3(Vec3 a, Vec3 b) {
    return make_vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static Vec3 sub_vec3(Vec3 a, Vec3 b) {
    return make_vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static Vec3 scale_vec3(Vec3 v, float s) {
    return make_vec3(v.x * s, v.y * s, v.z * s);
}

static float dot_vec3(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static float length_vec3(Vec3 v) {
    return std::sqrt(dot_vec3(v, v));
}

static Vec3 normalize_vec3(Vec3 v, Vec3 fallback) {
    float len = length_vec3(v);
    if (len <= 0.0001f) return fallback;
    return scale_vec3(v, 1.0f / len);
}

static Vec3 cross_vec3(Vec3 a, Vec3 b) {
    return make_vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static void get_camera_basis(Vec3* forward, Vec3* right, Vec3* up) {
    Vec3 eye = make_vec3(cam_eyeX, cam_eyeY, cam_eyeZ);
    Vec3 center = make_vec3(cam_cX, cam_cY, cam_cZ);
    Vec3 world_up = normalize_vec3(make_vec3(cam_upX, cam_upY, cam_upZ), make_vec3(0.0f, 1.0f, 0.0f));

    *forward = normalize_vec3(sub_vec3(center, eye), make_vec3(0.0f, 0.0f, -1.0f));
    *right = normalize_vec3(cross_vec3(*forward, world_up), make_vec3(1.0f, 0.0f, 0.0f));
    *up = normalize_vec3(cross_vec3(*right, *forward), make_vec3(0.0f, 1.0f, 0.0f));
}

static void collect_frustum_points(std::vector<Vec3>& points) {
    points.clear();

    Vec3 eye = make_vec3(cam_eyeX, cam_eyeY, cam_eyeZ);
    Vec3 forward, right, up;
    get_camera_basis(&forward, &right, &up);

    float near_dist = current_proj == PERSP ? p_zNear : o_zNear;
    float far_dist = current_proj == PERSP ? p_zFar : o_zFar;
    float near_h, near_w, far_h, far_w;

    if (current_proj == PERSP) {
        float fovy_rad = p_fovy * 3.14159f / 180.0f;
        near_h = 2.0f * tan(fovy_rad / 2.0f) * near_dist;
        near_w = near_h * custom_aspect;
        far_h = 2.0f * tan(fovy_rad / 2.0f) * far_dist;
        far_w = far_h * custom_aspect;
    } else {
        near_h = o_size * 2.0f;
        near_w = near_h * custom_aspect;
        far_h = near_h;
        far_w = near_w;
    }

    Vec3 near_center = add_vec3(eye, scale_vec3(forward, near_dist));
    Vec3 far_center = add_vec3(eye, scale_vec3(forward, far_dist));
    float near_half_w = near_w * 0.5f;
    float near_half_h = near_h * 0.5f;
    float far_half_w = far_w * 0.5f;
    float far_half_h = far_h * 0.5f;

    points.push_back(eye);
    points.push_back(make_vec3(cam_cX, cam_cY, cam_cZ));

    points.push_back(add_vec3(add_vec3(near_center, scale_vec3(right, -near_half_w)), scale_vec3(up, -near_half_h)));
    points.push_back(add_vec3(add_vec3(near_center, scale_vec3(right,  near_half_w)), scale_vec3(up, -near_half_h)));
    points.push_back(add_vec3(add_vec3(near_center, scale_vec3(right,  near_half_w)), scale_vec3(up,  near_half_h)));
    points.push_back(add_vec3(add_vec3(near_center, scale_vec3(right, -near_half_w)), scale_vec3(up,  near_half_h)));

    points.push_back(add_vec3(add_vec3(far_center, scale_vec3(right, -far_half_w)), scale_vec3(up, -far_half_h)));
    points.push_back(add_vec3(add_vec3(far_center, scale_vec3(right,  far_half_w)), scale_vec3(up, -far_half_h)));
    points.push_back(add_vec3(add_vec3(far_center, scale_vec3(right,  far_half_w)), scale_vec3(up,  far_half_h)));
    points.push_back(add_vec3(add_vec3(far_center, scale_vec3(right, -far_half_w)), scale_vec3(up,  far_half_h)));
}

static void update_debug_auto_frame(bool immediate) {
    std::vector<Vec3> points;
    collect_frustum_points(points);
    if (points.empty()) return;

    Vec3 min_p = points[0];
    Vec3 max_p = points[0];
    for (const Vec3& p : points) {
        min_p.x = std::min(min_p.x, p.x);
        min_p.y = std::min(min_p.y, p.y);
        min_p.z = std::min(min_p.z, p.z);
        max_p.x = std::max(max_p.x, p.x);
        max_p.y = std::max(max_p.y, p.y);
        max_p.z = std::max(max_p.z, p.z);
    }

    debug_target_focus = scale_vec3(add_vec3(min_p, max_p), 0.5f);

    float radius = 1.0f;
    for (const Vec3& p : points) {
        radius = std::max(radius, length_vec3(sub_vec3(p, debug_target_focus)));
    }

    debug_target_distance = std::max(6.0f, radius * 2.15f + debug_manual_zoom);
    if (immediate) {
        debug_focus = debug_target_focus;
        debug_distance = debug_target_distance;
    }
}

static void adjust_debug_zoom(float delta) {
    debug_manual_zoom += delta;
    if (debug_manual_zoom < -18.0f) debug_manual_zoom = -18.0f;
    if (debug_manual_zoom > 80.0f) debug_manual_zoom = 80.0f;
    update_debug_auto_frame(false);
}

static void draw_container(float x1, float y1, float x2, float y2) {
    glColor3f(0.145f, 0.090f, 0.204f);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();

    glColor3f(0.541f, 0.431f, 0.667f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
    glLineWidth(1.0f);
}





void draw_axes() {
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0,0,0); glVertex3f(3,0,0); // Eixo X (Vermelho)
    glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0,0,0); glVertex3f(0,3,0); // Eixo Y (Verde)
    glColor3f(0.0f, 0.5f, 1.0f); glVertex3f(0,0,0); glVertex3f(0,0,3); // Eixo Z (Azul)
    glEnd();
    glLineWidth(1.0f);
}

void draw_frustum() {
    float fX = cam_cX - cam_eyeX;
    float fY = cam_cY - cam_eyeY;
    float fZ = cam_cZ - cam_eyeZ;
    float fMag = std::sqrt(fX*fX + fY*fY + fZ*fZ);
    if (fMag > 0.0f) { fX /= fMag; fY /= fMag; fZ /= fMag; }

    float upX = cam_upX;
    float upY = cam_upY;
    float upZ = cam_upZ;

    float sX = fY * upZ - fZ * upY;
    float sY = fZ * upX - fX * upZ;
    float sZ = fX * upY - fY * upX;
    float sMag = std::sqrt(sX*sX + sY*sY + sZ*sZ);
    if (sMag > 0.0f) { sX /= sMag; sY /= sMag; sZ /= sMag; }

    float uX = sY * fZ - sZ * fY;
    float uY = sZ * fX - sX * fZ;
    float uZ = sX * fY - sY * fX;

    float rot_matrix[16] = {
         sX,  sY,  sZ, 0.0f, 
         uX,  uY,  uZ, 0.0f, 
        -fX, -fY, -fZ, 0.0f, 
        0.0f,0.0f,0.0f, 1.0f 
    };

    glPushMatrix();
    
    glTranslatef(cam_eyeX, cam_eyeY, cam_eyeZ);
    
    glMultMatrixf(rot_matrix);

    if (current_proj == PERSP) {
        float nearH = 2.0f * tan((p_fovy * 3.14159f / 180.0f) / 2.0f) * p_zNear;
        float nearW = nearH * custom_aspect;
        float farH  = 2.0f * tan((p_fovy * 3.14159f / 180.0f) / 2.0f) * p_zFar;
        float farW  = farH * custom_aspect;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glColor4f(1.0f, 0.12f, 0.12f, 0.26f);
        glBegin(GL_QUADS);
        glVertex3f(-nearW/2, -nearH/2, -p_zNear);
        glVertex3f( nearW/2, -nearH/2, -p_zNear);
        glVertex3f( nearW/2,  nearH/2, -p_zNear);
        glVertex3f(-nearW/2,  nearH/2, -p_zNear);
        glEnd();

        glColor4f(0.15f, 0.45f, 1.0f, 0.22f);
        glBegin(GL_QUADS);
        glVertex3f(-farW/2, -farH/2, -p_zFar);
        glVertex3f( farW/2, -farH/2, -p_zFar);
        glVertex3f( farW/2,  farH/2, -p_zFar);
        glVertex3f(-farW/2,  farH/2, -p_zFar);
        glEnd();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex3f(0,0,0); glVertex3f(-farW/2, -farH/2, -p_zFar);
        glVertex3f(0,0,0); glVertex3f(farW/2, -farH/2, -p_zFar);
        glVertex3f(0,0,0); glVertex3f(-farW/2, farH/2, -p_zFar);
        glVertex3f(0,0,0); glVertex3f(farW/2, farH/2, -p_zFar);
        glEnd();
        
        glBegin(GL_LINE_LOOP);
        glVertex3f(-nearW/2, -nearH/2, -p_zNear); glVertex3f(nearW/2, -nearH/2, -p_zNear);
        glVertex3f(nearW/2, nearH/2, -p_zNear); glVertex3f(-nearW/2, nearH/2, -p_zNear);
        glEnd();
        
        glBegin(GL_LINE_LOOP);
        glVertex3f(-farW/2, -farH/2, -p_zFar); glVertex3f(farW/2, -farH/2, -p_zFar);
        glVertex3f(farW/2, farH/2, -p_zFar); glVertex3f(-farW/2, farH/2, -p_zFar);
        glEnd();

    } else {
        float left = -o_size * custom_aspect;
        float right = o_size * custom_aspect;
        float bottom = -o_size;
        float top = o_size;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glColor4f(1.0f, 0.12f, 0.12f, 0.26f);
        glBegin(GL_QUADS);
        glVertex3f(left, bottom, -o_zNear);
        glVertex3f(right, bottom, -o_zNear);
        glVertex3f(right, top, -o_zNear);
        glVertex3f(left, top, -o_zNear);
        glEnd();

        glColor4f(0.15f, 0.45f, 1.0f, 0.22f);
        glBegin(GL_QUADS);
        glVertex3f(left, bottom, -o_zFar);
        glVertex3f(right, bottom, -o_zFar);
        glVertex3f(right, top, -o_zFar);
        glVertex3f(left, top, -o_zFar);
        glEnd();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(left, bottom, -o_zNear); glVertex3f(right, bottom, -o_zNear);
        glVertex3f(right, top, -o_zNear); glVertex3f(left, top, -o_zNear);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3f(left, bottom, -o_zFar); glVertex3f(right, bottom, -o_zFar);
        glVertex3f(right, top, -o_zFar); glVertex3f(left, top, -o_zFar);
        glEnd();
        
        glBegin(GL_LINES);
        glVertex3f(left, bottom, -o_zNear); glVertex3f(left, bottom, -o_zFar);
        glVertex3f(right, bottom, -o_zNear); glVertex3f(right, bottom, -o_zFar);
        glVertex3f(right, top, -o_zNear); glVertex3f(right, top, -o_zFar);
        glVertex3f(left, top, -o_zNear); glVertex3f(left, top, -o_zFar);
        glEnd();
    }

    glPopMatrix();
    glLineWidth(1.0f);
}

struct Point2D {
    float x;
    float y;
};

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

static void clamp_proj_vars() {
    if (current_proj == PERSP) {
        if (p_zNear < 0.1f) p_zNear = 0.1f;
        if (p_zFar < p_zNear + 0.5f) p_zFar = p_zNear + 0.5f;
        if (p_fovy < 10.0f) p_fovy = 10.0f;
        if (p_fovy > 170.0f) p_fovy = 170.0f;
    } else {
        if (o_zNear < 0.1f) o_zNear = 0.1f;
        if (o_zFar < o_zNear + 0.5f) o_zFar = o_zNear + 0.5f;
        if (o_size < 1.0f) o_size = 1.0f;
    }
    if (custom_aspect < 0.2f) custom_aspect = 0.2f;
    if (custom_aspect > 3.0f) custom_aspect = 3.0f;
}

static void get_slider_limits_proj(int idx, float* min_val, float* max_val, std::string& label, float* val) {
    switch (idx) {
        case 1:
            *min_val = -10.0f; *max_val = 10.0f; label = "Eye X"; *val = cam_eyeX;
            break;
        case 2:
            *min_val = -10.0f; *max_val = 10.0f; label = "Eye Y"; *val = cam_eyeY;
            break;
        case 3:
            *min_val = -20.0f; *max_val = 20.0f; label = "Eye Z"; *val = cam_eyeZ;
            break;
        case 4:
            *min_val = -10.0f; *max_val = 10.0f; label = "Center X"; *val = cam_cX;
            break;
        case 5:
            *min_val = -10.0f; *max_val = 10.0f; label = "Center Y"; *val = cam_cY;
            break;
        case 6:
            *min_val = -10.0f; *max_val = 10.0f; label = "Center Z"; *val = cam_cZ;
            break;
        case 7:
            *min_val = -2.0f; *max_val = 2.0f; label = "Up X"; *val = cam_upX;
            break;
        case 8:
            *min_val = -2.0f; *max_val = 2.0f; label = "Up Y"; *val = cam_upY;
            break;
        case 9:
            *min_val = -2.0f; *max_val = 2.0f; label = "Up Z"; *val = cam_upZ;
            break;
        case 10:
            *min_val = 0.2f; *max_val = 3.0f; label = "Aspect"; *val = custom_aspect;
            break;
        case 11:
            if (current_proj == PERSP) {
                *min_val = 10.0f; *max_val = 150.0f; label = "Fovy"; *val = p_fovy;
            } else {
                *min_val = 1.0f; *max_val = 15.0f; label = "Size"; *val = o_size;
            }
            break;
        case 12:
            if (current_proj == PERSP) {
                *min_val = 0.1f; *max_val = 10.0f; label = "Near"; *val = p_zNear;
            } else {
                *min_val = 0.1f; *max_val = 10.0f; label = "Near"; *val = o_zNear;
            }
            break;
        case 13:
            if (current_proj == PERSP) {
                *min_val = 2.0f; *max_val = 30.0f; label = "Far"; *val = p_zFar;
            } else {
                *min_val = 2.0f; *max_val = 30.0f; label = "Far"; *val = o_zFar;
            }
            break;
        default:
            *min_val = 0.0f; *max_val = 1.0f; label = ""; *val = 0.0f;
            break;
    }
}

static void set_slider_val_proj(int idx, float val) {
    switch (idx) {
        case 1: cam_eyeX = val; break;
        case 2: cam_eyeY = val; break;
        case 3: cam_eyeZ = val; break;
        case 4: cam_cX = val; break;
        case 5: cam_cY = val; break;
        case 6: cam_cZ = val; break;
        case 7: cam_upX = val; break;
        case 8: cam_upY = val; break;
        case 9: cam_upZ = val; break;
        case 10: custom_aspect = val; break;
        case 11:
            if (current_proj == PERSP) p_fovy = val;
            else o_size = val;
            break;
        case 12:
            if (current_proj == PERSP) p_zNear = val;
            else o_zNear = val;
            break;
        case 13:
            if (current_proj == PERSP) p_zFar = val;
            else o_zFar = val;
            break;
    }
}

static void draw_view_label_proj(float x, float y, const std::string& title, const std::string& subtitle) {
    float bx1 = x + 16.0f;
    float by1 = y - 16.0f - 46.0f;
    float bx2 = x + 16.0f + 250.0f;
    float by2 = y - 16.0f;

    glColor3f(0.145f, 0.090f, 0.204f);
    glBegin(GL_QUADS);
    glVertex2f(bx1, by1);
    glVertex2f(bx2, by1);
    glVertex2f(bx2, by2);
    glVertex2f(bx1, by2);
    glEnd();

    glColor3f(0.541f, 0.431f, 0.667f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bx1, by1);
    glVertex2f(bx2, by1);
    glVertex2f(bx2, by2);
    glVertex2f(bx1, by2);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(0.941f, 0.839f, 0.780f);
    draw_text(bx1 + 12.0f, by2 - 19.0f, GLUT_BITMAP_HELVETICA_18, title);
    glColor3f(0.690f, 0.620f, 0.780f);
    draw_text(bx1 + 12.0f, by2 - 36.0f, GLUT_BITMAP_HELVETICA_12, subtitle);
}

static void draw_bitmap_text_centered(float x1, float x2, float y, void* font, const std::string& text) {
    float text_w = 0.0f;
    for (char c : text) {
        text_w += glutBitmapWidth(font, c);
    }
    float start_x = (x1 + x2) / 2.0f - text_w / 2.0f;
    draw_text(start_x, y, font, text);
}

void projections_display() {
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    glDisable(GL_LIGHTING);
    glClearColor(0.102f, 0.071f, 0.149f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1600, 0, 900);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    // Draw main titles
    glColor3f(0.941f, 0.839f, 0.780f);
    draw_text(50.0f, 865.0f, GLUT_BITMAP_HELVETICA_18, "Projecoes OpenGL - Compare a cena renderizada com a camera e o volume de recorte");
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(1420.0f, 865.0f, GLUT_BITMAP_HELVETICA_18, "ESC para voltar");

    // Draw background containers
    draw_container(50.0f, 350.0f, 650.0f, 850.0f);     // Left Viewport box
    draw_container(670.0f, 350.0f, 1270.0f, 850.0f);   // Right Viewport box
    draw_container(1290.0f, 350.0f, 1550.0f, 850.0f);  // Parameters panel
    draw_container(50.0f, 40.0f, 790.0f, 310.0f);      // Code panel 1
    draw_container(810.0f, 40.0f, 1550.0f, 310.0f);    // Code panel 2

    // Draw labels inside viewports
    draw_view_label_proj(50.0f, 850.0f, "Cena projetada", current_proj == PERSP ? "gluPerspective ativa" : "glOrtho ativa");
    draw_view_label_proj(670.0f, 850.0f, "Camera e frustum", "visao externa da projecao");

    // Draw code panels title & lines
    draw_bitmap_text_centered(50.0f, 790.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "MATRIZ DE PROJECAO");
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(50.0f, 265.0f);
    glVertex2f(790.0f, 265.0f);
    glEnd();
    glLineWidth(1.0f);

    draw_bitmap_text_centered(810.0f, 1550.0f, 275.0f, GLUT_BITMAP_HELVETICA_18, "MATRIZ MODELVIEW / CAMERA");
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(810.0f, 265.0f);
    glVertex2f(1550.0f, 265.0f);
    glEnd();
    glLineWidth(1.0f);

    // Draw active code content
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(70.0f, 235.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_PROJECTION);");
    draw_text(70.0f, 205.0f, GLUT_BITMAP_HELVETICA_18, "glLoadIdentity();");

    char buf[256];
    if (current_proj == PERSP) {
        snprintf(buf, sizeof(buf), "gluPerspective(%.1ff, %.2ff, %.1ff, %.1ff);", p_fovy, custom_aspect, p_zNear, p_zFar);
    } else {
        snprintf(buf, sizeof(buf), "glOrtho(%.1ff, %.1ff, %.1ff, %.1ff, %.1ff, %.1ff);",
                 -o_size * custom_aspect, o_size * custom_aspect, -o_size, o_size, o_zNear, o_zFar);
    }
    draw_text(70.0f, 175.0f, GLUT_BITMAP_HELVETICA_18, buf);

    draw_text(830.0f, 235.0f, GLUT_BITMAP_HELVETICA_18, "glMatrixMode(GL_MODELVIEW);");
    draw_text(830.0f, 205.0f, GLUT_BITMAP_HELVETICA_18, "glLoadIdentity();");
    snprintf(buf, sizeof(buf), "gluLookAt(%.1ff, %.1ff, %.1ff,  // eye", cam_eyeX, cam_eyeY, cam_eyeZ);
    draw_text(830.0f, 175.0f, GLUT_BITMAP_HELVETICA_18, buf);
    snprintf(buf, sizeof(buf), "          %.1ff, %.1ff, %.1ff,  // center", cam_cX, cam_cY, cam_cZ);
    draw_text(830.0f, 145.0f, GLUT_BITMAP_HELVETICA_18, buf);
    snprintf(buf, sizeof(buf), "          %.1ff, %.1ff, %.1ff); // up", cam_upX, cam_upY, cam_upZ);
    draw_text(830.0f, 115.0f, GLUT_BITMAP_HELVETICA_18, buf);

    // Draw parameters title & toggle button & sliders
    draw_bitmap_text_centered(1290.0f, 1550.0f, 820.0f, GLUT_BITMAP_HELVETICA_18, "PARAMETROS");
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(1290.0f, 810.0f);
    glVertex2f(1550.0f, 810.0f);
    glEnd();
    glLineWidth(1.0f);

    // Toggle button
    bool hover_toggle = is_inside(hover_mouse_x, hover_mouse_y, 1310.0f, 765.0f, 1530.0f, 800.0f);
    if (hover_toggle) {
        glColor3f(0.353f, 0.243f, 0.459f);
    } else {
        glColor3f(0.231f, 0.153f, 0.306f);
    }
    glBegin(GL_QUADS);
    glVertex2f(1310.0f, 765.0f);
    glVertex2f(1530.0f, 765.0f);
    glVertex2f(1530.0f, 800.0f);
    glVertex2f(1310.0f, 800.0f);
    glEnd();

    if (selected_var == 0) {
        glColor3f(1.0f, 0.5f, 0.5f);
    } else {
        glColor3f(0.918f, 0.804f, 0.761f);
    }
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(1310.0f, 765.0f);
    glVertex2f(1530.0f, 765.0f);
    glVertex2f(1530.0f, 800.0f);
    glVertex2f(1310.0f, 800.0f);
    glEnd();
    glLineWidth(1.0f);

    std::string proj_btn_text = (current_proj == PERSP) ? "PROJ: PERSPECTIVA" : "PROJ: ORTOGRAFICA";
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_bitmap_text_centered(1310.0f, 1530.0f, 777.0f, GLUT_BITMAP_HELVETICA_12, proj_btn_text);

    // Sliders
    for (int idx = 1; idx <= 13; idx++) {
        float min_val, max_val;
        std::string label;
        float val;
        get_slider_limits_proj(idx, &min_val, &max_val, label, &val);

        float rail_y = 735.0f - (idx - 1) * 28.0f;

        glColor3f(0.918f, 0.804f, 0.761f);
        draw_text(1300.0f, rail_y - 4.0f, GLUT_BITMAP_HELVETICA_12, label);

        glColor3f(0.5f, 0.5f, 0.6f);
        glBegin(GL_LINES);
        glVertex2f(1365.0f, rail_y - 8.0f);
        glVertex2f(1485.0f, rail_y - 8.0f);
        glEnd();

        float t = (val - min_val) / (max_val - min_val);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        float sx = 1365.0f + t * 120.0f;

        if (selected_var == idx) {
            glColor3f(1.0f, 0.5f, 0.5f);
        } else {
            glColor3f(0.918f, 0.804f, 0.761f);
        }
        glBegin(GL_QUADS);
        glVertex2f(sx - 4.0f, rail_y - 14.0f);
        glVertex2f(sx + 4.0f, rail_y - 14.0f);
        glVertex2f(sx + 4.0f, rail_y - 2.0f);
        glVertex2f(sx - 4.0f, rail_y - 2.0f);
        glEnd();

        glColor3f(0.918f, 0.804f, 0.761f);
        char val_buf[32];
        if (idx == 10) {
            snprintf(val_buf, sizeof(val_buf), "%.2f", val);
        } else {
            snprintf(val_buf, sizeof(val_buf), "%.1f", val);
        }
        draw_text(1495.0f, rail_y - 12.0f, GLUT_BITMAP_HELVETICA_12, val_buf);
    }

    // Now render 3D scenes inside the left/right viewports
    // Calculate pixel positions
    float rx_left = 50.0f / 1600.0f;
    float ry_left = 350.0f / 900.0f;
    float rw_left = (650.0f - 50.0f) / 1600.0f;
    float rh_left = (850.0f - 350.0f) / 900.0f;
    int px_left = viewport_x + (int)(rx_left * viewport_width);
    int py_left = viewport_y + (int)(ry_left * viewport_height);
    int pw_left = (int)(rw_left * viewport_width);
    int ph_left = (int)(rh_left * viewport_height);

    float rx_right = 670.0f / 1600.0f;
    float ry_right = 350.0f / 900.0f;
    float rw_right = (1270.0f - 670.0f) / 1600.0f;
    float rh_right = (850.0f - 350.0f) / 900.0f;
    int px_right = viewport_x + (int)(rx_right * viewport_width);
    int py_right = viewport_y + (int)(ry_right * viewport_height);
    int pw_right = (int)(rw_right * viewport_width);
    int ph_right = (int)(rh_right * viewport_height);

    // Viewport 1: Projected Scene
    glViewport(px_left, py_left, pw_left, ph_left);
    glEnable(GL_SCISSOR_TEST);
    glScissor(px_left, py_left, pw_left, ph_left);
    glClearColor(0.102f, 0.071f, 0.149f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (current_proj == PERSP) {
        gluPerspective(p_fovy, custom_aspect, p_zNear, p_zFar);
    } else {
        glOrtho(-o_size * custom_aspect, o_size * custom_aspect, -o_size, o_size, o_zNear, o_zFar);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cam_eyeX, cam_eyeY, cam_eyeZ,  
              cam_cX, cam_cY, cam_cZ,                 
              cam_upX, cam_upY, cam_upZ);

    glColor3f(0.5f, 0.5f, 0.5f);
    glutWireTorus(0.5, 1.5, 15, 30);
    draw_axes();

    // Viewport 2: Camera and Frustum
    glViewport(px_right, py_right, pw_right, ph_right);
    glScissor(px_right, py_right, pw_right, ph_right);
    glClearColor(0.102f, 0.071f, 0.149f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float debug_aspect = (float)pw_right / (float)ph_right;
    gluPerspective(45.0, debug_aspect, 0.1, std::max(100.0f, debug_distance * 5.0f));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float yaw_rad = debug_orbit_yaw * 3.14159f / 180.0f;
    float pitch_rad = debug_orbit_pitch * 3.14159f / 180.0f;
    float cam_x = debug_focus.x + debug_distance * std::cos(pitch_rad) * std::sin(yaw_rad);
    float cam_y = debug_focus.y + debug_distance * std::sin(pitch_rad);
    float cam_z = debug_focus.z + debug_distance * std::cos(pitch_rad) * std::cos(yaw_rad);
    gluLookAt(cam_x, cam_y, cam_z, 
              debug_focus.x, debug_focus.y, debug_focus.z, 
              0.0, 1.0, 0.0);

    glColor3f(0.5f, 0.5f, 0.5f);
    glutWireTorus(0.5, 1.5, 15, 30);
    draw_axes();
    draw_frustum();

    glPushMatrix();
    glTranslatef(cam_eyeX, cam_eyeY, cam_eyeZ);
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidCube(0.5f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(cam_cX, cam_cY, cam_cZ);
    glColor3f(1.0f, 0.0f, 0.0f);
    glutSolidSphere(0.2f, 10, 10);
    glPopMatrix();

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);

    glutSwapBuffers();
}

void projections_keyboard(unsigned char key) {
    unsigned char lower_key = (unsigned char)std::tolower(key);
    if (lower_key == 'q') {
        adjust_debug_zoom(-1.2f);
    } else if (lower_key == 'e') {
        adjust_debug_zoom(1.2f);
    } else if (lower_key == 'f') {
        debug_manual_zoom = 0.0f;
        update_debug_auto_frame(true);
    } else if (key == ',') {
        selected_var--;
        if (selected_var < 0) selected_var = MAX_VARS;
    } else if (key == '.') {
        selected_var++;
        if (selected_var > MAX_VARS) selected_var = 0;
    } 
    else if (key == '-' || key == '+' || key == '=') {
        float dir = (key == '-' ) ? -1.0f : 1.0f;
        
        if (selected_var == 0) {
            current_proj = (current_proj == PERSP) ? ORTHO : PERSP;
            update_debug_auto_frame(true);
        } else {
            float min_val, max_val;
            std::string label;
            float val;
            get_slider_limits_proj(selected_var, &min_val, &max_val, label, &val);

            float step = (max_val - min_val) * 0.02f;
            if (selected_var == 10) step = 0.05f; // custom step for aspect
            
            float new_val = val + dir * step;
            if (new_val < min_val) new_val = min_val;
            if (new_val > max_val) new_val = max_val;
            
            set_slider_val_proj(selected_var, new_val);
            clamp_proj_vars();
            update_debug_auto_frame(false);
        }
    }
    glutPostRedisplay();
}

void projections_special(int key) {
    if (key == GLUT_KEY_LEFT) {
        selected_var--;
        if (selected_var < 0) selected_var = MAX_VARS;
    } else if (key == GLUT_KEY_RIGHT) {
        selected_var++;
        if (selected_var > MAX_VARS) selected_var = 0;
    } else if (key == GLUT_KEY_DOWN || key == GLUT_KEY_UP) {
        projections_keyboard(key == GLUT_KEY_DOWN ? '-' : '+');
        return;
    }
    glutPostRedisplay();
}

void projections_mouse(int button, int state, int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // 1. Toggle button
            if (is_inside(mouse_pos.x, mouse_pos.y, 1310.0f, 765.0f, 1530.0f, 800.0f)) {
                current_proj = (current_proj == PERSP) ? ORTHO : PERSP;
                selected_var = 0;
                update_debug_auto_frame(true);
                glutPostRedisplay();
                return;
            }

            // 2. Sliders
            for (int idx = 1; idx <= 13; idx++) {
                float min_val, max_val;
                std::string label;
                float val;
                get_slider_limits_proj(idx, &min_val, &max_val, label, &val);

                float rail_y = 735.0f - (idx - 1) * 28.0f;

                if (mouse_pos.y >= rail_y - 22.0f && mouse_pos.y <= rail_y + 6.0f &&
                    mouse_pos.x >= 1355.0f && mouse_pos.x <= 1495.0f) {
                    selected_var = idx;
                    
                    float new_t = (mouse_pos.x - 1365.0f) / 120.0f;
                    if (new_t < 0.0f) new_t = 0.0f;
                    if (new_t > 1.0f) new_t = 1.0f;
                    set_slider_val_proj(idx, min_val + new_t * (max_val - min_val));
                    clamp_proj_vars();
                    update_debug_auto_frame(false);
                    glutPostRedisplay();
                    break;
                }
            }

            // 3. Right Viewport orbiting
            if (is_inside(mouse_pos.x, mouse_pos.y, 670.0f, 350.0f, 1270.0f, 850.0f)) {
                debug_orbiting = true;
                debug_last_mouse_x = x;
                debug_last_mouse_y = y;
            }
        } else if (state == GLUT_UP) {
            debug_orbiting = false;
        }
    } else if (state == GLUT_UP) {
        if (button == 3) {
            adjust_debug_zoom(-1.2f);
        } else if (button == 4) {
            adjust_debug_zoom(1.2f);
        }
    }
    glutPostRedisplay();
}

void projections_motion(int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);
    hover_mouse_x = mouse_pos.x;
    hover_mouse_y = mouse_pos.y;

    if (debug_orbiting) {
        int dx = x - debug_last_mouse_x;
        int dy = y - debug_last_mouse_y;
        debug_orbit_yaw += dx * 0.35f;
        debug_orbit_pitch += dy * 0.35f;
        if (debug_orbit_pitch < -82.0f) debug_orbit_pitch = -82.0f;
        if (debug_orbit_pitch > 82.0f) debug_orbit_pitch = 82.0f;

        debug_last_mouse_x = x;
        debug_last_mouse_y = y;
        glutPostRedisplay();
        return;
    }

    if (selected_var >= 1 && selected_var <= 13) {
        float min_val, max_val;
        std::string label;
        float val;
        get_slider_limits_proj(selected_var, &min_val, &max_val, label, &val);

        float t = (mouse_pos.x - 1365.0f) / 120.0f;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        set_slider_val_proj(selected_var, min_val + t * (max_val - min_val));
        clamp_proj_vars();
        update_debug_auto_frame(false);
        glutPostRedisplay();
    }
}

void projections_passive_motion(int x, int y) {
    Point2D mouse_pos = map_mouse_to_ortho(x, y);
    hover_mouse_x = mouse_pos.x;
    hover_mouse_y = mouse_pos.y;
    glutPostRedisplay();
}

void projections_update() {
    update_debug_auto_frame(false);
    debug_distance += (debug_target_distance - debug_distance) * 0.08f;
    debug_focus.x += (debug_target_focus.x - debug_focus.x) * 0.08f;
    debug_focus.y += (debug_target_focus.y - debug_focus.y) * 0.08f;
    debug_focus.z += (debug_target_focus.z - debug_focus.z) * 0.08f;
}
