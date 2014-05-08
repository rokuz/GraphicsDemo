#version 430 core

out vec4 fragmentColor;
uniform vec4 color = vec4(1,1,1,1);

void main()
{
    fragmentColor = color;
}