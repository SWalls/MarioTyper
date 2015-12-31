//
//  LightSource.cpp
//  Mario Typer
//
//  Created by Soeren Walls on 11/26/15.
//  Copyright © 2015 Soeren Walls. All rights reserved.
//

#import "LightSource.hpp"

void DirectionalLight::apply( GLenum openglLightName )
{
    float aglPos[] = {dir.x, dir.y, dir.z, 0.0f};
    glLightfv(openglLightName, GL_POSITION, aglPos);
    float aglZero[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glLightfv(openglLightName, GL_AMBIENT, aglZero);
    float aglIntensity[] = {radiance.x, radiance.y, radiance.z, 1.0f};
    glLightfv(openglLightName, GL_DIFFUSE, aglIntensity);
    glLightfv(openglLightName, GL_SPECULAR, aglIntensity);
    glLightf(openglLightName, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(openglLightName, GL_LINEAR_ATTENUATION, 0.0f);
    glLightf(openglLightName, GL_QUADRATIC_ATTENUATION, 0.0f);
}

void PointLight::apply(GLenum openglLightName) {
    float aglPos[] = {pos.x, pos.y, pos.z, 1.0f};
    glLightfv(openglLightName, GL_POSITION, aglPos);
    float aglZero[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glLightfv(openglLightName, GL_AMBIENT, aglZero);
    float aglIntensity[] = {power.x, power.y, power.z, 1.0f};
    glLightfv(openglLightName, GL_DIFFUSE, aglIntensity);
    glLightfv(openglLightName, GL_SPECULAR, aglIntensity);
    glLightf(openglLightName, GL_CONSTANT_ATTENUATION, 0.0f);
    glLightf(openglLightName, GL_LINEAR_ATTENUATION, 0.0f);
    glLightf(openglLightName, GL_QUADRATIC_ATTENUATION, 0.25f / 3.14f);
}