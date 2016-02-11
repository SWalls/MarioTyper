//
//  main.cpp
//  Mario Typer
//
//  Created by Soeren Walls on 11/26/15.
//  Copyright © 2015 Soeren Walls. All rights reserved.
//
//  All trademarks, trade names, and characters appearing in this game are the
//  property of their respective owners, including in some instances Nintendo.
//
//  This is an educational (non-commercial) game created in a weekend,
//  for the Computer Graphics course at AIT-Budapest.
//

#define _USE_MATH_DEFINES
#import <math.h>
#import <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// Needed on MsWindows
#include <windows.h>
#endif // Win32 platform

#import "float2.h"
#import "LightSource.hpp"
#import "Object.hpp"

#import <vector>
#import <map>
#import <fstream>
#import <string>

const unsigned int window_width = 1200;
const unsigned int window_height = 800;

std::vector<std::vector<std::string>> dictionary_lvl_1;
std::vector<std::vector<std::string>> dictionary_lvl_2;
std::vector<std::vector<std::string>> dictionary_lvl_3;

std::string pickRandomWord(int level)
{
    std::string word;
    int idx = rand(0,25);
    switch(level) {
        case 1:
            word = dictionary_lvl_1.at(idx).at(rand(0, (int)dictionary_lvl_1.at(idx).size()-1));
            break;
        case 2:
            word = dictionary_lvl_2.at(idx).at(rand(0, (int)dictionary_lvl_2.at(idx).size()-1));
            break;
        case 3:
            word = dictionary_lvl_3.at(idx).at(rand(0, (int)dictionary_lvl_3.at(idx).size()-1));
            break;
    }
    return word;
}

class Camera
{
    float3 eye;
    
    float3 ahead;
    float3 lookAt;
    float3 right;
    float3 up;
    
    float fov;
    float aspect;
    
    float2 lastMousePos;
    float2 mouseDelta;
    
    bool inMotion = false;
    bool moveLeft = false;
    float motionAngle = 0;
    
public:
    float3 getEye()
    {
        return eye;
    }
    Camera()
    {
        eye = float3(0, 0.75, 0);
        lookAt = float3(0, 0, 0);
        right = float3(1, 0, 0);
        up = float3(0, 1, 0);
        
        fov = 1.1;
        aspect  = 1;
    }
    
    void apply()
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fov /3.14*180, aspect, 0.1, 500);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(eye.x, eye.y, eye.z, lookAt.x, lookAt.y, lookAt.z, 0.0, 1.0, 0.0);
    }
    
    void setAspectRatio(float ar) { aspect= ar; }
    
    void move(float dt, std::vector<bool>& keysPressed, bool noClip)
    {
        float yaw = atan2f( ahead.x, ahead.z );
        float pitch = -atan2f( ahead.y, sqrtf(ahead.x * ahead.x + ahead.z * ahead.z) );
        
        if(noClip) {
            
            if(keysPressed.at('w'))
                eye += ahead * dt * 20;
            if(keysPressed.at('s'))
                eye -= ahead * dt * 20;
            if(keysPressed.at('a'))
                eye -= right * dt * 20;
            if(keysPressed.at('d'))
                eye += right * dt * 20;
            if(keysPressed.at('q'))
                eye -= float3(0,1,0) * dt * 20;
            if(keysPressed.at('e'))
                eye += float3(0,1,0) * dt * 20;
            
            yaw -= mouseDelta.x * 0.02f;
            pitch += mouseDelta.y * 0.02f;
            
        }
        
        if(!inMotion) {
            
            if(keysPressed.at(258)) { // left
                inMotion = true;
                moveLeft = true;
            } else if(keysPressed.at(259)) { //right
                inMotion = true;
                moveLeft = false;
            }
            
        } else {
            
            if(motionAngle < 90) {
                if(!noClip)
                    yaw += (moveLeft ? 1 : -1) * (M_PI/2/18); // 0.018f;
                motionAngle += 5;
            } else {
                inMotion = false;
                motionAngle = 0;
            }
            
        }
        
        if(pitch > 3.14/2) pitch = 3.14/2;
        if(pitch < -3.14/2) pitch = -3.14/2;
        
        mouseDelta = float2(0, 0);
        
        ahead = float3(sin(yaw)*cos(pitch), -sin(pitch), cos(yaw)*cos(pitch) );
        right = ahead.cross(float3(0, 1, 0)).normalize();
        up = right.cross(ahead);
        lookAt = eye + ahead;
    }
    
    void startDrag(int x, int y)
    {
        lastMousePos = float2(x, y);
    }
    void drag(int x, int y)
    {
        float2 mousePos(x, y);
        mouseDelta = mousePos - lastMousePos;
        lastMousePos = mousePos;
    }
    void endDrag()
    {
        mouseDelta = float2(0, 0);
    }
    
    bool isMoving() {
        return inMotion;
    }
    
    bool movingLeft() {
        return moveLeft;
    }
    
    float getMotionAngle() {
        return (motionAngle/180) * M_PI;
    }
    
};

