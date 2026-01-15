#include <GL/freeglut.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdlib>

const float PI = 3.14159265f;
const float MOUSE_SENSITIVITY = 0.15f;

float moveSpeed = 0.015f;

float cameraPosX = 0.0f, cameraPosY = 3.0f, cameraPosZ = 10.0f;
float cameraYaw = 0.0f, cameraPitch = -15.0f;

int windowWidth = 1200, windowHeight = 800;

bool keysNormal[256] = {false};
bool keysSpecial[512] = {false};
bool mouseActive = false;
bool ignoreWarpEvent = false;

bool lightingEnabled = true;
int  lightMode = 1;
bool isDirectional = false;
bool isSpotlight = true;
float spotCutoff = 32.0f;
float spotExponent = 22.0f;
float linearAttenuation = 0.040f;
bool  isShiny = true;

float rackX = 0.0f;
float rackZ = 4.4f;

float chairsOffsetX = 0.0f;
float tableZ = 1.2f;

float windowOpen = 0.0f;
float doorAngle = 0.0f;

// ====== ROTASI MEJA (U / I)
float tableRotY = 0.0f;

// ====== SCALE KURSI (V / B)
float chairScale = 1.0f;

// lamp
const float LAMP_X = 0.0f;
const float LAMP_Y = 5.8f;
const float LAMP_Z = 0.0f;

// door dims (sinkron dengan drawDoorFront)
const float DOOR_W  = 1.25f;
const float DOOR_H  = 3.05f;
const float DOOR_CX = 6.6f;
const float DOOR_CY = 1.55f;
const float DOOR_CZ = 5.88f; // dekat dinding depan (+Z)

// window dims (sinkron dengan drawWindow)
const float WIN_CX = 0.0f;
const float WIN_CY = 3.3f;
const float WIN_CZ = -5.85f; // dekat dinding belakang (-Z)

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void applyLightingState() {
    if (lightingEnabled) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_NORMALIZE);
    } else {
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);
    }
}

void initLighting() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    GLfloat mat_specular[]  = {0.80f, 0.80f, 0.80f, 1.0f};
    GLfloat mat_shininess[] = {55.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void updateLightingLogic() {
    if (!lightingEnabled) {
        glDisable(GL_LIGHTING);
        return;
    }
    glEnable(GL_LIGHTING);

    GLfloat amb[]  = {0.18f, 0.17f, 0.16f, 1.0f};
    GLfloat dif[]  = {1.00f, 0.93f, 0.80f, 1.0f};
    GLfloat spec[] = {0.75f, 0.75f, 0.75f, 1.0f};
    GLfloat blk[]  = {0,0,0,1};

    if (lightMode == 1) {
        glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
    } else if (lightMode == 2) {
        glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  blk);
        glLightfv(GL_LIGHT0, GL_SPECULAR, blk);
    } else if (lightMode == 3) {
        glLightfv(GL_LIGHT0, GL_AMBIENT,  blk);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, blk);
    } else {
        glLightfv(GL_LIGHT0, GL_AMBIENT,  blk);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  blk);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
    }

    GLfloat w = isDirectional ? 0.0f : 1.0f;
    GLfloat light_pos[] = {LAMP_X, LAMP_Y, LAMP_Z, w};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    if (isSpotlight) {
        GLfloat spotDir[] = {0.0f, -1.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDir);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF,  spotCutoff);
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spotExponent);
    } else {
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);
    }

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    linearAttenuation);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.015f);

    if (isShiny) {
        GLfloat mat_spec[] = {0.80f, 0.80f, 0.80f, 1.0f};
        GLfloat shin[]     = {55.0f};
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
        glMaterialfv(GL_FRONT, GL_SHININESS, shin);
    } else {
        GLfloat mat_spec[] = {0.0f, 0.0f, 0.0f, 1.0f};
        GLfloat shin[]     = {1.0f};
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
        glMaterialfv(GL_FRONT, GL_SHININESS, shin);
    }
}

void drawScaledCube(float sx, float sy, float sz) {
    glPushMatrix();
    glScalef(sx, sy, sz);
    glutSolidCube(1.0);
    glPopMatrix();
}

static void drawDisk2D(float r, int seg) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    for (int i=0;i<=seg;i++){
        float a = 2.0f*PI*i/seg;
        glVertex3f((float)cos(a)*r, (float)sin(a)*r, 0.0f);
    }
    glEnd();
}

static void drawCylinder(float r0, float r1, float h, int slices) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluCylinder(q, r0, r1, h, slices, 2);

    glPushMatrix();
    glRotatef(180, 1,0,0);
    gluDisk(q, 0.0, r0, slices, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,0,h);
    gluDisk(q, 0.0, r1, slices, 1);
    glPopMatrix();

    gluDeleteQuadric(q);
}

static void drawWallSegment(float x1, float x2, float y1, float y2, float z, float thick,
                            float r, float g, float b) {
    float w = x2 - x1;
    float h = y2 - y1;
    if (w <= 0.001f || h <= 0.001f) return;
    glPushMatrix();
    glTranslatef((x1+x2)*0.5f, (y1+y2)*0.5f, z);
    glColor3f(r,g,b);
    drawScaledCube(w, h, thick);
    glPopMatrix();
}

