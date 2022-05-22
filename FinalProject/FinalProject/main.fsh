#version 330

// UV-coordinate of the fragment (interpolated by the rasterization stage)
in vec2 outUV;

// Color of the fragment received from the vertex shader (interpolated by the rasterization stage)
in vec3 outColor;

// Final color of the fragment that will be rendered on the screen
out vec4 fragColor;

in vec3 fragNormal;
in vec3 fragPosition;

// Texture unit of the texture
uniform sampler2D tex;

// Light position Uniform
uniform vec3 directional_light;

// Light ambient Uniform
uniform vec3 point_ambient_intensity;

// Light diffuse Uniform
uniform vec3 point_diffuse_intensity;

// Light specular Uniform
uniform vec3 point_specular_intensity;

// Shininess Uniform
uniform float u_shininess;

uniform vec3 eyePosition;
void main()
{
	// Get pixel color of the texture at the current UV coordinate
	// and output it as our final fragment color
	vec4 texColor = texture(tex, outUV);

	float ambientStrength = 0.5f;
	vec3 ambient = ambientStrength * point_ambient_intensity;
	vec4 ambient4 = vec4(ambient, 1.0f);

	// diffuse light directional
	vec3 norm = normalize(fragNormal);
	vec3 directional_light_dir = -directional_light;
	float dirDiff = max(dot(norm, directional_light_dir), 0.0f);
	vec3 dirDiffuse = dirDiff * point_diffuse_intensity;

	vec3 viewDir = normalize(eyePosition - fragPosition);
	vec3 reflectDirDiff = reflect(-directional_light_dir, fragNormal);

	float specDir = pow(max(dot(viewDir, reflectDirDiff), 0.0),u_shininess);
	vec3 specularDir = specDir * point_specular_intensity;

	vec3 texColor3 = vec3(texColor);

	vec3 sum = (ambient + dirDiffuse + specularDir) * texColor3;

	fragColor = vec4(sum, 1.0f);

	//fragColor = texture(tex, outUV);
}
