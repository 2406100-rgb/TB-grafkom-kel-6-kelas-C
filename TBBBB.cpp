#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>

// ============================= Konstanta & Global
const float PI = 3.14159265f;
const float MOVE_SPEED = 0.05f;
const float MOUSE_SENSITIVITY = 0.15f;

float cameraPosX = 0.0f, cameraPosY = 3.0f, cameraPosZ = 10.0f;
float cameraYaw = 0.0f, cameraPitch = -15.0f;

int windowWidth = 1200, windowHeight = 800;

// Input
bool keysNormal[256] = {false};
bool keysSpecial[512] = {false};
bool mouseActive = false;
bool ignoreWarpEvent = false;

// Lighting
bool lightingEnabled = true;
int  lightMode = 1;
bool isDirectional = false;
bool isSpotlight = false;
float spotCutoff = 50.0f;
float spotExponent = 8.0f;
float linearAttenuation = 0.03f;
bool  isShiny = true;

// ============================= Utility
static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ============================= Lighting
void initLighting() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    GLfloat mat_specular[]  = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat mat_shininess[] = {30.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glEnable(GL_NORMALIZE);
}

void updateLightingLogic() {
    if (!lightingEnabled) {
        glDisable(GL_LIGHTING);
        return;
    }
    glEnable(GL_LIGHTING);

    GLfloat amb[]  = {0.18f, 0.16f, 0.14f, 1.0f};
    GLfloat dif[]  = {0.95f, 0.88f, 0.75f, 1.0f};
    GLfloat spec[] = {0.70f, 0.70f, 0.70f, 1.0f};
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

    // posisi lampu di atas tengah ruangan
    GLfloat w = isDirectional ? 0.0f : 1.0f;
    GLfloat light_pos[] = {0.0f, 6.0f, 0.0f, w};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    if (isSpotlight) {
        GLfloat spotDir[] = {0.0f, -1.0f, 0.0f}; // arah ke bawah
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDir);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF,  spotCutoff);
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spotExponent);
    } else {
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);
    }

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    linearAttenuation);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01f);

    if (isShiny) {
        GLfloat mat_spec[] = {0.65f, 0.65f, 0.65f, 1.0f};
        GLfloat shin[]     = {35.0f};
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
        glMaterialfv(GL_FRONT, GL_SHININESS, shin);
    } else {
        GLfloat mat_spec[] = {0.0f, 0.0f, 0.0f, 1.0f};
        GLfloat shin[]     = {1.0f};
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
        glMaterialfv(GL_FRONT, GL_SHININESS, shin);
    }
}

// ============================= Primitive helper
void drawScaledCube(float sx, float sy, float sz) {
    glPushMatrix();
    glScalef(sx, sy, sz);
    glutSolidCube(1.0);
    glPopMatrix();
}

// ============================= Lantai kayu
void drawFloorWood() {
    glBegin(GL_QUADS);
    glNormal3f(0,1,0);
    glColor3f(0.55f, 0.38f, 0.22f);
    glVertex3f(-10, 0, -10);
    glVertex3f(-10, 0,  10);
    glVertex3f( 10, 0,  10);
    glVertex3f( 10, 0, -10);
    glEnd();

    glDisable(GL_LIGHTING);
    glColor3f(0.42f, 0.28f, 0.16f);
    glBegin(GL_LINES);
    for (float z = -10; z <= 10; z += 0.6f) {
        glVertex3f(-10, 0.01f, z);
        glVertex3f( 10, 0.01f, z);
    }
    glEnd();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// ============================= Dinding belakang
void drawBackWallWarm() {
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, -6.0f);
    glColor3f(0.93f, 0.90f, 0.84f);
    drawScaledCube(18.0f, 6.0f, 0.25f);
    glPopMatrix();
}

