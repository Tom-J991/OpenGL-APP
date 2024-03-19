#version 410 core

// Basic texture shader for render targets.

out vec4 fragColor;

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;
in vec3 vTangent;
in vec3 vBiTangent;
in vec3 vViewPosition;

uniform sampler2D diffuseTex;

void main()
{
	float fogDistance = length(vViewPosition);
	float fogAmount = smoothstep(0.1, 25.0, fogDistance);

	// Output texture with the worst fog effect of all time.
	fragColor = mix(texture(diffuseTex, vTexCoords), vec4(0.25, 0.25, 0.25, 1.0), fogAmount);
}
