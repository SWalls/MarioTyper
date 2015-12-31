//
//  LightSource.hpp
//  Mario Typer
//
//  Created by Soeren Walls on 11/26/15.
//  Copyright © 2015 Soeren Walls. All rights reserved.
//

#ifndef LightSource_hpp
#define LightSource_hpp

#import <stdio.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <GLUT/glut.h>
#import "float3.h"

class LightSource
{
public:
    virtual float3 getRadianceAt  ( float3 x )=0;
    virtual float3 getLightDirAt  ( float3 x )=0;
    virtual float  getDistanceFrom( float3 x )=0;
    virtual void   apply( GLenum openglLightName )=0;
};

class DirectionalLight : public LightSource
{
    float3 dir;
    float3 radiance;
public:
    DirectionalLight(float3 dir, float3 radiance)
    :dir(dir), radiance(radiance){}
    float3 getRadianceAt  ( float3 x ){return radiance;}
    float3 getLightDirAt  ( float3 x ){return dir;}
    float  getDistanceFrom( float3 x ){return 900000000;}
    void   apply( GLenum openglLightName );
};

class PointLight : public LightSource
{
    float3 pos;
    float3 power;
public:
    PointLight(float3 pos, float3 power)
    :pos(pos), power(power){}
    float3 getRadianceAt  ( float3 x ){return power*(1/(x-pos).norm2()*4*3.14);}
    float3 getLightDirAt  ( float3 x ){return (pos-x).normalize();}
    float  getDistanceFrom( float3 x ){return (pos-x).norm();}
    void   apply( GLenum openglLightName );
};

#endif /* LightSource_hpp */
