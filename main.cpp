#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <iostream>
#include <cmath>
#include <optional>
#include <vector>
#include <algorithm>

#define PI 3.14159265358979323846f

bool colorMaterialEnabled = true;
struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float life;
    float size;
};

static std::vector<Particle> particles;

static void initParticles() {
    particles.reserve(200);
}

static void updateParticles(float dt) {
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life <= 0.0f; }),
        particles.end()
    );
    
    for (auto& p : particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.z += p.vz * dt;
        p.life -= dt;
        p.vy -= 0.5f * dt;
    }
}

static void drawParticles(GLUquadric* gQuad) {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (const auto& p : particles) {
        float alpha = p.life / 1.0f;
        glColor4f(0.8f, 0.7f, 0.5f, alpha * 0.6f);
        
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        gluSphere(gQuad, p.size, 6, 6);
        glPopMatrix();
    }
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void spawnDustParticles(float carX, float carZ, float speed) {
    if (std::abs(speed) < 0.1f) return;
    for (int i = 0; i < 5; i++) {
        Particle p;
        p.x = carX + ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
        p.y = 0.1f;
        p.z = carZ - 0.5f + ((rand() % 100) / 100.0f - 0.5f) * 0.3f;
        
        p.vx = ((rand() % 100) / 100.0f - 0.5f) * 1.0f;
        p.vy = (rand() % 100) / 100.0f * 2.0f;
        p.vz = -std::abs(speed) * 0.3f + ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
        
        p.life = 0.5f + (rand() % 100) / 100.0f * 0.5f;
        p.size = 0.05f + (rand() % 100) / 100.0f * 0.1f;
        
        particles.push_back(p);
    }

    if (particles.size() > 200) {
        particles.erase(particles.begin(), particles.begin() + (particles.size() - 200));
    }
}



namespace {
    struct AppState {
        GLuint skyTexture = 0;
        GLuint groundTexture = 0;
        float carPos = 0.0f;
        float carSpeed = 0.0f;
        float wheelAngle = 0.0f;
        float car2Pos = 0.0f;
        float car2Speed = 35.0f;
        float rotX = 0.f;
        float rotY = -25.f;
        bool brokenNoPushPop = false;
        bool showLocalAxes = true;
        sf::Vector3f eye{ 2.2f, 1.6f, 3.6f };
        sf::Vector3f center{ 0.0f, 0.6f, 0.0f };
        sf::Vector3f up{ 0.0f, 1.0f, 0.0f };
        bool chaseCam = false;
        float fovDeg = 60.0f;
        float nearP = 0.1f, farP = 300.0f;
        bool gameStarted = false;
    } G;

    float deg2rad(float d) { return d * PI / 180.f; }
    float clamp(float v, float a, float b) { return (v < a ? a : (v > b ? b : v)); }
}

bool loadTexture(const std::string& filename, GLuint& texID) {
    sf::Image img;
    if (!img.loadFromFile(filename)) return false;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        img.getSize().x, img.getSize().y, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

static void drawSky(float size = 50.0f)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    if (G.skyTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, G.skyTexture);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glColor3f(0.4f, 0.6f, 0.9f);
    }

    glBegin(GL_QUADS);
    // Front
    glTexCoord2f(0, 0); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, -size);
    // Back
    glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, size);
    // Left
    glTexCoord2f(0, 0); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(-size, size, size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, -size);
    // Right
    glTexCoord2f(0, 0); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(size, size, -size);
    // Top
    glTexCoord2f(0, 0); glVertex3f(-size, size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, size);
    glEnd();

    if (G.skyTexture) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}


static void initOpenGL() {
    glColor3f(0.8f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glDisable(GL_BLEND);

    GLfloat lightPos[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);

    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0f);
}

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
}