class Scene
{
    Camera camera;
    Ground *ground;
    Object *avatar;
    
    std::vector<LightSource*> lightSources;
    std::vector<Object*> objects;
    std::vector<Mesh*> meshes;
    std::vector<Material*> materials;
    
    int avatarPosition = 0; // value from 0 to 3. represents which of the 4 tunnels the avatar is looking at
    bool f1_pressed = false;
    bool f2_pressed = false;
    bool n2_pressed = false;
    bool noClipMode = false;
    bool showSpheres = false;
    
    bool gameOver = false;
    bool gamePaused = true;
    
    int currentLevel = 1;
    std::string words[4]; // one word per avatar position
    int wordsBeginTypingIndex[4]; // the character position of each word at which user should type next letter
    
public:
    void initialize()
    {
        // BUILD THE SCENE HERE
        
        lightSources.push_back(
                               new DirectionalLight(
                                                    float3(1, 1, -1).normalize(),
                                                    float3(1, 0.5, 1)));
        lightSources.push_back(
                               new PointLight(
                                              float3(-1, -1, 1),
                                              float3(0.2, 0.1, 0.1)));
        
        Material* yellowDiffuseMaterial = new Material();
        yellowDiffuseMaterial->kd = float3(1, 1, 0);
        
        materials.push_back(new TexturedMaterial("res/lava.png", GL_LINEAR));
        materials.push_back(new TexturedMaterial("res/marioD.jpg", GL_LINEAR));
        materials.push_back(new TexturedMaterial("res/boo-body-white.png", GL_LINEAR));
        materials.push_back(new TexturedMaterial("res/stone.png", GL_LINEAR));
        materials.push_back(new TexturedMaterial("res/gate.bmp", GL_LINEAR));
        materials.push_back(new TexturedMaterial("res/fire.jpeg", GL_LINEAR));
        materials.push_back(new TexturedMaterial("res/grass.jpg", GL_LINEAR));
        materials.push_back(new TexturedMaterial("res/sky.jpg", GL_LINEAR));
        
        meshes.push_back(new Mesh("res/plane.obj"));
        meshes.push_back(new Mesh("res/mario_obj.obj"));
        meshes.push_back(new Mesh("res/boo-body.obj"));
        meshes.push_back(new Mesh("res/Pedestal.obj"));
        meshes.push_back(new Mesh("res/gate.obj"));
        meshes.push_back(new Mesh("res/mountain.obj"));
        meshes.push_back(new Mesh("res/fireball.obj"));
        
        ground = new Ground(meshes.at(0), materials.at(3), float3(0,1,0), float3(0,0,0));
        
        // ground
        objects.push_back(ground);
        // sky north
        objects.push_back((new Sky(meshes.at(0), materials.at(7), float3(0,0,-1), float3(0,50,200), float3(1,0,0), float3(0,0,1))));
        // sky west
        objects.push_back((new Sky(meshes.at(0), materials.at(7), float3(0,0,-1), float3(200,50,0), float3(0,0,1), float3(0,0,0))));
        // sky south
        objects.push_back((new Sky(meshes.at(0), materials.at(7), float3(0,0,-1), float3(0,50,-200), float3(-1,0,0), float3(0,0,1))));
        // sky east
        objects.push_back((new Sky(meshes.at(0), materials.at(7), float3(0,0,-1), float3(-200,50,0), float3(0,0,-1), float3(0,0,0))));
        // mountains north
        objects.push_back((new MeshInstance(meshes.at(5), materials.at(0)))
                          ->setShadow(false)
                          ->translate(float3(0, -10, 100))
                          ->scale(float3(0.000003, 0.000004, 0.000003)) );
        // archway north
        objects.push_back((new MeshInstance(meshes.at(4), materials.at(4)))
                          ->translate(float3(3.3, 0, 18))
                          ->rotate(90)
                          ->scale(float3(1, 1, 1)) );
        // pedestal north left
        objects.push_back((new MeshInstance(meshes.at(3), materials.at(4)))
                          ->translate(float3(5, 0, 7))
                          ->scale(float3(0.4, 0.5, 0.4)) );
        // pedestal north right
        objects.push_back((new MeshInstance(meshes.at(3), materials.at(4)))
                          ->translate(float3(-5, 0, 7))
                          ->scale(float3(0.4, 0.5, 0.4)) );
        // mountains east
        objects.push_back((new MeshInstance(meshes.at(5), materials.at(0)))
                          ->setShadow(false)
                          ->translate(float3(-100, -10, 0))
                          ->scale(float3(0.000003, 0.000004, 0.000003)) );
        // archway east
        objects.push_back((new MeshInstance(meshes.at(4), materials.at(4)))
                          ->translate(float3(-5, 0, -3.3))
                          ->rotate(180)
                          ->scale(float3(1, 1, 1)) );
        // pedestal east left
        objects.push_back((new MeshInstance(meshes.at(3), materials.at(4)))
                          ->translate(float3(-7, 0, 5))
                          ->scale(float3(0.4, 0.5, 0.4)) );
        // pedestal east right
        objects.push_back((new MeshInstance(meshes.at(3), materials.at(4)))
                          ->translate(float3(-7, 0, -5))
                          ->scale(float3(0.4, 0.5, 0.4)) );
        // mountains south
        objects.push_back((new MeshInstance(meshes.at(5), materials.at(0)))
                          ->setShadow(false)
                          ->translate(float3(0, -10, -100))
                          ->scale(float3(0.000003, 0.000004, 0.000003)) );
        // archway south
        objects.push_back((new MeshInstance(meshes.at(4), materials.at(4)))
                          ->translate(float3(3.3, 0, -4.8))
                          ->rotate(90)
                          ->scale(float3(1, 1, 1)) );
        // pedestal south left
        objects.push_back((new MeshInstance(meshes.at(3), materials.at(4)))
                          ->translate(float3(5, 0, -7))
                          ->scale(float3(0.4, 0.5, 0.4)) );
        // pedestal south right
        objects.push_back((new MeshInstance(meshes.at(3), materials.at(4)))
                          ->translate(float3(-5, 0, -7))
                          ->scale(float3(0.4, 0.5, 0.4)) );
        // mountains west
        objects.push_back((new MeshInstance(meshes.at(5), materials.at(0)))
                          ->setShadow(false)
                          ->translate(float3(100, -10, 0))
                          ->scale(float3(0.000003, 0.000004, 0.000003)) );
        // archway west
        objects.push_back((new MeshInstance(meshes.at(4), materials.at(4)))
                          ->translate(float3(5, 0, 3.3))
                          ->scale(float3(1, 1, 1)) );
        // pedestal west left
        objects.push_back((new MeshInstance(meshes.at(3), materials.at(4)))
                          ->translate(float3(7, 0, 5))
                          ->scale(float3(0.4, 0.5, 0.4)) );
        // pedestal west right
        objects.push_back((new MeshInstance(meshes.at(3), materials.at(4)))
                          ->translate(float3(7, 0, -5))
                          ->scale(float3(0.4, 0.5, 0.4)) );
        
        reset();
        
    }
    
