//
//  Material.cpp
//  Mario Typer
//
//  Created by Soeren Walls on 11/26/15.
//  Copyright © 2015 Soeren Walls. All rights reserved.
//

#import "Material.hpp"

void Material::apply()
{
    glDisable(GL_TEXTURE_2D);
    float aglDiffuse[] = {kd.x, kd.y, kd.z, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, aglDiffuse);
    float aglSpecular[] = {kd.x, kd.y, kd.z, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aglSpecular);
    if(shininess <= 128)
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    else
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
}

TexturedMaterial::TexturedMaterial(const char* filename, GLint filtering)
{
    this->filtering = filtering;
    unsigned char* data;
    int width;
    int height;
    int nComponents = 4;
    data = stbi_load(filename, &width, &height, &nComponents, 0);
    if(data == NULL) return;
    glGenTextures(1, &textureName);  // id generation
    glBindTexture(GL_TEXTURE_2D, textureName);      // binding
    if(nComponents == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data); // uploading
    else if(nComponents == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, data); // uploading
    delete data;
}

void TexturedMaterial::apply()
{
    Material::apply();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, filtering);
    glTexEnvi(GL_TEXTURE_ENV,
              GL_TEXTURE_ENV_MODE, GL_REPLACE);
}