// ============================= DINDING SAMPING KIRI & KANAN
void drawSideWallsWarm() {
    // Dinding kiri (x = -9)
    glPushMatrix();
    glTranslatef(-9.0f, 3.0f, 0.0f);
    glColor3f(0.92f, 0.89f, 0.83f);
    drawScaledCube(0.25f, 6.0f, 18.0f);
    glPopMatrix();

    // Dinding kanan (x = +9)
    glPushMatrix();
    glTranslatef(9.0f, 3.0f, 0.0f);
    glColor3f(0.92f, 0.89f, 0.83f);
    drawScaledCube(0.25f, 6.0f, 18.0f);
    glPopMatrix();
}

// ============================= Karpet oval
void drawRugOval() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 0.02f, 0.8f);

    glColor3f(0.86f, 0.82f, 0.74f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    int seg = 90;
    float rx = 4.2f, rz = 2.8f;
    for (int i=0;i<=seg;i++){
        float a = (2.0f*PI*i)/seg;
        glVertex3f(std::cos(a)*rx, 0, std::sin(a)*rz);
    }
    glEnd();

    glColor3f(0.78f, 0.72f, 0.63f);
    glBegin(GL_LINE_LOOP);
    for (int i=0;i<seg;i++){
        float a = (2.0f*PI*i)/seg;
        glVertex3f(std::cos(a)*(rx-0.25f), 0.001f, std::sin(a)*(rz-0.25f));
    }
    glEnd();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// ============================= Meja
void drawCoffeeTable(float x, float z) {
    const float topW = 2.6f, topD = 1.4f, topT = 0.12f;
    const float legH = 0.60f, legS = 0.12f;

    float baseY = 0.0f;
    float topY  = baseY + legH + topT * 0.5f;

    glPushMatrix();
    glTranslatef(x, 0, z);

    glColor3f(0.60f, 0.40f, 0.20f);
    glPushMatrix();
    glTranslatef(0, topY, 0);
    glScalef(topW, topT, topD);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.52f, 0.34f, 0.18f);
    glPushMatrix();
    glTranslatef(0, topY-0.12f, 0);
    glScalef(topW-0.10f, 0.10f, topD-0.10f);
    glutSolidCube(1.0);
    glPopMatrix();

    float offsetX = (topW * 0.5f) - (legS * 0.5f) - 0.06f;
    float offsetZ = (topD * 0.5f) - (legS * 0.5f) - 0.06f;
    float legY    = baseY + legH * 0.5f;

    glColor3f(0.45f, 0.28f, 0.14f);

    glPushMatrix(); glTranslatef( offsetX, legY,  offsetZ); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-offsetX, legY,  offsetZ); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef( offsetX, legY, -offsetZ); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-offsetX, legY, -offsetZ); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();

    glPopMatrix();
}

// ============================= Vas bunga di atas meja
void drawFlowerVase(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // vas
    glColor3f(0.85f, 0.85f, 0.88f);
    glPushMatrix();
    glScalef(0.35f, 0.45f, 0.35f);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    // leher vas
    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.0f);
    glScalef(0.20f, 0.25f, 0.20f);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();

    // batang + daun + bunga (tanpa lighting biar warna keluar)
    glDisable(GL_LIGHTING);

    glColor3f(0.20f, 0.55f, 0.25f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.45f, 0.0f);
    glVertex3f(0.0f, 0.85f, 0.0f);
    glEnd();

    glPushMatrix();
    glTranslatef(0.10f, 0.65f, 0.0f);
    glutSolidSphere(0.07, 10, 10);
    glTranslatef(-0.20f, 0.05f, 0.0f);
    glutSolidSphere(0.07, 10, 10);
    glPopMatrix();

    glColor3f(0.90f, 0.40f, 0.45f);
    glPushMatrix();
    glTranslatef(0.0f, 0.90f, 0.0f);
    glutSolidSphere(0.12, 16, 16);
    glPopMatrix();

    if (lightingEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();
}