void drawFloorWood() {
    glBegin(GL_QUADS);
    glNormal3f(0,1,0);
    glColor3f(0.44f, 0.28f, 0.16f);
    glVertex3f(-10, 0, -10);
    glVertex3f(-10, 0,  10);
    glVertex3f( 10, 0,  10);
    glVertex3f( 10, 0, -10);
    glEnd();

    glDisable(GL_LIGHTING);
    glColor3f(0.28f, 0.17f, 0.10f);
    glBegin(GL_LINES);
    for (float z = -10; z <= 10; z += 0.55f) {
        glVertex3f(-10, 0.01f, z);
        glVertex3f( 10, 0.01f, z);
    }
    glEnd();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// ====== WARNA DINDING BIRU CERAH
static const float WALL_R = 0.70f;
static const float WALL_G = 0.84f;
static const float WALL_B = 0.95f;

// ====== BACK WALL (bolong jendela + outside)
void drawWallPanelBack() {
    const float zWall = -6.0f;
    const float thick = 0.25f;
    const float xMin = -9.0f, xMax = 9.0f;
    const float yMin = 0.0f, yMax = 6.0f;

    const float holeW = 6.40f;
    const float holeH = 2.70f;

    float hx1 = WIN_CX - holeW*0.5f;
    float hx2 = WIN_CX + holeW*0.5f;
    float hy1 = WIN_CY - holeH*0.5f;
    float hy2 = WIN_CY + holeH*0.5f;

    drawWallSegment(xMin, hx1, yMin, yMax, zWall, thick, WALL_R, WALL_G, WALL_B);
    drawWallSegment(hx2, xMax, yMin, yMax, zWall, thick, WALL_R, WALL_G, WALL_B);
    drawWallSegment(hx1, hx2, hy2, yMax, zWall, thick, WALL_R, WALL_G, WALL_B);
    drawWallSegment(hx1, hx2, yMin, hy1, zWall, thick, WALL_R, WALL_G, WALL_B);

    // wainscot + baseboard
    glPushMatrix();
    glTranslatef(0.0f, 1.10f, -5.88f);
    glColor3f(0.88f, 0.90f, 0.92f);
    drawScaledCube(18.0f, 2.2f, 0.10f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.18f, -5.83f);
    glColor3f(0.55f, 0.40f, 0.25f);
    drawScaledCube(18.1f, 0.20f, 0.10f);
    glPopMatrix();

    // outside view di balik lubang (tetap)
    glDisable(GL_LIGHTING);
    float zOut = -6.85f;
    glColor3f(0.06f, 0.12f, 0.20f);
    glBegin(GL_QUADS);
        glVertex3f(hx1, hy2, zOut);
        glVertex3f(hx2, hy2, zOut);
        glVertex3f(hx2, yMax, zOut);
        glVertex3f(hx1, yMax, zOut);
    glEnd();
    glColor3f(0.14f, 0.12f, 0.10f);
    glBegin(GL_QUADS);
        glVertex3f(hx1, yMin, zOut);
        glVertex3f(hx2, yMin, zOut);
        glVertex3f(hx2, hy2, zOut);
        glVertex3f(hx1, hy2, zOut);
    glEnd();
    glColor3f(0.95f, 0.90f, 0.70f);
    glPushMatrix();
        glTranslatef(hx2 - 0.75f, yMax - 0.85f, zOut + 0.001f);
        drawDisk2D(0.22f, 40);
    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// ====== FRONT WALL (bolong pintu)  ==> OUTSIDE DI BELAKANG PINTU DIHILANGKAN
void drawWallPanelFront() {
    const float zWall = 6.0f;
    const float thick = 0.25f;
    const float xMin = -9.0f, xMax = 9.0f;
    const float yMin = 0.0f, yMax = 6.0f;

    const float holeW = DOOR_W + 0.55f;
    const float holeH = DOOR_H + 0.35f;

    float hx1 = DOOR_CX - holeW*0.5f;
    float hx2 = DOOR_CX + holeW*0.5f;
    float hy1 = DOOR_CY - holeH*0.5f;
    float hy2 = DOOR_CY + holeH*0.5f;
    hy1 = clampf(hy1, yMin, yMax);
    hy2 = clampf(hy2, yMin, yMax);

    drawWallSegment(xMin, hx1, yMin, yMax, zWall, thick, WALL_R, WALL_G, WALL_B);
    drawWallSegment(hx2, xMax, yMin, yMax, zWall, thick, WALL_R, WALL_G, WALL_B);
    drawWallSegment(hx1, hx2, hy2, yMax, zWall, thick, WALL_R, WALL_G, WALL_B);
    drawWallSegment(hx1, hx2, yMin, hy1, zWall, thick, WALL_R, WALL_G, WALL_B);

    // panel bawah + baseboard
    glPushMatrix();
    glTranslatef(0.0f, 1.10f, 5.88f);
    glColor3f(0.88f, 0.90f, 0.92f);
    drawScaledCube(18.0f, 2.2f, 0.10f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.18f, 5.83f);
    glColor3f(0.55f, 0.40f, 0.25f);
    drawScaledCube(18.1f, 0.20f, 0.10f);
    glPopMatrix();

    // outside view DIHAPUS
}

void drawWallPanelSides() {
    glPushMatrix();
    glTranslatef(-9.0f, 3.0f, 0.0f);
    glColor3f(WALL_R, WALL_G, WALL_B);
    drawScaledCube(0.25f, 6.0f, 18.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-8.88f, 1.10f, 0.0f);
    glColor3f(0.88f, 0.90f, 0.92f);
    drawScaledCube(0.10f, 2.2f, 18.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-8.83f, 0.18f, 0.0f);
    glColor3f(0.55f, 0.40f, 0.25f);
    drawScaledCube(0.10f, 0.20f, 18.1f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(9.0f, 3.0f, 0.0f);
    glColor3f(WALL_R, WALL_G, WALL_B);
    drawScaledCube(0.25f, 6.0f, 18.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(8.88f, 1.10f, 0.0f);
    glColor3f(0.88f, 0.90f, 0.92f);
    drawScaledCube(0.10f, 2.2f, 18.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(8.83f, 0.18f, 0.0f);
    glColor3f(0.55f, 0.40f, 0.25f);
    drawScaledCube(0.10f, 0.20f, 18.1f);
    glPopMatrix();
}

void drawCeilingNice() {
    glPushMatrix();
    glTranslatef(0.0f, 6.0f, 0.0f);
    glColor3f(0.78f, 0.78f, 0.80f);
    drawScaledCube(18.0f, 0.25f, 18.0f);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.65f, 0.63f, 0.60f);
    glBegin(GL_LINE_LOOP);
        glVertex3f(-9.0f, 5.88f, -6.0f);
        glVertex3f( 9.0f, 5.88f, -6.0f);
        glVertex3f( 9.0f, 5.88f,  6.0f);
        glVertex3f(-9.0f, 5.88f,  6.0f);
    glEnd();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

void drawWallPictureStyled(float x, float y, float z, float rotY, float w, float h) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotY, 0, 1, 0);

    glColor3f(0.30f, 0.18f, 0.09f);
    drawScaledCube(w, h, 0.12f);

    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.07f);

    glColor3f(0.94f, 0.92f, 0.88f);
    glBegin(GL_QUADS);
        glVertex3f(-(w*0.40f), -(h*0.38f), 0);
        glVertex3f( (w*0.40f), -(h*0.38f), 0);
        glVertex3f( (w*0.40f),  (h*0.38f), 0);
        glVertex3f(-(w*0.40f),  (h*0.38f), 0);
    glEnd();

    glColor3f(0.74f, 0.86f, 0.96f);
    glBegin(GL_QUADS);
        glVertex3f(-(w*0.40f), 0.0f, 0.001f);
        glVertex3f( (w*0.40f), 0.0f, 0.001f);
        glVertex3f( (w*0.40f), (h*0.38f), 0.001f);
        glVertex3f(-(w*0.40f), (h*0.38f), 0.001f);
    glEnd();

    glColor3f(0.72f, 0.78f, 0.70f);
    glBegin(GL_QUADS);
        glVertex3f(-(w*0.40f), -(h*0.38f), 0.001f);
        glVertex3f( (w*0.40f), -(h*0.38f), 0.001f);
        glVertex3f( (w*0.40f), 0.0f, 0.001f);
        glVertex3f(-(w*0.40f), 0.0f, 0.001f);
    glEnd();

    glColor3f(0.45f, 0.55f, 0.60f);
    glBegin(GL_TRIANGLES);
        glVertex3f(-(w*0.34f), -0.05f, 0.002f);
        glVertex3f(-(w*0.10f),  (h*0.26f), 0.002f);
        glVertex3f( (w*0.12f), -0.05f, 0.002f);

        glVertex3f(-(w*0.05f), -0.05f, 0.002f);
        glVertex3f( (w*0.18f),  (h*0.22f), 0.002f);
        glVertex3f( (w*0.35f), -0.05f, 0.002f);
    glEnd();

    glColor3f(0.25f, 0.45f, 0.70f);
    glBegin(GL_QUADS);
        glVertex3f(-(w*0.10f), -(h*0.38f), 0.003f);
        glVertex3f( (w*0.12f), -(h*0.38f), 0.003f);
        glVertex3f( (w*0.05f), -(h*0.05f), 0.003f);
        glVertex3f(-(w*0.02f), -(h*0.05f), 0.003f);
    glEnd();

    glPushMatrix();
    glTranslatef((w*0.22f), (h*0.18f), 0.004f);
    glColor3f(0.98f, 0.88f, 0.35f);
    drawDisk2D(0.16f, 40);
    glPopMatrix();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawRugOval() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 0.02f, 0.8f);

    glColor3f(0.90f, 0.86f, 0.78f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    int seg = 90;
    float rx = 4.2f, rz = 2.8f;
    for (int i=0;i<=seg;i++){
        float a = (2.0f*PI*i)/seg;
        glVertex3f((float)cos(a)*rx, 0, (float)sin(a)*rz);
    }
    glEnd();

    glColor3f(0.80f, 0.74f, 0.64f);
    glBegin(GL_LINE_LOOP);
    for (int i=0;i<seg;i++){
        float a = (2.0f*PI*i)/seg;
        glVertex3f((float)cos(a)*(rx-0.25f), 0.001f, (float)sin(a)*(rz-0.25f));
    }
    glEnd();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

void drawCoffeeTable(float x, float z, float rotYDeg) {
    const float topW = 2.6f, topD = 1.4f, topT = 0.12f;
    const float legH = 0.60f, legS = 0.12f;
    float topY  = legH + topT * 0.5f;

    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(rotYDeg, 0, 1, 0);

    glColor3f(0.56f, 0.36f, 0.20f);
    glPushMatrix(); glTranslatef(0, topY, 0); glScalef(topW, topT, topD); glutSolidCube(1.0); glPopMatrix();

    glColor3f(0.48f, 0.30f, 0.18f);
    glPushMatrix(); glTranslatef(0, topY-0.12f, 0); glScalef(topW-0.10f, 0.10f, topD-0.10f); glutSolidCube(1.0); glPopMatrix();

    float offsetX = (topW * 0.5f) - (legS * 0.5f) - 0.06f;
    float offsetZ = (topD * 0.5f) - (legS * 0.5f) - 0.06f;
    float legY    = legH * 0.5f;

    glColor3f(0.36f, 0.22f, 0.12f);
    glPushMatrix(); glTranslatef( offsetX, legY,  offsetZ); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-offsetX, legY,  offsetZ); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef( offsetX, legY, -offsetZ); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-offsetX, legY, -offsetZ); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();

    glPopMatrix();
}

void drawOneFlower(float fx, float fy, float fz, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(fx, fy, fz);

    glColor3f(r, g, b);
    for (int i = 0; i < 7; i++) {
        float a = 2.0f * PI * i / 7.0f;
        glPushMatrix();
        glTranslatef((float)cos(a) * 0.05f, (float)sin(a) * 0.05f, 0.0f);
        glutSolidSphere(0.032, 14, 14);
        glPopMatrix();
    }
    glColor3f(0.98f, 0.88f, 0.25f);
    glutSolidSphere(0.028, 14, 14);

    glPopMatrix();
}

void drawFlowerVaseRealistic(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    glColor3f(0.90f, 0.90f, 0.92f);
    glPushMatrix(); glRotatef(-90, 1,0,0); drawCylinder(0.16f, 0.11f, 0.34f, 28); glPopMatrix();

    glColor3f(0.86f, 0.86f, 0.88f);
    glPushMatrix();
        glTranslatef(0.0f, 0.31f, 0.0f);
        glRotatef(-90, 1,0,0);
        drawCylinder(0.10f, 0.11f, 0.06f, 28);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.18f, 0.55f, 0.25f);
    glBegin(GL_LINES);
        glVertex3f( 0.00f, 0.34f, 0.00f); glVertex3f( 0.06f, 0.74f, 0.03f);
        glVertex3f( 0.00f, 0.34f, 0.00f); glVertex3f(-0.05f, 0.72f,-0.02f);
        glVertex3f( 0.00f, 0.34f, 0.00f); glVertex3f( 0.02f, 0.66f,-0.06f);
    glEnd();

    glColor3f(0.14f, 0.48f, 0.22f);
    glPushMatrix(); glTranslatef( 0.03f, 0.54f, 0.02f); glutSolidSphere(0.045, 14,14); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.03f, 0.52f,-0.02f); glutSolidSphere(0.045, 14,14); glPopMatrix();

    drawOneFlower( 0.06f, 0.76f, 0.03f, 0.92f, 0.45f, 0.55f);
    drawOneFlower(-0.05f, 0.74f,-0.02f, 0.95f, 0.62f, 0.32f);
    drawOneFlower( 0.02f, 0.68f,-0.06f, 0.80f, 0.50f, 0.88f);
    drawOneFlower(-0.01f, 0.70f, 0.05f, 0.95f, 0.55f, 0.35f);

    if (lightingEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawChair(float x, float z, float rotYDeg) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(rotYDeg, 0,1,0);
    glScalef(chairScale, chairScale, chairScale);

    glColor3f(0.44f, 0.46f, 0.50f);
    glPushMatrix(); glTranslatef(0.0f, 0.55f, 0.0f); glScalef(2.0f, 0.35f, 1.3f); glutSolidCube(1.0); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 1.10f, -0.55f); glScalef(2.0f, 1.2f, 0.30f); glutSolidCube(1.0); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.95f, 0.75f, 0.0f); glScalef(0.25f, 0.55f, 1.25f); glutSolidCube(1.0); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.95f, 0.75f, 0.0f); glScalef(0.25f, 0.55f, 1.25f); glutSolidCube(1.0); glPopMatrix();

    glColor3f(0.82f, 0.78f, 0.70f);
    glPushMatrix(); glTranslatef(0.0f, 0.78f, -0.10f); glScalef(1.25f, 0.25f, 0.70f); glutSolidCube(1.0); glPopMatrix();

    glColor3f(0.18f, 0.12f, 0.09f);
    float legY = 0.25f, lx = 0.80f, lz = 0.50f, legS = 0.12f, legH = 0.50f;
    glPushMatrix(); glTranslatef( lx, legY,  lz); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-lx, legY,  lz); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef( lx, legY, -lz); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-lx, legY, -lz); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();

    glPopMatrix();
}

