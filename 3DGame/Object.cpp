//
//  Object.cpp
//  Mario Typer
//
//  Created by Soeren Walls on 11/26/15.
//  Copyright © 2015 Soeren Walls. All rights reserved.
//

#import "Object.hpp"

// return random integer between min and max (inclusive)
int rand(int min, int max) { return rand()%(max-min + 1) + min; }

Object* Object::translate(float3 offset){
    position += offset;
    sphereCenter += offset;
    return this;
}
Object* Object::scale(float3 factor){
    scaleFactor *= factor;
    sphereRadius *= fmax(factor.x, fmax(factor.y, factor.z));
    sphereCenter *= factor;
    return this;
}
Object* Object::rotate(float angle){
    orientationAngle += angle;
    if(orientationAngle >= 360) orientationAngle -= 360;
    if(orientationAngle <= 0) orientationAngle += 360;
    // printf("Rotating by %f degrees.\n", angle);
    // printf("Sphere Center before rotation: (%f, %f, %f).\n", sphereCenter.x, sphereCenter.y, sphereCenter.z);
    
    float theta = (angle/180)*M_PI;
    float a = position.x;
    float b = position.y;
    float c = position.z;
    float x = sphereCenter.x;
    float y = sphereCenter.y;
    float z = sphereCenter.z;
    float u = orientationAxis.x;
    float v = orientationAxis.y;
    float w = orientationAxis.z;
    
    // Formula for computing rotation of 3D point around arbitrary axis.
    
    sphereCenter.x = (a*(v*v + w*w) - u*(b*v + c*w - u*x - v*y - w*z))*(1 - cos(theta))
    + (x*cos(theta)) + (-c*v + b*w - w*y + v*z)*sin(theta);
    sphereCenter.y = (b*(u*u + w*w) - v*(a*u + c*w - u*x - v*y - w*z))*(1 - cos(theta))
    + (y*cos(theta)) + (c*u - a*w + w*x - u*z)*sin(theta);
    sphereCenter.z = (c*(u*u + v*v) - w*(a*u + b*v - u*x - v*y - w*z))*(1 - cos(theta))
    + (z*cos(theta)) + (-b*u + a*v - v*x + u*y)*sin(theta);
    
    // printf("Sphere Center after rotation: (%f, %f, %f).\n\n", sphereCenter.x, sphereCenter.y, sphereCenter.z);
    return this;
}

void Object::draw(bool drawSpheres)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    material->apply();
    // apply scaling, translation and orientation
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(orientationAngle, orientationAxis.x, orientationAxis.y, orientationAxis.z);
    glScalef(scaleFactor.x, scaleFactor.y, scaleFactor.z);
    drawModel();
    glPopMatrix();
    if(drawSpheres && type != NEUTRAL) {
        drawSphere();
    }
}

void Object::drawSphere()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    if(colliding) {
        glColor3f(0.8f,0.0f,0.9f);
    } else {
        glColor3f(1.0,0.0,0.0);
    }
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(sphereCenter.x, sphereCenter.y, sphereCenter.z);
    // glRotatef(orientationAngle, orientationAxis.x, orientationAxis.y, orientationAxis.z);
    glutWireSphere(sphereRadius, 10, 10);
    glPopMatrix();
}

bool MeshInstance::interact(Object* obj) {
    bool foundCollision = false;
    if(obj->type != NEUTRAL && this->type != NEUTRAL) {
        float3 dist = obj->center() - this->center();
        float mind = obj->boundingRadius()*0.8 + this->boundingRadius()*0.8;
        if(dist.norm() < mind) {
            foundCollision = true;
            colliding = true;
            obj->setColliding(true);
            collisionTime++;
            if(collisionTime > 0 && obj->type == ENEMY) {
                if(type == FRIENDLY_PROJECTILE) {
                    obj->kill();
                    this->kill();
                    collisionTime = 0;
                } else if(type == AVATAR) {
                    this->kill();
                }
            }
        }
    }
    return foundCollision;
}

void MeshInstance::drawShadow(float3 lightDir, float3 groundNormal, float3 groundPosition)
{
    if(!shadow) return;
    float shear[] = {
        1, 0, 0, 0,
        -lightDir.x/lightDir.y, 1, -lightDir.z/lightDir.y, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    // glColor3f(0,0,0);
    glColor4f(0, 0, 0, 0.8);
    // apply scaling, translation and orientation
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(1, 0, 1);
    glTranslatef(0, groundPosition.y-position.y, 0);
    glMultMatrixf(shear);
    glTranslatef(position.x, position.y, position.z);
    glRotatef(orientationAngle, orientationAxis.x, orientationAxis.y, orientationAxis.z);
    glScalef(scaleFactor.x, scaleFactor.y, scaleFactor.z);
    drawModel();
    glPopMatrix();
}

void Enemy::control(std::vector<bool>& keysPressed, std::vector<Object*> objects, int currentLevel) {
    MeshInstance::control(keysPressed, objects, currentLevel);
    Object* avatar = nullptr;
    for(Object *obj : objects) {
        if(obj->type == AVATAR)
            avatar = obj;
    }
    // all enemies move toward avatar
    if(avatar != nullptr)
    {
        float3 dir = (avatar->center()-this->center()).normalize();
        float3 motionV = float3(0.01,0.01,0.01)*dir;
        translate(motionV);
        translate(float3(0,0.01*sin(enemyVertTheta),0));
        enemyVertTheta += M_PI/(18.0f/(0.8f));
    }
}

void Projectile::control(std::vector<bool>& keysPressed, std::vector<Object*> objects, int currentLevel) {
    MeshInstance::control(keysPressed, objects, currentLevel);
    Object* avatar = nullptr;
    Enemy* towardEnemy = nullptr;
    for(Object *obj : objects) {
        if(obj->type == AVATAR) {
            avatar = obj;
        } else if(obj->type == ENEMY) {
            Enemy* e = nullptr;
            if((e = dynamic_cast<Enemy*>(obj))) {
                if(e->getPosition() == this->getPosition()) {
                    towardEnemy = e;
                }
            }
        }
    }
    if(towardEnemy != nullptr && avatar != nullptr && type == Object::FRIENDLY_PROJECTILE)
    {
        // all projectiles spin
        rotate(5);
        orientationAxis += float3(rand(1,5)*0.2,rand(1,5)*0.2,rand(1,5)*0.2);
        orientationAxis.normalize();
        
        float3 dist = float3(0.2, 0.2, 0.2) * ((towardEnemy->center()-avatar->center()).normalize());
        translate(dist);
    }
}