void setMaterial(float shininess) {
    GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


static void setupProjection(sf::Vector2u s) {
    if (!s.y) s.y = 1;
    const double aspect = s.x / static_cast<double>(s.y);

    glViewport(0, 0, (GLsizei)s.x, (GLsizei)s.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(G.fovDeg, aspect, G.nearP, G.farP);
    glMatrixMode(GL_MODELVIEW);
}


static void setupView() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (G.chaseCam) {
        float camDistance = 3.0f;
        float camHeight = 1.5f;
        sf::Vector3f camPos(-1.0f, camHeight, G.carPos - camDistance);
        sf::Vector3f camTarget(-1.0f, 0.6f, G.carPos);
        
        gluLookAt(camPos.x, camPos.y, camPos.z,
                  camTarget.x, camTarget.y, camTarget.z,
                  0, 1, 0);
    }
    else {
        gluLookAt(G.eye.x, G.eye.y, G.eye.z,
                  G.center.x, G.center.y, G.center.z,
                  G.up.x, G.up.y, G.up.z);
    }
}

static void drawAxes(float len = 0.4f) {
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(+len, 0, 0);
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, +len, 0);
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, +len);
    glEnd();
    glEnable(GL_LIGHTING);
}


static GLUquadric* gQuad = nullptr;

static void initQuadric() {
    gQuad = gluNewQuadric();
    gluQuadricNormals(gQuad, GLU_SMOOTH);
}

static void freeQuadric() {
    if (gQuad) gluDeleteQuadric(gQuad);
}

static void drawWheel(float radius = 0.25f, float width = 0.15f) {
    glPushMatrix();

    glColor3f(0.1f, 0.1f, 0.1f);
    glRotatef(90, 0, 1, 0);

    gluCylinder(gQuad, radius, radius, width, 24, 1);
    gluDisk(gQuad, 0.0, radius, 24, 1);

    glTranslatef(0, 0, width);
    gluDisk(gQuad, 0.0, radius, 24, 1);

    glPopMatrix();
}

static void drawBox(float sx, float sy, float sz) {
    glPushMatrix();
    glScalef(sx, sy, sz);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f(-0.5, -0.5, 0.5); glVertex3f(0.5, -0.5, 0.5); glVertex3f(0.5, 0.5, 0.5); glVertex3f(-0.5, 0.5, 0.5);
    glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(-0.5, -0.5, -0.5); glVertex3f(-0.5, 0.5, -0.5); glVertex3f(0.5, 0.5, -0.5); glVertex3f(0.5, -0.5, -0.5);
    glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f(-0.5, -0.5, -0.5); glVertex3f(-0.5, -0.5, 0.5); glVertex3f(-0.5, 0.5, 0.5); glVertex3f(-0.5, 0.5, -0.5);
    glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f(0.5, -0.5, -0.5); glVertex3f(0.5, 0.5, -0.5); glVertex3f(0.5, 0.5, 0.5); glVertex3f(0.5, -0.5, 0.5);
    glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(-0.5, 0.5, 0.5); glVertex3f(0.5, 0.5, 0.5); glVertex3f(0.5, 0.5, -0.5); glVertex3f(-0.5, 0.5, -0.5);
    glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(-0.5, -0.5, 0.5); glVertex3f(-0.5, -0.5, -0.5); glVertex3f(0.5, -0.5, -0.5); glVertex3f(0.5, -0.5, 0.5);
    glEnd();

    glPopMatrix();
}

static void updateCarMovement(float dt)
{
    G.carPos += G.carSpeed * dt;
    G.carSpeed *= 0.95f;
    
    if (G.carPos > 450.0f) {
        G.carPos = 450.0f;
        G.carSpeed = 0.0f;
    }
    if (G.carPos < -45.0f) G.carPos = -45.0f;

    if (G.gameStarted) {
        G.car2Pos += G.car2Speed * dt;
        if (G.car2Pos > 450.0f) {
            G.car2Pos = 450.0f;
            G.car2Speed = 0.0f;
        }
    }

    updateParticles(dt);
    spawnDustParticles(-1.0f, G.carPos, G.carSpeed);
    spawnDustParticles(-3.0f, G.car2Pos, G.car2Speed);
}