void drawShelf(float x, float z, float rotYDeg) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(rotYDeg, 0,1,0);

    glColor3f(0.56f, 0.36f, 0.20f);
    glPushMatrix(); glTranslatef(0.0f, 0.90f, 0.0f); glScalef(2.2f, 1.8f, 0.7f); glutSolidCube(1.0); glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.32f, 0.20f, 0.12f);
    glBegin(GL_LINES);
        glVertex3f(-1.05f, 1.20f, 0.36f); glVertex3f(1.05f, 1.20f, 0.36f);
        glVertex3f(-1.05f, 0.70f, 0.36f); glVertex3f(1.05f, 0.70f, 0.36f);
    glEnd();

    glPushMatrix(); glTranslatef(-0.7f, 0.65f, 0.38f); glColor3f(0.25f, 0.45f, 0.85f); glScalef(0.25f, 0.45f, 0.10f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.4f, 0.65f, 0.38f); glColor3f(0.85f, 0.35f, 0.25f); glScalef(0.25f, 0.40f, 0.10f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.1f, 0.65f, 0.38f); glColor3f(0.25f, 0.75f, 0.35f); glScalef(0.25f, 0.50f, 0.10f); glutSolidCube(1); glPopMatrix();

    if (lightingEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawTVOnRack(float rx, float rz, float rotYDeg) {
    float rackTopY = 1.80f;
    float tvHalfH  = 1.05f * 0.5f;

    float tvX = rx;
    float tvY = rackTopY + tvHalfH + 0.05f;
    float tvZ = rz - 0.02f;

    glPushMatrix();
    glTranslatef(tvX, tvY, tvZ);
    glRotatef(rotYDeg, 0,1,0);

    glColor3f(0.06f, 0.06f, 0.07f);
    glPushMatrix(); glScalef(1.8f, 1.05f, 0.12f); glutSolidCube(1.0); glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.10f, 0.18f, 0.25f);
    glBegin(GL_QUADS);
        glVertex3f(-0.80f, -0.45f, 0.07f);
        glVertex3f( 0.80f, -0.45f, 0.07f);
        glVertex3f( 0.80f,  0.45f, 0.07f);
        glVertex3f(-0.80f,  0.45f, 0.07f);
    glEnd();
    if (lightingEnabled) glEnable(GL_LIGHTING);

    glColor3f(0.06f, 0.06f, 0.07f);
    glPushMatrix(); glTranslatef(0.0f, -0.65f, 0.0f); glScalef(0.6f, 0.08f, 0.45f); glutSolidCube(1.0); glPopMatrix();

    glPopMatrix();
}