    ~Scene()
    {
        for (std::vector<LightSource*>::iterator iLightSource = lightSources.begin(); iLightSource != lightSources.end(); ++iLightSource)
            delete *iLightSource;
        for (std::vector<Material*>::iterator iMaterial = materials.begin(); iMaterial != materials.end(); ++iMaterial)
            delete *iMaterial;
        for (std::vector<Object*>::iterator iObject = objects.begin(); iObject != objects.end(); ++iObject)
            delete *iObject;
    }
    
    void reset()
    {
        for (std::vector<Object*>::iterator it=objects.begin(); it!=objects.end(); )
        {
            if((*it)->type != Object::NEUTRAL) {
                it = objects.erase(it);
            } else {
                ++it;
            }
        }
        
        avatarPosition = 0;
        f1_pressed = false;
        f2_pressed = false;
        n2_pressed = false;
        noClipMode = false;
        gameOver = false;
        gamePaused = true;
        currentLevel = 1;
        
        for(int i=0; i<4; i++) {
            words[i] = "";
            wordsBeginTypingIndex[i] = 0;
        }
        
        // mario
        avatar = (new MeshInstance(meshes.at(1), materials.at(1), Object::AVATAR))
        ->scale(float3(0.008, 0.008, 0.008))
        ->translate(float3(0, 0, 1))
        ->rotate(10);
        objects.push_back(avatar);
        
        camera = *new Camera();
        camera.setAspectRatio((float)window_width/window_height);
        
    }
    
