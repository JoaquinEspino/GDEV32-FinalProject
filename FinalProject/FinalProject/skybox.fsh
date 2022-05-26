#version 330
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;
uniform float time;
void main()
{    
    vec4 color = vec4(time, time, time, 1.0f);
    vec4 initialColor = texture(skybox, texCoords);
    FragColor = initialColor * color;
}