// ============================= Kursi
void drawChair(float x, float z, float rotYDeg) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(rotYDeg, 0,1,0);

    glColor3f(0.62f, 0.66f, 0.72f);

    glPushMatrix();
    glTranslatef(0.0f, 0.55f, 0.0f);
    glScalef(2.0f, 0.35f, 1.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 1.10f, -0.55f);
    glScalef(2.0f, 1.2f, 0.30f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.95f, 0.75f, 0.0f);
    glScalef(0.25f, 0.55f, 1.25f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.95f, 0.75f, 0.0f);
    glScalef(0.25f, 0.55f, 1.25f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.80f, 0.78f, 0.72f);
    glPushMatrix();
    glTranslatef(0.0f, 0.78f, -0.10f);
    glScalef(1.2f, 0.25f, 0.65f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.28f, 0.18f, 0.10f);
    float legY = 0.25f;
    float lx = 0.80f, lz = 0.50f;
    float legS = 0.12f, legH = 0.50f;

    glPushMatrix(); glTranslatef( lx, legY,  lz); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-lx, legY,  lz); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef( lx, legY, -lz); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-lx, legY, -lz); glScalef(legS, legH, legS); glutSolidCube(1); glPopMatrix();

    glPopMatrix();
}

// ============================= Rak + buku
void drawShelf(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.62f, 0.42f, 0.22f);
    glPushMatrix();
    glTranslatef(0.0f, 0.90f, 0.0f);
    glScalef(2.2f, 1.8f, 0.7f);
    glutSolidCube(1.0);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.40f, 0.25f, 0.14f);
    glBegin(GL_LINES);
    glVertex3f(-1.05f, 1.20f, 0.36f); glVertex3f(1.05f, 1.20f, 0.36f);
    glVertex3f(-1.05f, 0.70f, 0.36f); glVertex3f(1.05f, 0.70f, 0.36f);
    glEnd();

    glPushMatrix(); glTranslatef(-0.7f, 0.65f, 0.38f);
    glColor3f(0.25f, 0.45f, 0.85f);
    glScalef(0.25f, 0.45f, 0.10f); glutSolidCube(1); glPopMatrix();

    glPushMatrix(); glTranslatef(-0.4f, 0.65f, 0.38f);
    glColor3f(0.85f, 0.35f, 0.25f);
    glScalef(0.25f, 0.40f, 0.10f); glutSolidCube(1); glPopMatrix();

    glPushMatrix(); glTranslatef(-0.1f, 0.65f, 0.38f);
    glColor3f(0.25f, 0.75f, 0.35f);
    glScalef(0.25f, 0.50f, 0.10f); glutSolidCube(1); glPopMatrix();

    if (lightingEnabled) glEnable(GL_LIGHTING);

    glPopMatrix();
}

