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

static void draw_filled_rect(float x1, float y1, float x2, float y2, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void draw_view_label(float x, float y, const std::string& title, const std::string& subtitle) {
    draw_filled_rect(x, y, x + 250.0f, y + 46.0f, 0.145f, 0.090f, 0.204f);
    glColor3f(0.941f, 0.839f, 0.780f);
    draw_text(x + 12.0f, y + 19.0f, GLUT_BITMAP_HELVETICA_18, title);
    glColor3f(0.690f, 0.620f, 0.780f);
    draw_text(x + 12.0f, y + 36.0f, GLUT_BITMAP_HELVETICA_12, subtitle);
}

static void draw_section_title(float x, float y, const std::string& text) {
    glColor3f(0.690f, 0.620f, 0.780f);
    draw_text(x, y, GLUT_BITMAP_HELVETICA_12, text);
}

static void draw_value_text(float x, float y, int var_index, const std::string& text) {
    if (selected_var == var_index) {
        float text_w = 0.0f;
        for (char c : text) {
            text_w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        }
        draw_filled_rect(x - 8.0f, y - 17.0f, x + text_w + 8.0f, y + 8.0f, 0.451f, 0.192f, 0.271f);
        glColor3f(1.0f, 0.820f, 0.650f);
    } else {
        glColor3f(0.918f, 0.804f, 0.761f);
    }
    draw_text(x, y, GLUT_BITMAP_HELVETICA_18, text);
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

void projections_display() {
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    glDisable(GL_LIGHTING);
    glClearColor(0.102f, 0.071f, 0.149f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int panel_h = viewport_height * 0.34f;
    int header_h = 58;
    int gap = 14;
    int view_h = viewport_height - panel_h - header_h - gap;
    int view_w = (viewport_width - gap) / 2;

    glViewport(viewport_x, viewport_y + viewport_height - header_h, viewport_width, header_h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, viewport_width, header_h, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    draw_filled_rect(0.0f, 0.0f, viewport_width, header_h, 0.145f, 0.090f, 0.204f);
    glColor3f(0.941f, 0.839f, 0.780f);
    draw_text(28.0f, 25.0f, GLUT_BITMAP_HELVETICA_18, "Projecoes OpenGL");
    glColor3f(0.690f, 0.620f, 0.780f);
    draw_text(28.0f, 45.0f, GLUT_BITMAP_HELVETICA_12, "Compare a cena renderizada com a camera e o volume de recorte");
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(viewport_width - 430.0f, 35.0f, GLUT_BITMAP_HELVETICA_18, "ESC volta ao menu");

    glViewport(viewport_x, viewport_y + panel_h, view_w, view_h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, view_w, view_h, 0); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    draw_container(0.0f, 0.0f, view_w - 1.0f, view_h - 1.0f);
    draw_view_label(16.0f, 16.0f, "Cena projetada", current_proj == PERSP ? "gluPerspective ativo" : "glOrtho ativo");

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

    glViewport(viewport_x + view_w + gap, viewport_y + panel_h, view_w, view_h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, view_w, view_h, 0); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    draw_container(0.0f, 0.0f, view_w - 1.0f, view_h - 1.0f);
    draw_view_label(16.0f, 16.0f, "Camera e frustum", "visao externa da projecao");

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float debug_aspect = (float)view_w / (float)view_h;
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

    glViewport(viewport_x, viewport_y, viewport_width, panel_h);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, viewport_width, panel_h, 0); 
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    draw_filled_rect(0.0f, 0.0f, viewport_width, panel_h, 0.118f, 0.078f, 0.169f);
    draw_container(18.0f, 14.0f, viewport_width - 18.0f, panel_h - 14.0f);
    draw_filled_rect(18.0f, 14.0f, viewport_width - 18.0f, 58.0f, 0.169f, 0.106f, 0.231f);

    char buf[50];
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(34, 40, GLUT_BITMAP_HELVETICA_18, "Setas navegam/alteram | Q/E ou scroll zoom externo | F recentraliza | arraste a view direita para orbitar | ESC volta");

    std::string proj_str = (current_proj == PERSP) ? "gluPerspective" : "glOrtho";
    draw_section_title(34.0f, 84.0f, "PROJECAO ATIVA");
    draw_value_text(34.0f, 110.0f, 0, proj_str);

    glColor3f(0.918f, 0.804f, 0.761f);
    draw_section_title(245.0f, 84.0f, "gluLookAt");
    draw_text(245, 110, GLUT_BITMAP_HELVETICA_18, "Eye");
    
    snprintf(buf, sizeof(buf), "x %.1f", cam_eyeX); draw_value_text(245, 138, 1, buf);
    snprintf(buf, sizeof(buf), "y %.1f", cam_eyeY); draw_value_text(345, 138, 2, buf);
    snprintf(buf, sizeof(buf), "z %.1f", cam_eyeZ); draw_value_text(445, 138, 3, buf);
    
    glColor3f(0.918f, 0.804f, 0.761f); draw_text(560, 110, GLUT_BITMAP_HELVETICA_18, "Center");
    snprintf(buf, sizeof(buf), "x %.1f", cam_cX); draw_value_text(560, 138, 4, buf);
    snprintf(buf, sizeof(buf), "y %.1f", cam_cY); draw_value_text(660, 138, 5, buf);
    snprintf(buf, sizeof(buf), "z %.1f", cam_cZ); draw_value_text(760, 138, 6, buf);

    glColor3f(0.918f, 0.804f, 0.761f); draw_text(875, 110, GLUT_BITMAP_HELVETICA_18, "Up");
    snprintf(buf, sizeof(buf), "x %.1f", cam_upX); draw_value_text(875, 138, 7, buf);
    snprintf(buf, sizeof(buf), "y %.1f", cam_upY); draw_value_text(975, 138, 8, buf);
    snprintf(buf, sizeof(buf), "z %.1f", cam_upZ); draw_value_text(1075, 138, 9, buf);

    draw_section_title(34.0f, 176.0f, current_proj == PERSP ? "gluPerspective" : "glOrtho");
    if (current_proj == PERSP) {
        snprintf(buf, sizeof(buf), "fovy %.1f", p_fovy); draw_value_text(34, 205, 11, buf);
        snprintf(buf, sizeof(buf), "aspect %.2f", custom_aspect); draw_value_text(150, 205, 10, buf);
        snprintf(buf, sizeof(buf), "near %.1f", p_zNear); draw_value_text(285, 205, 12, buf);
        snprintf(buf, sizeof(buf), "far %.1f", p_zFar); draw_value_text(405, 205, 13, buf);
    } else {
        snprintf(buf, sizeof(buf), "aspect %.2f", custom_aspect); draw_value_text(34, 205, 10, buf);
        snprintf(buf, sizeof(buf), "size %.1f", o_size); draw_value_text(170, 205, 11, buf);
        snprintf(buf, sizeof(buf), "near %.1f", o_zNear); draw_value_text(285, 205, 12, buf);
        snprintf(buf, sizeof(buf), "far %.1f", o_zFar); draw_value_text(405, 205, 13, buf);
    }

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
        float speed = 0.5f;

        switch (selected_var) {
            case 0: 
                current_proj = (current_proj == PERSP) ? ORTHO : PERSP;
                selected_var = 0; 
                break;
            // -- View (Posição) --
            case 1: cam_eyeX += dir * speed; break;
            case 2: cam_eyeY += dir * speed; break;
            case 3: cam_eyeZ += dir * speed; break;
            // -- View (Alvo) --
            case 4: cam_cX += dir * speed; break;
            case 5: cam_cY += dir * speed; break;
            case 6: cam_cZ += dir * speed; break;
            // -- View (Up) --
            case 7: cam_upX += dir * 0.1f; break; 
            case 8: cam_upY += dir * 0.1f; break;
            case 9: cam_upZ += dir * 0.1f; break;
            // -- Matriz de Projeção --
            case 10: custom_aspect += dir * 0.05f; break; 
            case 11: 
                if (current_proj == PERSP) p_fovy += dir * 2.0f;
                else o_size += dir * speed; 
                break;
            case 12: 
                if (current_proj == PERSP) p_zNear += dir * speed;
                else o_zNear += dir * speed; 
                break;
            case 13: 
                if (current_proj == PERSP) p_zFar += dir * speed;
                else o_zFar += dir * speed; 
                break;
        }
        update_debug_auto_frame(false);
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
    int panel_h = viewport_height * 0.34f;
    int header_h = 58;
    int gap = 14;
    int view_h = viewport_height - panel_h - header_h - gap;
    int view_w = (viewport_width - gap) / 2;
    int right_x = viewport_x + view_w + gap;
    int right_y = viewport_y + header_h;
    bool inside_debug_view = x >= right_x && x <= right_x + view_w &&
                             y >= right_y && y <= right_y + view_h;

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN && inside_debug_view) {
            debug_orbiting = true;
            debug_last_mouse_x = x;
            debug_last_mouse_y = y;
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
    if (!debug_orbiting) return;

    int dx = x - debug_last_mouse_x;
    int dy = y - debug_last_mouse_y;
    debug_orbit_yaw += dx * 0.35f;
    debug_orbit_pitch += dy * 0.35f;
    if (debug_orbit_pitch < -82.0f) debug_orbit_pitch = -82.0f;
    if (debug_orbit_pitch > 82.0f) debug_orbit_pitch = 82.0f;

    debug_last_mouse_x = x;
    debug_last_mouse_y = y;
    glutPostRedisplay();
}

void projections_update() {
    update_debug_auto_frame(false);
    debug_distance += (debug_target_distance - debug_distance) * 0.08f;
    debug_focus.x += (debug_target_focus.x - debug_focus.x) * 0.08f;
    debug_focus.y += (debug_target_focus.y - debug_focus.y) * 0.08f;
    debug_focus.z += (debug_target_focus.z - debug_focus.z) * 0.08f;
}
