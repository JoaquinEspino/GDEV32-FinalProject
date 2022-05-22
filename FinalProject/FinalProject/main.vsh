#version 330

// Vertex position
layout(location = 0) in vec3 vertexPosition;

// Vertex color
layout(location = 1) in vec3 vertexColor;

// Vertex UV coordinate
layout(location = 2) in vec2 vertexUV;

// Vertex Normal Vector Coordinate
layout(location = 3) in vec3 vertexNV;

// UV coordinate (will be passed to the fragment shader)
out vec2 outUV;

// Color (will be passed to the fragment shader)
out vec3 outColor;

uniform mat4 mat, model;

out vec3 fragPosition;
out vec3 fragNormal;

void main()
{
	// Convert our vertex position to homogeneous coordinates by introducing the w-component.
	// Vertex positions are ... positions, so we specify the w-coordinate as 1.0.

	fragPosition = vec3(model * vec4(vertexPosition, 1.0f));
	fragNormal = mat3(transpose(inverse(model))) * vertexNV;
	vec4 finalPosition = mat * vec4(vertexPosition, 1.0);

	// Give OpenGL the final position of our vertex
	gl_Position = finalPosition;

	outUV = vertexUV;
	outColor = vertexColor;
}