static void sceneCar()
{
    glPushMatrix();

    glColor3f(0.8f, 0.0f, 0.0f);
    setMaterial(128.0f);

    drawBox(1.0f, 0.3f, 0.7f);

    glPushMatrix();
    glTranslatef(0.06, 0.3f, 0.0f);
    drawBox(0.6f, 0.35f, 0.6f);
    glPopMatrix();

    const float wheelX = 0.5f;
    const float wheelZ = 0.4;
    const float wheelY = -0.10f;

    auto drawOneWheel = [&](float x, float z) {
        glPushMatrix();
        glTranslatef(x, wheelY, z);
        glRotatef(G.wheelAngle, 0, 0, 1);
        drawWheel();
        glPopMatrix();
    };

    drawOneWheel(+wheelX, +wheelZ);
    drawOneWheel(+wheelX, -wheelZ);
    drawOneWheel(-wheelX, +wheelZ);
    drawOneWheel(-wheelX, -wheelZ);

    glPopMatrix();
}

static void carMoving()
{
    glPushMatrix();

    glColor3f(0.0f, 0.0f, 0.0f);
    setMaterial(50);

    drawBox(1.0f, 0.3f, 0.7f);

    glPushMatrix();
    glTranslatef(0.06f, 0.3f, 0.0f);
    drawBox(0.6f, 0.35f, 0.6f);
    glPopMatrix();

    const float wheelX = 0.5f;
    const float wheelZ = 0.4f;
    const float wheelY = -0.10f;

    auto drawOneWheel = [&](float x, float z) {
        glPushMatrix();
        glTranslatef(x, wheelY, z);
        glRotatef(G.wheelAngle, 0, 0, 1);
        drawWheel();
        glPopMatrix();
    };

    drawOneWheel(+wheelX, +wheelZ);
    drawOneWheel(+wheelX, -wheelZ);
    drawOneWheel(-wheelX, +wheelZ);
    drawOneWheel(-wheelX, -wheelZ);

    glPopMatrix();
}

static void drawGround(float size = 50.0f)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    if (G.groundTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, G.groundTexture);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glColor3f(0.8f, 0.6f, 0.4f);
    }

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    
    const float tiles = 25.0f;

    glTexCoord2f(0, 0);       glVertex3f(-size, 0, -size);
    glTexCoord2f(tiles, 0);   glVertex3f(+size, 0, -size);
    glTexCoord2f(tiles, tiles); glVertex3f(+size, 0, +size);
    glTexCoord2f(0, tiles);   glVertex3f(-size, 0, +size);
    glEnd();

    if (G.groundTexture) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
}

static void drawCactus(float height = 2.0f) {
    glColor3f(0.2f, 0.6f, 0.2f);

    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(gQuad, 0.15, 0.12, height, 16, 1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.25f, height * 0.5f, 0);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(gQuad, 0.1, 0.08, height * 0.4f, 12, 1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.25f, height * 0.6f, 0);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(gQuad, 0.1, 0.08, height * 0.5f, 12, 1);
    glPopMatrix();
}

static void drawStartPole(float height = 2.5f) {
    glColor3f(1.0f, 0.8f, 0.0f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(gQuad, 0.1, 0.1, height, 16, 1);
    glPopMatrix();
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0, height, 0);
    glRotatef(90, 0, 1, 0);
    glScalef(0.5f, 0.3f, 0.05f);
    drawBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();
}


static void drawFinishLine(float zPos = 40.0f) {
    glDisable(GL_LIGHTING);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-5.0f, 0.01f, zPos);
    glVertex3f(5.0f, 0.01f, zPos);
    glEnd();
    glEnable(GL_LIGHTING);
}

static void drawRoad() {
    glDisable(GL_LIGHTING);
    
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-4.5f, 0.005f, -50.0f);
    glVertex3f(2.5f, 0.005f, -50.0f);
    glVertex3f(2.5f, 0.005f, 700.0f);
    glVertex3f(-4.5f, 0.005f, 700.0f);
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    for (float z = -50.0f; z < 700.0f; z += 8.0f) {
        glBegin(GL_LINES);
        glVertex3f(-1.0f, 0.01f, z);
        glVertex3f(-1.0f, 0.01f, z + 4.0f);
        glEnd();
    }
    
    glColor3f(1.0f, 0.9f, 0.0f);
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex3f(-4.3f, 0.01f, -50.0f);
    glVertex3f(-4.3f, 0.01f, 700.0f);
    glVertex3f(2.3f, 0.01f, -50.0f);
    glVertex3f(2.3f, 0.01f, 700.0f);
    glEnd();
    
    glEnable(GL_LIGHTING);
}


