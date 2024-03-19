#version 410 core

// Basic texture shader for render targets.

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec4 aTangent;

out vec4 vPosition;
out vec3 vNormal;
out vec2 vTexCoords;
out vec3 vTangent;
out vec3 vBiTangent;
out vec3 vViewPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 mvp;

void main()
{
	vPosition = model * aPos;
	vViewPosition = (view * vPosition).xyz;
	vNormal = (model * aNormal).xyz;
	vTexCoords = aTexCoords;
	vTangent = (model * vec4(aTangent.xyz, 0.0)).xyz;
	vBiTangent = cross(vNormal, vTangent) * aTangent.w;

	gl_Position = mvp * aPos;
}