void drawSideTableAndPlant(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.56f, 0.36f, 0.20f);
    glPushMatrix(); glTranslatef(0, 0.75f, 0); glScalef(1.0f, 0.10f, 1.0f); glutSolidCube(1); glPopMatrix();

    glColor3f(0.36f, 0.22f, 0.12f);
    glPushMatrix(); glTranslatef(0.35f, 0.35f, 0.35f); glScalef(0.10f, 0.70f, 0.10f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.35f,0.35f, 0.35f); glScalef(0.10f, 0.70f, 0.10f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(0.35f, 0.35f,-0.35f); glScalef(0.10f, 0.70f, 0.10f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.35f,0.35f,-0.35f); glScalef(0.10f, 0.70f, 0.10f); glutSolidCube(1); glPopMatrix();

    glColor3f(0.50f, 0.32f, 0.18f);
    glPushMatrix(); glTranslatef(0.0f, 0.90f, 0.0f); glutSolidSphere(0.18, 20, 20); glPopMatrix();

    glColor3f(0.18f, 0.55f, 0.25f);
    glPushMatrix();
        glTranslatef(0.0f, 1.10f, 0.0f); glutSolidSphere(0.20, 20, 20);
        glTranslatef(0.12f, 0.10f, 0.0f); glutSolidSphere(0.16, 20, 20);
        glTranslatef(-0.24f, 0.00f, 0.0f); glutSolidSphere(0.16, 20, 20);
    glPopMatrix();

    glPopMatrix();
}

