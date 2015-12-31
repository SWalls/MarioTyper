//
//  Mesh.hpp
//  Mario Typer
//
//  Created by Soeren Walls on 11/26/15.
//  Copyright © 2015 Soeren Walls. All rights reserved.
//

#pragma once
#import "float2.h"
#import "float3.h"
#import <vector>

class   Mesh
{
	struct  Face
	{
		int       positionIndices[5];
		int       normalIndices[5];
		int       texcoordIndices[5];
		bool      isQuad;
        bool      isPentagon;
	};

	std::vector<std::string*>	rows;
	std::vector<float3*>		positions;
	std::vector<std::vector<Face*> >          submeshFaces;
	std::vector<float3*>		normals;
	std::vector<float2*>		texcoords;

	int            modelid;

public:
    Mesh(const char *filename);
	~Mesh();

	void        draw();
	void        drawSubmesh(unsigned int iSubmesh);
    std::vector<float3*> getVertices() { return positions; }
};