// ============================= TV
void drawTV(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 1.85f, z);

    glColor3f(0.06f, 0.06f, 0.07f);
    glPushMatrix();
    glScalef(1.8f, 1.05f, 0.12f);
    glutSolidCube(1.0);
    glPopMatrix();

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
    glPushMatrix();
    glTranslatef(0.0f, -0.65f, 0.0f);
    glScalef(0.6f, 0.08f, 0.45f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

// ============================= meja kecil + tanaman
void drawSideTableAndPlant(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.60f, 0.40f, 0.20f);
    glPushMatrix(); glTranslatef(0, 0.75f, 0); glScalef(1.0f, 0.10f, 1.0f); glutSolidCube(1); glPopMatrix();
    glColor3f(0.45f, 0.28f, 0.14f);
    glPushMatrix(); glTranslatef(0.35f, 0.35f, 0.35f); glScalef(0.10f, 0.70f, 0.10f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.35f,0.35f, 0.35f); glScalef(0.10f, 0.70f, 0.10f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(0.35f, 0.35f,-0.35f); glScalef(0.10f, 0.70f, 0.10f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.35f,0.35f,-0.35f); glScalef(0.10f, 0.70f, 0.10f); glutSolidCube(1); glPopMatrix();

    glColor3f(0.55f, 0.35f, 0.20f);
    glPushMatrix();
    glTranslatef(0.0f, 0.90f, 0.0f);
    glutSolidSphere(0.18, 20, 20);
    glPopMatrix();

    glColor3f(0.18f, 0.55f, 0.25f);
    glPushMatrix();
    glTranslatef(0.0f, 1.10f, 0.0f);
    glutSolidSphere(0.20, 20, 20);
    glTranslatef(0.12f, 0.10f, 0.0f); glutSolidSphere(0.16, 20, 20);
    glTranslatef(-0.24f, 0.00f, 0.0f); glutSolidSphere(0.16, 20, 20);
    glPopMatrix();

    glPopMatrix();
}

// ============================= jendela
void drawWindow() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 3.3f, -5.85f);

    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_QUADS);
    glVertex3f(-2.9f, -1.1f, 0);
    glVertex3f( 2.9f, -1.1f, 0);
    glVertex3f( 2.9f,  1.1f, 0);
    glVertex3f(-2.9f,  1.1f, 0);
    glEnd();

    glColor3f(0.70f, 0.83f, 0.93f);
    glBegin(GL_QUADS);
    glVertex3f(-2.65f, -0.90f, 0.01f);
    glVertex3f( 2.65f, -0.90f, 0.01f);
    glVertex3f( 2.65f,  0.90f, 0.01f);
    glVertex3f(-2.65f,  0.90f, 0.01f);
    glEnd();

    glColor3f(0.70f, 0.70f, 0.72f);
    glBegin(GL_LINES);
    glVertex3f(0, -0.90f, 0.02f); glVertex3f(0, 0.90f, 0.02f);
    glVertex3f(-2.65f, 0, 0.02f); glVertex3f(2.65f, 0, 0.02f);
    glEnd();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// ============================= Lampu gantung (mengarah ke bawah)
void drawCeilingLamp() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 5.8f, 0.0f);

    glColor3f(0.12f, 0.12f, 0.12f);
    glBegin(GL_LINES);
    glVertex3f(0, 0.8f, 0);
    glVertex3f(0, 0.0f, 0);
    glEnd();

    glColor3f(0.92f, 0.92f, 0.90f);
    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.6, 0.6, 20, 10);
    glPopMatrix();

    glColor3f(1.0f, 0.96f, 0.75f);
    glPushMatrix();
    glTranslatef(0.0f, -0.55f, 0.0f);
    glutSolidSphere(0.16, 16, 16);
    glPopMatrix();

    glPopMatrix();
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// ============================= Marker lampu (debug)
void drawLightSourceMarker() {
    if (!lightingEnabled) return;
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 6.0f, 0.0f);
    glColor3f(1.0f, 0.95f, 0.80f);
    glutSolidSphere(0.12, 16, 16);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// ============================= Camera update
void updateCamera() {
    float yawRad   = cameraYaw   * PI / 180.0f;
    float pitchRad = cameraPitch * PI / 180.0f;

    float forwardX = std::sin(yawRad) * std::cos(pitchRad);
    float forwardY = -std::sin(pitchRad);
    float forwardZ = -std::cos(yawRad) * std::cos(pitchRad);

    float rightX = std::sin(yawRad + PI/2.0f);
    float rightZ = -std::cos(yawRad + PI/2.0f);

    if (keysNormal['w'] || keysNormal['W']) {
        cameraPosX += forwardX * MOVE_SPEED;
        cameraPosY += forwardY * MOVE_SPEED;
        cameraPosZ += forwardZ * MOVE_SPEED;
    }
    if (keysNormal['s'] || keysNormal['S']) {
        cameraPosX -= forwardX * MOVE_SPEED;
        cameraPosY -= forwardY * MOVE_SPEED;
        cameraPosZ -= forwardZ * MOVE_SPEED;
    }
    if (keysNormal['a'] || keysNormal['A']) {
        cameraPosX -= rightX * MOVE_SPEED;
        cameraPosZ -= rightZ * MOVE_SPEED;
    }
    if (keysNormal['d'] || keysNormal['D']) {
        cameraPosX += rightX * MOVE_SPEED;
        cameraPosZ += rightZ * MOVE_SPEED;
    }
    if (keysNormal[' ']) cameraPosY += MOVE_SPEED;
    if (keysSpecial[GLUT_KEY_CTRL_L] || keysSpecial[GLUT_KEY_CTRL_R]) cameraPosY -= MOVE_SPEED;
}