void drawWindow() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(WIN_CX, WIN_CY, WIN_CZ);

    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_QUADS);
        glVertex3f(-2.9f, -1.1f, 0);
        glVertex3f( 2.9f, -1.1f, 0);
        glVertex3f( 2.9f,  1.1f, 0);
        glVertex3f(-2.9f,  1.1f, 0);
    glEnd();

    float openShift = 1.15f * windowOpen;

    glColor3f(0.70f, 0.83f, 0.93f);

    glPushMatrix();
    glTranslatef(-openShift, 0.0f, 0.01f);
    glBegin(GL_QUADS);
        glVertex3f(-2.65f, -0.90f, 0.0f);
        glVertex3f( 0.00f, -0.90f, 0.0f);
        glVertex3f( 0.00f,  0.90f, 0.0f);
        glVertex3f(-2.65f,  0.90f, 0.0f);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(openShift, 0.0f, 0.01f);
    glBegin(GL_QUADS);
        glVertex3f( 0.00f, -0.90f, 0.0f);
        glVertex3f( 2.65f, -0.90f, 0.0f);
        glVertex3f( 2.65f,  0.90f, 0.0f);
        glVertex3f( 0.00f,  0.90f, 0.0f);
    glEnd();
    glPopMatrix();

    glColor3f(0.70f, 0.70f, 0.72f);
    glBegin(GL_LINES);
        glVertex3f(0, -0.90f, 0.02f); glVertex3f(0, 0.90f, 0.02f);
        glVertex3f(-2.65f, 0, 0.02f); glVertex3f(2.65f, 0, 0.02f);
    glEnd();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

