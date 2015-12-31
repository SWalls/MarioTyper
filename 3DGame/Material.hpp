//
//  Material.hpp
//  Mario Typer
//
//  Created by Soeren Walls on 11/26/15.
//  Copyright © 2015 Soeren Walls. All rights reserved.
//

#ifndef Material_hpp
#define Material_hpp

#import <stdio.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <GLUT/glut.h>
#import "float3.h"

extern "C" unsigned char* stbi_load(char const *filename, int *x, int *y, int
                                    *comp, int req_comp);

class Material
{
public:
    float3 kd;			// diffuse reflection coefficient
    float3 ks;			// specular reflection coefficient
    float shininess;	// specular exponent
    Material()
    {
        kd = float3(0.5, 0.5, 0.5) + float3::random() * 0.5;
        ks = float3(1, 1, 1);
        shininess = 15;
    }
    virtual void apply();
};

class TexturedMaterial : public Material
{
    GLuint textureName;
    GLint filtering;
public:
    TexturedMaterial(const char* filename,
                     GLint filtering = GL_LINEAR_MIPMAP_LINEAR);
    virtual void apply();
};

#endif /* Material_hpp */