// ============================= Display
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)windowWidth / (double)windowHeight, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float yawRad   = cameraYaw   * PI / 180.0f;
    float pitchRad = cameraPitch * PI / 180.0f;

    float lookX = cameraPosX + std::sin(yawRad) * std::cos(pitchRad);
    float lookY = cameraPosY - std::sin(pitchRad);
    float lookZ = cameraPosZ - std::cos(yawRad) * std::cos(pitchRad);

    gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
              lookX, lookY, lookZ,
              0.0f, 1.0f, 0.0f);

    updateLightingLogic();

    // Base room
    drawFloorWood();
    drawBackWallWarm();
    drawSideWallsWarm();   // <-- dinding kiri & kanan

    // Decor
    drawRugOval();
    drawWindow();
    drawCeilingLamp();

    // Objek utama
    drawCoffeeTable(0.0f, 1.2f);

    // Vas bunga di atas meja (y sekitar tinggi permukaan meja)
    drawFlowerVase(0.0f, 0.78f, 1.2f);

    drawChair(-3.2f, -0.5f, 0.0f);
    drawChair( 3.2f, -0.5f, 0.0f);

    drawShelf(6.0f, -4.8f);
    drawTV(6.0f, -4.35f);

    drawSideTableAndPlant(-6.0f, -4.8f);

    drawLightSourceMarker();

    glutSwapBuffers();
}

// ============================= Keyboard
void keyboardDown(unsigned char key, int, int) {
    keysNormal[key] = true;

    if (key == 27) std::exit(0); // ESC
    if (key == 'l' || key == 'L') lightingEnabled = !lightingEnabled;

    if (key == '1') lightMode = 1;
    if (key == '2') lightMode = 2;
    if (key == '3') lightMode = 3;
    if (key == '4') lightMode = 4;

    if (key == 'y' || key == 'Y') isDirectional = !isDirectional;
    if (key == 'h' || key == 'H') isSpotlight   = !isSpotlight;

    if (key == 'j' || key == 'J') spotCutoff = clampf(spotCutoff - 2.0f, 0.0f, 90.0f);
    if (key == 'k' || key == 'K') spotCutoff = clampf(spotCutoff + 2.0f, 0.0f, 90.0f);

    if (key == 'n' || key == 'N') linearAttenuation += 0.01f;
    if (key == 'm' || key == 'M') {
        linearAttenuation -= 0.01f;
        if (linearAttenuation < 0.0f) linearAttenuation = 0.0f;
    }

    if (key == 'i' || key == 'I') isShiny = !isShiny;
}

void keyboardUp(unsigned char key, int, int) { keysNormal[key] = false; }

void specialDown(int key, int, int) { if (key >= 0 && key < 512) keysSpecial[key] = true; }
void specialUp(int key, int, int)   { if (key >= 0 && key < 512) keysSpecial[key] = false; }

// ============================= Mouse look
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

// ============================= Idle & reshape
void idle() { updateCamera(); glutPostRedisplay(); }

void reshape(int w, int h) {
    windowWidth  = (w <= 1) ? 1 : w;
    windowHeight = (h <= 1) ? 1 : h;
    glViewport(0, 0, windowWidth, windowHeight);
}

// ============================= main
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Ruang Tamu - Dinding Samping + Vas Bunga");
    glutFullScreen();

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    initLighting();

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