static void drawSceneObjects() {

    for (int i = 0; i < 2500; i++) {
        float z = i * 4.0f - 50.0f;
        
        glPushMatrix();
        glTranslatef(-5.5f, 0.0f, z);
        drawCactus(1.0f + (i % 4) * 0.3f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-7.5f - (i % 2) * 1.0f, 0.0f, z + 1.5f);
        drawCactus(1.3f + (i % 3) * 0.4f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-10.0f - (i % 3) * 1.5f, 0.0f, z + 0.5f);
        drawCactus(1.1f + (i % 5) * 0.3f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-13.0f - (i % 4) * 2.0f, 0.0f, z + 2.0f);
        drawCactus(1.4f + (i % 4) * 0.5f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-16.0f - (i % 2) * 1.0f, 0.0f, z + 1.0f);
        drawCactus(1.2f + (i % 3) * 0.3f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-19.0f - (i % 5) * 1.5f, 0.0f, z + 2.5f);
        drawCactus(1.5f + (i % 4) * 0.4f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(3.5f, 0.0f, z + 0.8f);
        drawCactus(1.1f + (i % 5) * 0.4f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(5.5f + (i % 2) * 1.0f, 0.0f, z + 2.2f);
        drawCactus(1.2f + (i % 4) * 0.3f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(8.0f + (i % 3) * 1.5f, 0.0f, z + 1.3f);
        drawCactus(1.3f + (i % 3) * 0.5f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(11.0f + (i % 4) * 2.0f, 0.0f, z + 0.7f);
        drawCactus(1.0f + (i % 5) * 0.4f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(14.0f + (i % 2) * 1.0f, 0.0f, z + 2.8f);
        drawCactus(1.4f + (i % 3) * 0.3f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(17.0f + (i % 5) * 1.5f, 0.0f, z + 1.5f);
        drawCactus(1.2f + (i % 4) * 0.5f);
        glPopMatrix();
    }
    
    drawFinishLine(450.0f);
    
    glPushMatrix();
    glTranslatef(-4.5f, 0.0f, -40.0f);
    drawStartPole(2.5f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(2.5f, 0.0f, -40.0f);
    drawStartPole(2.5f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-4.5f, 0.0f, 150.0f);
    drawStartPole(3.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(2.5f, 0.0f, 150.0f);
    drawStartPole(3.0f);
    glPopMatrix();
}

static void drawScene(float dt) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (G.chaseCam == false) {
        glRotatef(G.rotX, 1, 0, 0);
        glRotatef(G.rotY, 0, 1, 0);
    }
    drawSky(200.0f);
    glPopMatrix();
    
    setupView();

    if (G.chaseCam == false) {
        glRotatef(G.rotX, 1, 0, 0);
        glRotatef(G.rotY, 0, 1, 0);
    }

    drawGround(800.0f);
    
    drawRoad();
    drawSceneObjects();
    drawParticles(gQuad);
    
    glPushMatrix();
    glTranslatef(-1.0f, 0.01f, G.carPos);
    sceneCar();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.0f, 0.0f, G.car2Pos);
    carMoving();
    glPopMatrix();
}


int main() {
    sf::RenderWindow win(sf::VideoMode({1024, 768}), "3D car race");

    win.setVerticalSyncEnabled(true);
    (void)win.setActive(true);

    initOpenGL();
    initLighting();
    setMaterial(100);
    setupProjection(win.getSize());
    initQuadric();
    initParticles();
    
    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        std::cout << "Nie mozna wczytac czcionki!\n";
    }

    sf::Text startText(font, "Press SPACE to start the race!", 30);
    startText.setFillColor(sf::Color::White);
    startText.setPosition({300.f, 50.f});
    
    sf::Text controlsText(font, "W - Forward  |  S - Backward  |  Q - Nitro", 20);
    controlsText.setFillColor(sf::Color::White);
    controlsText.setPosition({10.f, 720.f});
    
    sf::Image img;
    if (!img.loadFromFile("sky.jpg")) {
        std::cout << "Nie można wczytać tekstury nieba!\n";
    }
    if (!loadTexture("sand.jpg", G.groundTexture)) {
        std::cout << "Nie można wczytać tekstury pustyni!\n";
    }
    else {
        glGenTextures(1, &G.skyTexture);
        glBindTexture(GL_TEXTURE_2D, G.skyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getSize().x, img.getSize().y, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    sf::Clock clock;
    

    bool running = true;
    while (running) {
        float dt = clock.restart().asSeconds();

        for (std::optional<sf::Event> event = win.pollEvent(); event.has_value(); event = win.pollEvent()) {
            sf::Event e = event.value();
            
            if (e.is<sf::Event::Closed>()) {
                running = false;
            }

            if (e.is<sf::Event::KeyPressed>() && e.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) {
                running = false;
            }
            
            if (e.is<sf::Event::Resized>()) {
                setupProjection(win.getSize());
            }

            if (e.is<sf::Event::KeyPressed>()) {
                sf::Keyboard::Key code = e.getIf<sf::Event::KeyPressed>()->code;
                switch (code) {
                case sf::Keyboard::Key::Left:     if (!G.chaseCam) G.rotY -= 5.f; break;
                case sf::Keyboard::Key::Right:    if (!G.chaseCam) G.rotY += 5.f; break;
                case sf::Keyboard::Key::Up:       if (!G.chaseCam) G.rotX += 5.f; break;
                case sf::Keyboard::Key::Down:     if (!G.chaseCam) G.rotX -= 5.f; break;
                case sf::Keyboard::Key::W: G.carSpeed += 2.5f; break;
                case sf::Keyboard::Key::S: G.carSpeed -= 2.0; break;
                case sf::Keyboard::Key::Q: G.carSpeed += 20.0f; break;
                case sf::Keyboard::Key::PageUp:
                case sf::Keyboard::Key::P:
                    G.eye.x *= 0.95f;
                    G.eye.z *= 0.95f;
                    break;
                    case sf::Keyboard::Key::Space:
                        if (!G.gameStarted) {
                            G.gameStarted = true;
                            G.chaseCam = true;
                            std::cout << "START: Race started!\n";
                        }
                        break;
                    break;
                case sf::Keyboard::Key::PageDown:
                case sf::Keyboard::Key::O:
                    G.eye.x *= 1.05f;
                    G.eye.z *= 1.05f;
                    break;
                case sf::Keyboard::Key::C:
                    G.chaseCam = !G.chaseCam;
                    break;
                default: break;
                }
            }
        }

        updateCarMovement(dt);
        drawScene(dt);
        
        if (!G.gameStarted) {
            win.pushGLStates();
            win.draw(startText);
            win.popGLStates();
        }
        
        if (G.gameStarted && (G.carPos >= 150.0f || G.car2Pos >= 150.0f)) {
            sf::Text winText(font, "", 60);
            winText.setFillColor(sf::Color::Yellow);
            winText.setPosition({250.f, 300.f});
            
            if (G.carPos >= 450.0f && G.car2Pos < 450.0f) {
                winText.setString("YOU WIN!");
                winText.setFillColor(sf::Color::Red);
            }
            else if (G.car2Pos >= 450.0f && G.carPos < 450.0f) {
                winText.setString("YOU LOSE!");
                winText.setFillColor(sf::Color::White);
            }
            
            win.pushGLStates();
            win.draw(winText);
            win.popGLStates();
        }
        
        if (G.gameStarted) {
            win.pushGLStates();
            win.draw(controlsText);
            win.popGLStates();
        }

        
        win.display();

    }

    freeQuadric();
    return 0;
}
