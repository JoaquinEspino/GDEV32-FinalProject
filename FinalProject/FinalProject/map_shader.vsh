#version 330

// Vertex position
layout(location = 0) in vec3 vertexPosition;

uniform mat4 projection, view, model;

void main()
{
	gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}