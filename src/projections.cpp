#include <GL/freeglut.h>
#include <string>
#include <vector>
#include <cmath>
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

    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);

    if (current_proj == PERSP) {
        float nearH = 2.0f * tan((p_fovy * 3.14159f / 180.0f) / 2.0f) * p_zNear;
        float nearW = nearH * custom_aspect;
        float farH  = 2.0f * tan((p_fovy * 3.14159f / 180.0f) / 2.0f) * p_zFar;
        float farW  = farH * custom_aspect;

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
    glClearColor(0.216f, 0.145f, 0.286f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int panel_h = viewport_height * 0.35f;
    int view_h = viewport_height - panel_h;
    int view_w = viewport_width / 2;

    glViewport(viewport_x, viewport_y + panel_h, view_w, view_h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, view_w, view_h, 0); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    draw_container(2.0f, 2.0f, view_w - 2.0f, view_h - 2.0f);

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

    glViewport(viewport_x + view_w, viewport_y + panel_h, view_w, view_h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, view_w, view_h, 0); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    draw_container(2.0f, 2.0f, view_w - 2.0f, view_h - 2.0f);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float debug_aspect = (float)view_w / (float)view_h;
    gluPerspective(45.0, debug_aspect, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(12.0, 10.0, 15.0, 
              0.0, 0.0, 0.0, 
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

    draw_container(10.0f, 10.0f, viewport_width - 10.0f, panel_h - 10.0f);

    auto set_text_color = [](int var_index) {
        if (selected_var == var_index) glColor3f(1.0f, 0.4f, 0.4f);
        else glColor3f(0.918f, 0.804f, 0.761f);
    };

    char buf[50];
    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(30, 35, GLUT_BITMAP_HELVETICA_18, "CONTROLES: ( < ) e ( > ) para navegar | ( - ) e ( + ) para alterar");

    set_text_color(0);
    std::string proj_str = (current_proj == PERSP) ? "gluPerspective" : "glOrtho";
    draw_text(30, 70, GLUT_BITMAP_HELVETICA_18, "Projecao Ativa: " + proj_str);

    glColor3f(0.918f, 0.804f, 0.761f);
    draw_text(30, 110, GLUT_BITMAP_HELVETICA_18, "gluLookAt( ");
    draw_text(130, 110, GLUT_BITMAP_HELVETICA_12, "Eye");
    
    set_text_color(1); snprintf(buf, sizeof(buf), "%.1f", cam_eyeX); draw_text(130, 135, GLUT_BITMAP_HELVETICA_18, buf);
    set_text_color(2); snprintf(buf, sizeof(buf), ", %.1f", cam_eyeY); draw_text(180, 135, GLUT_BITMAP_HELVETICA_18, buf);
    set_text_color(3); snprintf(buf, sizeof(buf), ", %.1f", cam_eyeZ); draw_text(240, 135, GLUT_BITMAP_HELVETICA_18, buf);
    
    glColor3f(0.918f, 0.804f, 0.761f); draw_text(310, 110, GLUT_BITMAP_HELVETICA_12, "Center");
    glColor3f(0.918f, 0.804f, 0.761f); draw_text(300, 135, GLUT_BITMAP_HELVETICA_18, ", ");
    set_text_color(4); snprintf(buf, sizeof(buf), "%.1f", cam_cX); draw_text(310, 135, GLUT_BITMAP_HELVETICA_18, buf);
    set_text_color(5); snprintf(buf, sizeof(buf), ", %.1f", cam_cY); draw_text(360, 135, GLUT_BITMAP_HELVETICA_18, buf);
    set_text_color(6); snprintf(buf, sizeof(buf), ", %.1f", cam_cZ); draw_text(420, 135, GLUT_BITMAP_HELVETICA_18, buf);

    glColor3f(0.918f, 0.804f, 0.761f); draw_text(490, 110, GLUT_BITMAP_HELVETICA_12, "Up");
    glColor3f(0.918f, 0.804f, 0.761f); draw_text(480, 135, GLUT_BITMAP_HELVETICA_18, ", ");
    set_text_color(7); snprintf(buf, sizeof(buf), "%.1f", cam_upX); draw_text(490, 135, GLUT_BITMAP_HELVETICA_18, buf);
    set_text_color(8); snprintf(buf, sizeof(buf), ", %.1f", cam_upY); draw_text(540, 135, GLUT_BITMAP_HELVETICA_18, buf);
    set_text_color(9); snprintf(buf, sizeof(buf), ", %.1f", cam_upZ); draw_text(600, 135, GLUT_BITMAP_HELVETICA_18, buf);
    
    glColor3f(0.918f, 0.804f, 0.761f); draw_text(660, 135, GLUT_BITMAP_HELVETICA_18, ");");

    if (current_proj == PERSP) {
        glColor3f(0.918f, 0.804f, 0.761f);
        draw_text(30, 185, GLUT_BITMAP_HELVETICA_18, "gluPerspective( ");
        
        set_text_color(11); snprintf(buf, sizeof(buf), "fovy: %.1f", p_fovy); draw_text(160, 185, GLUT_BITMAP_HELVETICA_18, buf);
        set_text_color(10); snprintf(buf, sizeof(buf), ", aspect: %.2f", custom_aspect); draw_text(260, 185, GLUT_BITMAP_HELVETICA_18, buf);
        set_text_color(12); snprintf(buf, sizeof(buf), ", near: %.1f", p_zNear); draw_text(410, 185, GLUT_BITMAP_HELVETICA_18, buf);
        set_text_color(13); snprintf(buf, sizeof(buf), ", far: %.1f", p_zFar); draw_text(520, 185, GLUT_BITMAP_HELVETICA_18, buf);
        
        glColor3f(0.918f, 0.804f, 0.761f); draw_text(630, 185, GLUT_BITMAP_HELVETICA_18, " );");
    } else {
        glColor3f(0.918f, 0.804f, 0.761f);
        draw_text(30, 185, GLUT_BITMAP_HELVETICA_18, "glOrtho( ");
        
        set_text_color(10); snprintf(buf, sizeof(buf), "aspect: %.2f", custom_aspect); draw_text(110, 185, GLUT_BITMAP_HELVETICA_18, buf);
        set_text_color(11); snprintf(buf, sizeof(buf), ", size: %.1f", o_size); draw_text(260, 185, GLUT_BITMAP_HELVETICA_18, buf);
        set_text_color(12); snprintf(buf, sizeof(buf), ", near: %.1f", o_zNear); draw_text(370, 185, GLUT_BITMAP_HELVETICA_18, buf);
        set_text_color(13); snprintf(buf, sizeof(buf), ", far: %.1f", o_zFar); draw_text(480, 185, GLUT_BITMAP_HELVETICA_18, buf);
        
        glColor3f(0.918f, 0.804f, 0.761f); draw_text(590, 185, GLUT_BITMAP_HELVETICA_18, " );");
    }

    glutSwapBuffers();
}

void projections_keyboard(unsigned char key) {
    if (key == ',') {
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
    }
    glutPostRedisplay();
}