    Camera& getCamera()
    {
        return camera;
    }
    
    Object* getAvatar()
    {
        return avatar;
    }
    
    void control(float t, float dt, std::vector<bool>& keysPressed)
    {
        if((t > 30 && currentLevel < 2) || (t > 60 && currentLevel < 3)) {
            currentLevel++;
            printf("Level %d!\n", currentLevel);
        }
        
        handleTyping(keysPressed);
        
        // toggle settings using F1 and F2 keys
        if(!f1_pressed && keysPressed.at(260)) {
            f1_pressed = true;
            noClipMode = !noClipMode;
        } else if(f1_pressed && !keysPressed.at(260)) {
            f1_pressed = false;
        }
        if(!f2_pressed && keysPressed.at(261)) {
            f2_pressed = true;
            showSpheres = !showSpheres;
        } else if(f2_pressed && !keysPressed.at(261)) {
            f2_pressed = false;
        }
        
        // Do camera and avatar moving
        bool wasMoving = camera.isMoving();
        camera.move(dt, keysPressed, noClipMode);
        
        // Game Over logic
        if(gameOver) {
            if(keysPressed.at('1')) {
                reset();
            }
            return;
        }
        
        // Game Pause logic
        if(!n2_pressed && keysPressed.at('2')) {
            n2_pressed = true;
            gamePaused = !gamePaused;
        } else if(n2_pressed && !keysPressed.at('2')) {
            n2_pressed = false;
        }
        
        if(gamePaused) return;
        
        // Move avatar depending on camera rotation
        if(camera.isMoving()) {
            if(wasMoving) avatar->rotate(camera.movingLeft() ? 5 : -5);
            float theta = camera.getMotionAngle() * (camera.movingLeft() ? 1 : -1);
            theta += ((M_PI/2)*avatarPosition) + (camera.movingLeft() ? 0 : -M_PI*2);
            float3 avatarPos = avatar->getPosition();
            avatar->translate(float3(-avatarPos.x+sin(theta), -avatarPos.y, -avatarPos.z+cos(theta)));
        } else {
            if(wasMoving) {
                avatarPosition += camera.movingLeft() ? 1 : -1;
                if(avatarPosition < 0) avatarPosition = 3;
                else if(avatarPosition > 3) avatarPosition = 0;
            }
        }
        
        // Erase dead objects
        for (std::vector<Object*>::iterator it=objects.begin(); it!=objects.end(); )
        {
            if((*it)->isDead()) {
                if((*it)->type == Object::AVATAR) {
                    gameOver = true;
                    return;
                }
                else
                    it = objects.erase(it);
            } else {
                ++it;
            }
        }
        
        // Control objects
        for(Object* object : objects) {
            object->control(keysPressed, objects, currentLevel);
        }
        
        // Do random word selection
        int likelihood = floor((float)rand(0,10000) * (1.0f+(currentLevel*0.05f)));
        if(likelihood > (10400 * (0.95f+(currentLevel*0.05f)))) {
            int side = rand(0,3);
            if(words[side] == "") {
                words[side] = pickRandomWord(currentLevel);
                printf("Word #%d is now: %s\n", side, words[side].c_str());
                // boo
                Object *boo = (new Enemy(meshes.at(2), materials.at(2), side, (int)words[side].length(), Object::ENEMY))
                ->scale(float3(0.005, 0.005, 0.005))
                ->translate(float3(8*fmod(side,2)*(side > 1 ? -1 : 1),
                                   1.5,
                                   8*fmod(side+1,2)*(side > 1 ? -1 : 1)))
                ->rotate(180 + 90*side);
                objects.push_back(boo);
            } else {
                // printf("Tried changing #%d.\n", side+1);
            }
        }
    }
    
