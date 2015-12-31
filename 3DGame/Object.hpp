//
//  Object.hpp
//  Mario Typer
//
//  Created by Soeren Walls on 11/26/15.
//  Copyright © 2015 Soeren Walls. All rights reserved.
//

#ifndef Object_hpp
#define Object_hpp

#import <stdio.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <GLUT/glut.h>
#import "float3.h"
#import "Material.hpp"
#import "Mesh.hpp"

int rand(int min, int max);

class Object
{
protected:
    Material* material;
    float3 scaleFactor;
    float3 position;
    float3 orientationAxis;
    float orientationAngle;
    float3 sphereCenter = float3(0,0,0);
    float sphereRadius = 0;
    bool dead = false;
    bool colliding = false;
    float collisionTime = 0;
public:
    enum Type { AVATAR, ENEMY, FRIENDLY_PROJECTILE, ENEMY_PROJECTILE, NEUTRAL };
    Type type = NEUTRAL;
    Object(Material* material, Type t = NEUTRAL):
    material(material),orientationAngle(0.0f),scaleFactor(1.0,1.0,1.0),orientationAxis(0.0,1.0,0.0)
    { type = t; }
    virtual ~Object(){}
    Object* translate(float3 offset);
    Object* scale(float3 factor);
    Object* rotate(float angle);
    float3 getOrientationAxis() { return orientationAxis; }
    void setOrientationAxis(float3 axis) { orientationAxis = axis.normalize(); }
    float getAngle() { return orientationAngle; }
    float3 getPosition() { return position; }
    bool isColliding() { return colliding; }
    void setColliding(bool c) { colliding = c; }
    float3 center() { return sphereCenter; }
    float boundingRadius() { return sphereRadius; }
    virtual void draw(bool drawSpheres);
    virtual void drawSphere();
    virtual void drawModel()=0;
    virtual void control(std::vector<bool>& keysPressed, std::vector<Object*> objects, int currentLevel) {}
    virtual bool interact(Object* obj) { return false; }
    virtual void move(double t, double dt){}
    virtual void kill() { dead = true; }
    virtual bool isDead() { return dead; }
    virtual void drawShadow(float3 lightDir, float3 groundNormal, float3 groundPosition)=0;
};

class Teapot : public Object
{
public:
    Teapot(Material* material):Object(material){}
    void drawModel() { glutSolidTeapot(1.0f); }
};

class MeshInstance : public Object
{
    Mesh* mesh;
    bool shadow = true;
public:
    MeshInstance(Mesh* mesh, Material* material, Type t = NEUTRAL):
    Object(material, t), mesh(mesh)
    {
        // construct collision sphere from mesh points
        std::vector<float3*> vertices = mesh->getVertices();
        int numV = 0;
        for(float3 *v : vertices) {
            sphereCenter += *v;
            numV++;
        }
        sphereCenter /= numV;
        float dist = 0;
        for(float3 *v : vertices) {
            dist = (sphereCenter - *v).norm();
            if(dist > sphereRadius) sphereRadius = dist;
        }
    }
    virtual void control(std::vector<bool>& keysPressed, std::vector<Object*> objects, int currentLevel) {
        bool foundCollision = false;
        for(Object *obj : objects) {
            if(obj != this)
                if(this->interact(obj))
                    foundCollision = true;
        }
        if(!foundCollision)
            colliding = false;
    }
    virtual bool interact(Object* obj);
    Object *setShadow(bool s) { shadow = s; return this; }
    void drawModel() { mesh->draw(); }
    virtual void drawShadow(float3 lightDir, float3 groundNormal, float3 groundPosition);
};

class Ground : public MeshInstance
{
protected:
    float3 normal;
public:
    Ground(Mesh* mesh, Material* m, float3 n, float3 pos):
    MeshInstance(mesh, m), normal(n)
    {
        position = pos;
        scale(float3(1,0.1,1));
        translate(float3(0,-0.1,0));
    }
    void drawShadow(float3 lightDir, float3 groundNormal, float3 groundPosition) {}
    float3 getNormal() { return normal; }
};

class Sky : public MeshInstance
{
protected:
    float3 normal;
    float3 oAxis1;
    float3 oAxis2;
public:
    Sky(Mesh* mesh, Material* m, float3 n, float3 pos, float3 oAxis1, float3 oAxis2):
    MeshInstance(mesh, m), normal(n)
    {
        position = pos;
        this->oAxis1 = oAxis1;
        this->oAxis2 = oAxis2;
        scale(float3(6,1,15));
        rotate(-90);
    }
    virtual void draw(bool drawSpheres)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        material->apply();
        // apply scaling, translation and orientation
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        if(oAxis2.norm2() != 0) {
            orientationAxis = oAxis2;
            glRotatef(orientationAngle, orientationAxis.x, orientationAxis.y, orientationAxis.z);
        }
        if(oAxis1.norm2() != 0)
            orientationAxis = oAxis1;
        glRotatef(orientationAngle, orientationAxis.x, orientationAxis.y, orientationAxis.z);
        glScalef(scaleFactor.x, scaleFactor.y, scaleFactor.z);
        drawModel();
        glPopMatrix();
        if(drawSpheres && type != NEUTRAL) {
            drawSphere();
        }
    }
    void drawShadow(float3 lightDir, float3 groundNormal, float3 groundPosition) {}
    float3 getNormal() { return normal; }
};

class Enemy : public MeshInstance
{
protected:
    int avatarPosition = 0;
    float enemyVertTheta = 0;
    int health = 1;
public:
    Enemy(Mesh* mesh, Material* material, int position, int health, Type t = ENEMY):
    MeshInstance(mesh, material, t)
    {
        avatarPosition = position;
        this->health = health;
    }
    virtual void control(std::vector<bool>& keysPressed, std::vector<Object*> objects, int currentLevel);
    int getPosition() { return avatarPosition; }
    int getHealth() { return health; }
    virtual void kill() {
        if(health > 1) health--;
        else dead = true;
    }
};

class Projectile : public MeshInstance
{
protected:
    int towardPosition = 0;
public:
    Projectile(Mesh* mesh, Material* material, int position, Type t = FRIENDLY_PROJECTILE):
    MeshInstance(mesh, material, t)
    {
        towardPosition = position;
    }
    int getPosition() { return towardPosition; }
    virtual void control(std::vector<bool>& keysPressed, std::vector<Object*> objects, int currentLevel);
};

#endif /* Object_hpp */