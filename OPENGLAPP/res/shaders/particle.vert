#version 410

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 Color;

out vec4 vColor;

uniform mat4 mvp;

void main()
{
	vColor = Color;
	gl_Position = mvp * Position;
}