    void handleTyping(std::vector<bool>& keysPressed)
    {
        std::string word = words[avatarPosition];
        if(word != "") {
            char c = word[wordsBeginTypingIndex[avatarPosition]];
            if(keysPressed[c]) {
                // printf("Typed '%c' in word '%s'\n", c, word.c_str());
                wordsBeginTypingIndex[avatarPosition]++;
                // fireball
                objects.push_back((new Projectile(meshes.at(6), materials.at(5), avatarPosition, Object::FRIENDLY_PROJECTILE))
                                  ->scale(float3(0.1,0.1,0.1))
                                  ->translate(avatar->center()) );
                if(wordsBeginTypingIndex[avatarPosition] >= word.length()) {
                    wordsBeginTypingIndex[avatarPosition] = 0;
                    words[avatarPosition] = "";
                    printf("Success: Typed word '%s'\n", word.c_str());
                }
            }
        }
    }
    
    void draw()
    {
        camera.apply();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        unsigned int iLightSource=0;
        for (; iLightSource<lightSources.size(); iLightSource++)
        {
            glEnable(GL_LIGHT0 + iLightSource);
            lightSources.at(iLightSource)->apply(GL_LIGHT0 + iLightSource);
        }
        iLightSource=0;
        for (; iLightSource<GL_MAX_LIGHTS; iLightSource++)
            glDisable(GL_LIGHT0 + iLightSource);
        
        for (unsigned int iObject=0; iObject<objects.size(); iObject++)
        {
            objects.at(iObject)->draw(showSpheres);
            float3 lightDir = lightSources.at(0)->getLightDirAt(float3(0,0,0));
            objects.at(iObject)->drawShadow(lightDir, ground->getNormal(), ground->getPosition());
        }
        drawWord();
    }
    
    void drawWord()
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, window_width, 0.0, window_height);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        if(gameOver) {
            glColor3f(1.0f, 0.0f, 0.0f);
            glRasterPos2f((float)window_width/2.0f - 50, (float)window_height/2.0f);
        }
        std::string str = gameOver ? "YOU DIED" : (gamePaused ? "PAUSED (PRESS 2 TO UNPAUSE)" : words[avatarPosition]);
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        float wordStartX = ((float)window_width/2.0f) - (str.length()*8);
        float wordStartY = (float)window_height*0.95f;
        void * font = GLUT_BITMAP_HELVETICA_18;
        int i=0;
        for(char c : str) {
            if(!gameOver) {
                if(i < wordsBeginTypingIndex[avatarPosition]) {
                    glColor3f(0.0f, 0.0f, 0.0f);
                    glRasterPos2f(wordStartX + 16*i, wordStartY);
                } else {
                    glColor3f(1.0f, 1.0f, 1.0f);
                    glRasterPos2f(wordStartX + 16*i, wordStartY);
                }
            }
            glutBitmapCharacter(font, c);
            i++;
        }
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
};

Scene scene;
std::vector<bool> keysPressed;