void drawCurtains() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 3.25f, -5.83f);

    glColor3f(0.86f, 0.82f, 0.74f);
    glBegin(GL_QUADS);
        glVertex3f(-3.35f, -1.25f, 0.0f);
        glVertex3f(-2.95f, -1.25f, 0.0f);
        glVertex3f(-2.95f,  1.25f, 0.0f);
        glVertex3f(-3.35f,  1.25f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glVertex3f( 2.95f, -1.25f, 0.0f);
        glVertex3f( 3.35f, -1.25f, 0.0f);
        glVertex3f( 3.35f,  1.25f, 0.0f);
        glVertex3f( 2.95f,  1.25f, 0.0f);
    glEnd();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

void drawWallClockBack() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 4.95f, -5.86f);

    glColor3f(0.25f, 0.18f, 0.12f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0,0,0);
        for (int i=0;i<=50;i++){
            float a = 2.0f*PI*i/50.0f;
            glVertex3f((float)cos(a)*0.32f, (float)sin(a)*0.32f, 0.0f);
        }
    glEnd();

    glColor3f(0.95f, 0.94f, 0.92f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0,0,0.001f);
        for (int i=0;i<=50;i++){
            float a = 2.0f*PI*i/50.0f;
            glVertex3f((float)cos(a)*0.27f, (float)sin(a)*0.27f, 0.001f);
        }
    glEnd();

    glColor3f(0.10f, 0.10f, 0.10f);
    glBegin(GL_LINES);
        glVertex3f(0,0,0.01f); glVertex3f(0.0f, 0.17f, 0.01f);
        glVertex3f(0,0,0.01f); glVertex3f(0.14f,-0.05f, 0.01f);
    glEnd();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

void drawCornerPlantBackRight() {
    glPushMatrix();
    glTranslatef(7.8f, 0.0f, -4.8f);

    glColor3f(0.45f, 0.28f, 0.16f);
    glPushMatrix();
        glTranslatef(0.0f, 0.35f, 0.0f);
        glRotatef(-90, 1,0,0);
        GLUquadric* q = gluNewQuadric();
        gluCylinder(q, 0.28, 0.22, 0.40, 22, 2);
        gluDeleteQuadric(q);
    glPopMatrix();

    glColor3f(0.16f, 0.52f, 0.25f);
    glPushMatrix(); glTranslatef(0.0f, 0.85f, 0.0f); glutSolidSphere(0.26, 18,18); glPopMatrix();
    glPushMatrix(); glTranslatef(0.18f, 0.80f, 0.05f); glutSolidSphere(0.22, 18,18); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.18f,0.78f,-0.05f); glutSolidSphere(0.22, 18,18); glPopMatrix();
    glPushMatrix(); glTranslatef(0.08f, 1.05f,-0.08f); glutSolidSphere(0.18, 18,18); glPopMatrix();

    glPopMatrix();
}

