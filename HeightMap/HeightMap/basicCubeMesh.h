#ifndef BASICCUBEMESH_H
#define BASICCUBEMESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader_m.h"

#include <string>
#include <vector>
#include <GLFW/glfw3.h>

using namespace std;

struct basicCubeVertex
{
	glm::vec3 Position;
	//glm::vec3 Colours;
	glm::vec2 Textures;
};

class basicCubeMesh {

public:
	vector<basicCubeVertex> vertices;
	unsigned int VAO;

	//constructor
	basicCubeMesh(vector<basicCubeVertex>vertices)
	{
		this->vertices = vertices;
		setupMesh();
	}

	void Draw(Shader& shader,int Color) //mesh needs to know what type of shader is being used
	{
		shader.use();
		if (Color == 0)
		{
			shader.setVec3("colourIn",0,0,0);
		}
		if (Color == 1)
		{
			shader.setVec3("colourIn", 1, 1, 1);
		}
		if (Color == 3)
		{
			shader.setVec3("colourIn", 1, 1, 1);
		}
		 //randomise colours over time
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

private:

	unsigned int VBO, EBO;
	void setupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		//generate buffer data
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(basicCubeVertex), &vertices[0], GL_STATIC_DRAW); // gl draw can be static or dynaic,static is great for memory but you cant change anything. dynaic is not great on memory but you change change things

		///Attribute pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(basicCubeVertex), (void*)offsetof(basicCubeVertex, Position));
		glEnableVertexAttribArray(0);

		//colour attributes
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(basicCubeVertex), (void*)offsetof(basicCubeVertex, Colours));
		//glEnableVertexAttribArray(1);

		//texture attributes
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(basicCubeVertex), (void*)offsetof(basicCubeVertex, Textures));
		glEnableVertexAttribArray(2);
	}

};



#endif // !BASICCUBEMESH_H