void parseDictionary()
{
    for(int i=0; i<26; i++) {
        dictionary_lvl_1.push_back(*new std::vector<std::string>());
        dictionary_lvl_2.push_back(*new std::vector<std::string>());
        dictionary_lvl_3.push_back(*new std::vector<std::string>());
    }
    char currLetter = ' ';
    int currIndex = -1;
    int numWords = 0;
    int numLvl1 = 0;
    int numLvl2 = 0;
    int numLvl3 = 0;
    std::ifstream file("ospd.txt");
    std::string str;
    printf("Parsing dictionary...\n");
    while (std::getline(file, str))
    {
        if(str.length()<3 || str.length()>10)
            continue;
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        if(str.at(0) != currLetter) {
            currIndex++;
            if(currIndex > 25) {
                printf("Error parsing dictionary. New letter '%c' does not fit after '%c'.\n", str.at(0), currLetter);
                break;
            }
            currLetter = str.at(0);
        }
        if(str.length() <= 5) {
            dictionary_lvl_1.at(currIndex).push_back(str);
            numLvl1++;
        } else if(str.length() <= 7) {
            dictionary_lvl_2.at(currIndex).push_back(str);
            numLvl2++;
        } else {
            dictionary_lvl_3.at(currIndex).push_back(str);
            numLvl3++;
        }
        numWords++;
        if(numWords % 50000 == 0) {
            printf("Parsed %d words...\n", numWords);
        }
    }
    printf("Done. Parsed %d words. \n(Lvl1: %d, Lvl2: %d, Lvl3: %d)\n", numWords, numLvl1, numLvl2, numLvl3);
}

void onDisplay( ) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear screen
    
    scene.draw();
    
    glutSwapBuffers(); // drawing finished
}

void onIdle()
{
    double t = glutGet(GLUT_ELAPSED_TIME)*0.001;        	// time elapsed since starting this program in sec
    static double lastTime = 0.0;
    double dt = t - lastTime;
    lastTime = t;
    
    scene.control(t, dt, keysPressed);
    
    glutPostRedisplay();
}

void onKeyboard(unsigned char key, int x, int y)
{
    keysPressed.at(key) = true;
}

void onKeyboardUp(unsigned char key, int x, int y)
{
    keysPressed.at(key) = false;
}

void onSpecialKey(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:
            keysPressed.at(256) = true;
            break;
        case GLUT_KEY_DOWN:
            keysPressed.at(257) = true;
            break;
        case GLUT_KEY_LEFT:
            keysPressed.at(258) = true;
            break;
        case GLUT_KEY_RIGHT:
            keysPressed.at(259) = true;
            break;
        case GLUT_KEY_F1:
            keysPressed.at(260) = true;
            break;
        case GLUT_KEY_F2:
            keysPressed.at(261) = true;
            break;
    }
}

void onSpecialKeyUp(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:
            keysPressed.at(256) = false;
            break;
        case GLUT_KEY_DOWN:
            keysPressed.at(257) = false;
            break;
        case GLUT_KEY_LEFT:
            keysPressed.at(258) = false;
            break;
        case GLUT_KEY_RIGHT:
            keysPressed.at(259) = false;
            break;
        case GLUT_KEY_F1:
            keysPressed.at(260) = false;
            break;
        case GLUT_KEY_F2:
            keysPressed.at(261) = false;
            break;
    }
}

void onMouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON) {
        if(state == GLUT_DOWN) {
            scene.getCamera().startDrag(x, y);
        } else {
            scene.getCamera().endDrag();
        }
    }
}

void onMouseMotion(int x, int y)
{
    scene.getCamera().drag(x, y);
}

void onReshape(int winWidth, int winHeight)
{
    glViewport(0, 0, winWidth, winHeight);
    scene.getCamera().setAspectRatio((float)winWidth/winHeight);
}

int main(int argc, char **argv) {
    
    srand(time(NULL));
    parseDictionary();
    
    // begin glut stuff
    glutInit(&argc, argv);						// initialize GLUT
    glutInitWindowSize(window_width, window_height);				// startup window size
    glutInitWindowPosition(100, 100);           // where to put window on screen
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);    // 8 bit R,G,B,A + double buffer + depth buffer
    
    glutCreateWindow("Mario Typer by Soeren Walls");	// application window is created and displayed
    
    glViewport(0, 0, window_width, window_height);
    
    glutDisplayFunc(onDisplay);					// register callback
    glutIdleFunc(onIdle);						// register callback
    glutReshapeFunc(onReshape);
    glutKeyboardFunc(onKeyboard);
    glutKeyboardUpFunc(onKeyboardUp);
    glutSpecialFunc(onSpecialKey);
    glutSpecialUpFunc(onSpecialKeyUp);
    glutMouseFunc(onMouse);
    glutMotionFunc(onMouseMotion);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    scene.initialize();
    for(int i=0; i<262; i++)
        keysPressed.push_back(false);
    
    glutMainLoop();								// launch event handling loop
    
    return 0;
    
}

