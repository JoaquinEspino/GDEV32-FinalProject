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

uniform vec3 pointLightPosition;

in vec4 lightFragmentPosition;

uniform sampler2D shadowMap;
uniform float time;
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

	vec3 fragLightNDC = lightFragmentPosition.xyz / lightFragmentPosition.w;
	fragLightNDC = (fragLightNDC + 1)/2;

	float bias = max(0.5 * (1.0-dot(fragNormal, directional_light_dir)), 0.0);
	bias = 0.05f;

	vec3 color = vec3(time, time, time);

	//diffuse point light
	vec3 lightDir = normalize(pointLightPosition - fragPosition);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * point_diffuse_intensity;
	vec4 diffuse4 = vec4(diffuse, 1.0f);

	vec3 reflectDir = reflect(-lightDir, fragNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0),u_shininess);
	vec3 specular = spec * point_specular_intensity;
	// point attenuation
	float d = sqrt(pow(fragPosition.x-pointLightPosition.x, 2) + pow(fragPosition.y-pointLightPosition.y, 2) + pow(fragPosition.z-pointLightPosition.z, 2));
	float Pattenuation = 1/(1.0f + (0.0014f * d) + (0.000007f * pow(d, 2)));
	// spot attenuation 

	if(texture(shadowMap, fragLightNDC.xy).r < (fragLightNDC.z-bias))
	{
		if(time==0.2f)//night time
		{
			vec3 sum = (ambient+ ((diffuse + specular)*Pattenuation)) * texColor3;
			fragColor = vec4(sum, 1.0f);
		}
		else
		{
			vec3 sum = (ambient) * texColor3;
			fragColor = vec4(sum, 1.0f);
		}

	}
	else
	{
		if(time==0.2f)//night time
		{
			vec3 sum = (ambient + ((dirDiffuse*color) + (specularDir*color) + ((diffuse + specular)*Pattenuation))) * texColor3;
			fragColor = vec4(sum, 1.0f);
		}
		else
		{
			vec3 sum = (ambient + ((dirDiffuse*color) + (specularDir*color))) * texColor3;
			fragColor = vec4(sum, 1.0f);
		}
	}

	//fragColor = texture(tex, outUV);
}
