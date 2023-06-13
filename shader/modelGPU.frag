#version 330 core
out vec4 FragColor;

in vec2 sTexcoord;
//in vec3 sColor;

uniform sampler2D diffTex;

void main()
{    
    FragColor = texture(diffTex, sTexcoord);
 // FragColor = vec4(sColor,1.0);
}