void drawCeilingLampStatic() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(LAMP_X, LAMP_Y, LAMP_Z);

    glColor3f(0.10f, 0.10f, 0.10f);
    glBegin(GL_LINES);
        glVertex3f(0, 0.8f, 0);
        glVertex3f(0, 0.0f, 0);
    glEnd();

    glColor3f(0.93f, 0.93f, 0.91f);
    glPushMatrix();
        glTranslatef(0.0f, -0.1f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(0.6, 0.6, 26, 14);
    glPopMatrix();

    glColor3f(0.25f, 0.25f, 0.25f);
    glPushMatrix();
        glTranslatef(0.0f, -0.20f, 0.0f);
        glScalef(0.12f, 0.22f, 0.12f);
        glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(1.0f, 0.96f, 0.75f);
    glPushMatrix();
        glTranslatef(0.0f, -0.55f, 0.0f);
        glutSolidSphere(0.16, 18, 18);
    glPopMatrix();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

void drawLightBeam() {
    if (!lightingEnabled) return;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    glTranslatef(LAMP_X, LAMP_Y - 0.55f, LAMP_Z);
    glRotatef(-90, 1,0,0);

    glColor4f(1.0f, 0.95f, 0.75f, 0.16f);
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, 0.10, 2.30, 5.30, 28, 1);
    gluDeleteQuadric(q);

    glPopMatrix();

    glDisable(GL_BLEND);
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

void drawLightSourceMarker() {
    if (!lightingEnabled) return;
    glDisable(GL_LIGHTING);
    glPushMatrix();
        glTranslatef(LAMP_X, LAMP_Y, LAMP_Z);
        glColor3f(1.0f, 0.95f, 0.80f);
        glutSolidSphere(0.12, 16, 16);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawDoorFront() {
    const float doorW = DOOR_W;
    const float doorH = DOOR_H;
    const float doorT = 0.10f;

    float doorCX = DOOR_CX;
    float doorCY = DOOR_CY;
    float doorCZ = DOOR_CZ;

    glPushMatrix();
    glTranslatef(doorCX, doorCY, doorCZ);

    glColor3f(0.92f, 0.88f, 0.82f);
    glPushMatrix(); glScalef(doorW + 0.35f, doorH + 0.30f, 0.10f); glutSolidCube(1.0f); glPopMatrix();

    glColor3f(0.82f, 0.76f, 0.68f);
    glPushMatrix(); glTranslatef(0.0f, -doorH*0.52f, 0.03f); glScalef(doorW + 0.40f, 0.18f, 0.08f); glutSolidCube(1.0f); glPopMatrix();

    glPushMatrix();
        glTranslatef(-doorW*0.5f, 0.0f, 0.02f);
        glRotatef(doorAngle, 0, 1, 0);
        glTranslatef(doorW*0.5f, 0.0f, 0.0f);

        glColor3f(0.34f, 0.20f, 0.12f);
        glPushMatrix();
            glScalef(doorW, doorH, doorT);
            glutSolidCube(1.0f);
        glPopMatrix();

        glDisable(GL_LIGHTING);
        glColor3f(0.22f, 0.13f, 0.08f);

        float zFace = doorT*0.5f + 0.001f;
        glBegin(GL_LINES);
            float bx = doorW*0.42f, by = doorH*0.45f;
            glVertex3f(-bx, -by, zFace); glVertex3f( bx, -by, zFace);
            glVertex3f( bx, -by, zFace); glVertex3f( bx,  by, zFace);
            glVertex3f( bx,  by, zFace); glVertex3f(-bx,  by, zFace);
            glVertex3f(-bx,  by, zFace); glVertex3f(-bx, -by, zFace);
        glEnd();

        glPushMatrix();
            glTranslatef(doorW*0.33f, 0.0f, zFace + 0.01f);
            glColor3f(0.85f, 0.78f, 0.55f);
            glutSolidSphere(0.08, 14, 14);
        glPopMatrix();

        if (lightingEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();

    glPopMatrix();
}

void updateCamera() {
    float yawRad   = cameraYaw   * PI / 180.0f;
    float pitchRad = cameraPitch * PI / 180.0f;

    float forwardX = (float)sin(yawRad) * (float)cos(pitchRad);
    float forwardY = -(float)sin(pitchRad);
    float forwardZ = -(float)cos(yawRad) * (float)cos(pitchRad);

    float rightX = (float)sin(yawRad + PI/2.0f);
    float rightZ = -(float)cos(yawRad + PI/2.0f);

    if (keysNormal['w'] || keysNormal['W']) {
        cameraPosX += forwardX * moveSpeed;
        cameraPosY += forwardY * moveSpeed;
        cameraPosZ += forwardZ * moveSpeed;
    }
    if (keysNormal['s'] || keysNormal['S']) {
        cameraPosX -= forwardX * moveSpeed;
        cameraPosY -= forwardY * moveSpeed;
        cameraPosZ -= forwardZ * moveSpeed;
    }
    if (keysNormal['a'] || keysNormal['A']) {
        cameraPosX -= rightX * moveSpeed;
        cameraPosZ -= rightZ * moveSpeed;
    }
    if (keysNormal['d'] || keysNormal['D']) {
        cameraPosX += rightX * moveSpeed;
        cameraPosZ += rightZ * moveSpeed;
    }

    if (keysNormal[' ']) cameraPosY += moveSpeed;
    if (keysNormal['c'] || keysNormal['C']) cameraPosY -= moveSpeed;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)windowWidth / (double)windowHeight, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float yawRad   = cameraYaw   * PI / 180.0f;
    float pitchRad = cameraPitch * PI / 180.0f;

    float lookX = cameraPosX + (float)sin(yawRad) * (float)cos(pitchRad);
    float lookY = cameraPosY - (float)sin(pitchRad);
    float lookZ = cameraPosZ - (float)cos(yawRad) * (float)cos(pitchRad);

    gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
              lookX, lookY, lookZ,
              0.0f, 1.0f, 0.0f);

    updateLightingLogic();

    drawFloorWood();
    drawWallPanelBack();
    drawWallPanelFront();
    drawWallPanelSides();
    drawCeilingNice();

    drawRugOval();
    drawWindow();
    drawCurtains();
    drawWallClockBack();
    drawCornerPlantBackRight();

    drawCeilingLampStatic();
    drawLightBeam();

    drawWallPictureStyled(-7.7f, 3.4f, -5.85f, 0.0f, 2.2f, 1.6f);
    drawWallPictureStyled( 7.7f, 3.4f, -5.85f, 0.0f, 2.2f, 1.6f);

    drawDoorFront();

    drawCoffeeTable(0.0f, tableZ, tableRotY);
    drawFlowerVaseRealistic(0.0f, 0.78f, tableZ);

    drawChair(-3.2f + chairsOffsetX, -0.5f, 0.0f);
    drawChair( 3.2f + chairsOffsetX, -0.5f, 0.0f);

    drawShelf(rackX, rackZ, 180.0f);
    drawTVOnRack(rackX, rackZ, 180.0f);

    drawSideTableAndPlant(-6.0f, -4.8f);

    drawLightSourceMarker();
    glutSwapBuffers();
}

void keyboardDown(unsigned char key, int, int) {
    keysNormal[key] = true;

    if (key == 27) std::exit(0);

    if (key == 'l' || key == 'L') {
        lightingEnabled = !lightingEnabled;
        applyLightingState();
    }

    if (key == '+' || key == '=') moveSpeed += 0.005f;
    if (key == '-' || key == '_') {
        moveSpeed -= 0.005f;
        if (moveSpeed < 0.005f) moveSpeed = 0.005f;
    }

    const float step = 0.25f;

    // rak + tv (TFGH)
    if (key == 't' || key == 'T') rackZ += step;
    if (key == 'g' || key == 'G') rackZ -= step;
    if (key == 'f' || key == 'F') rackX -= step;
    if (key == 'h' || key == 'H') rackX += step;
    rackX = clampf(rackX, -7.5f, 7.5f);
    rackZ = clampf(rackZ, -5.0f,  5.6f);

    // kursi geser (Z/X)
    if (key == 'z' || key == 'Z') chairsOffsetX -= step;
    if (key == 'x' || key == 'X') chairsOffsetX += step;
    chairsOffsetX = clampf(chairsOffsetX, -3.0f, 3.0f);

    // meja maju mundur (N/M)
    if (key == 'n' || key == 'N') tableZ -= step;
    if (key == 'm' || key == 'M') tableZ += step;
    tableZ = clampf(tableZ, -1.0f, 3.2f);

    // jendela (O/P)
    if (key == 'o' || key == 'O') windowOpen = clampf(windowOpen + 0.10f, 0.0f, 1.0f);
    if (key == 'p' || key == 'P') windowOpen = clampf(windowOpen - 0.10f, 0.0f, 1.0f);

    // pintu (Q/E)
    if (key == 'q' || key == 'Q') doorAngle = clampf(doorAngle + 5.0f, 0.0f, 90.0f);
    if (key == 'e' || key == 'E') doorAngle = clampf(doorAngle - 5.0f, 0.0f, 90.0f);

    // rotasi meja (U/I)
    if (key == 'u' || key == 'U') tableRotY += 5.0f;
    if (key == 'i' || key == 'I') tableRotY -= 5.0f;
    if (tableRotY > 360.0f) tableRotY -= 360.0f;
    if (tableRotY < 0.0f)   tableRotY += 360.0f;

    // scale kursi (V besar, B kecil)
    if (key == 'v' || key == 'V') chairScale = clampf(chairScale + 0.10f, 0.60f, 1.60f);
    if (key == 'b' || key == 'B') chairScale = clampf(chairScale - 0.10f, 0.60f, 1.60f);

    // RESET MEJA SAJA (R)
    if (key == 'r' || key == 'R') {
        tableZ    = 1.2f;
        tableRotY = 0.0f;
    }

    // ====== RESET KURSI (K) -> balik scale + posisi geser kursi
    if (key == 'k' || key == 'K') {
        chairScale     = 1.0f;
        chairsOffsetX  = 0.0f;
    }
}

void keyboardUp(unsigned char key, int, int) { keysNormal[key] = false; }

void specialDown(int key, int, int) { if (key >= 0 && key < 512) keysSpecial[key] = true; }
void specialUp(int key, int, int)   { if (key >= 0 && key < 512) keysSpecial[key] = false; }

void mouseMotion(int x, int y) {
    if (!mouseActive) return;
    if (ignoreWarpEvent) { ignoreWarpEvent = false; return; }

    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    int dx = x - centerX;
    int dy = y - centerY;

    cameraYaw   += dx * MOUSE_SENSITIVITY;
    cameraPitch += dy * MOUSE_SENSITIVITY;
    cameraPitch = clampf(cameraPitch, -89.0f, 89.0f);

    ignoreWarpEvent = true;
    glutWarpPointer(centerX, centerY);
}

void mouseButton(int button, int state, int, int) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouseActive = true;
        glutSetCursor(GLUT_CURSOR_NONE);
        ignoreWarpEvent = true;
        glutWarpPointer(windowWidth/2, windowHeight/2);
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        mouseActive = false;
        glutSetCursor(GLUT_CURSOR_INHERIT);
    }
}

void idle() { updateCamera(); glutPostRedisplay(); }

void reshape(int w, int h) {
    windowWidth  = (w <= 1) ? 1 : w;
    windowHeight = (h <= 1) ? 1 : h;
    glViewport(0, 0, windowWidth, windowHeight);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Ruang Tamu");
    glutFullScreen();

    glClearColor(0.07f, 0.07f, 0.08f, 1.0f);
    initLighting();
    applyLightingState();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);

    glutMouseFunc(mouseButton);
    glutPassiveMotionFunc(mouseMotion);
    glutMotionFunc(mouseMotion);

    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}

