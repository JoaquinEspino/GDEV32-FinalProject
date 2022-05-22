#version 330

// UV-coordinate of the fragment (interpolated by the rasterization stage)
in vec2 outUV;

// Color of the fragment received from the vertex shader (interpolated by the rasterization stage)
in vec3 outColor;

// Final color of the fragment that will be rendered on the screen
out vec4 fragColor;

// Texture unit of the texture
uniform sampler2D tex;

void main()
{
	// Get pixel color of the texture at the current UV coordinate
	// and output it as our final fragment color
	fragColor = texture(tex, outUV);
}
