#version 430 core

// Lit PBR shader.

#define MAX_LIGHTS 4

#define pi 3.1415926535897932384626433832f

struct PointLight
{
	vec4 position;
	vec4 color;
	float intensity;
};

struct SpotLight
{
	vec4 position;
	vec4 color;
	float intensity;

	vec4 direction;
	float innerCutoff;
	float outerCutoff;
};

// Outputs
out vec4 fragColor;

// Inputs
in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;
in vec3 vTangent;
in vec3 vBiTangent;
in vec3 vViewPosition;

// Uniforms
uniform vec3 cameraPosition;

uniform vec3 ambientColor;
uniform vec3 sunlightDir;
uniform vec3 sunlightColor;

uniform float specular; // Material specular power
uniform vec3 Ka; // Ambient material colour
uniform vec3 Kd; // Diffuse material colour
uniform vec3 Ks; // Specular material colour

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;

uniform int numPointLights;
uniform int numSpotLights;

uniform bool drawFog = false;

// Storage buffer objects.
layout (std430, binding = 0) readonly buffer PointLightSBO
{
	PointLight pointLights[];
};

layout (std430, binding = 1) readonly buffer SpotLightSBO
{
	SpotLight spotLights[];
};

// Constants
const float roughness = 0.5f; // Should probably be a parameter or uniform.
const float reflectionCoefficient = 1.4f; // Should also probably be a parameter or uniform.

vec3 GetDiffuse(vec3 direction, vec3 color, vec3 normal, vec3 view)
{
	// Oren-Nayar diffuse calculation.
	float NdL = max(0.0f, dot(normal, direction));
	float NdE = max(0.0f, dot(normal, view));

	float R2 = roughness * roughness;

	float A = 1.0f - 0.5f * R2 / (R2 + 0.33f);
	float B = 0.45f * R2 / (R2 + 0.09f);

	vec3 lightProjected = normalize(direction - normal * NdL);
	vec3 viewProjected = normalize(view - normal * NdE);
	float CX = max(0.0f, dot(lightProjected, viewProjected));

	float alpha = sin(max(acos(NdE), acos(NdL)));
	float beta = tan(min(acos(NdE), acos(NdL)));
	float DX = alpha * beta;

	return color * (NdL * (A + B * CX * DX));
}

vec3 GetSpecular(vec3 direction, vec3 color, vec3 normal, vec3 view)
{
	// Cook-Torrance specular calculation.
	float NdL = max(0.0f, dot(normal, direction));
	float NdE = max(0.0f, dot(normal, view));

	vec3 H = (direction + view) / 2;

	float NdH = max(0.0f, dot(normal, H));
	float NdH2 = NdH * NdH;
	float e = 2.71828182845904523536028747135f;

	float R2 = roughness * roughness;

	float exponent = -(1 - NdH2) / (NdH2 * R2);
	float D = pow(e, exponent) / (R2 * NdH2 * NdH2);

	float F = reflectionCoefficient + (1 - reflectionCoefficient) * pow(1 - NdE, 5);

	float X = 2.0f * NdH / dot(view, H);
	float G = min(1.0f, min(X * NdL, X * NdE));

	return color * (max((D*G*F) / (NdE * pi), 0.0f));
}

void main()
{
	// Sample textuers.
	vec3 diffSample = texture(diffuseTex, vTexCoords).rgb;
	vec3 specSample = texture(specularTex, vTexCoords).rgb;
	vec3 normSample = texture(normalTex, vTexCoords).rgb;

	// Make sure these are actually normalized.
	vec3 N = normalize(vNormal);
	vec3 L = normalize(sunlightDir);
	vec3 T = normalize(vTangent);
	vec3 B = normalize(vBiTangent);

	mat3 TBN = mat3(T,B,N);

	N = TBN * (normSample * 2 - 1); // Modify normals by normal map & tangents.

	vec3 V = normalize(cameraPosition - vPosition.xyz); // Calculate view vector.

	// Shade for sunlight.
	vec3 diffuseTotal = GetDiffuse(L, sunlightColor, N, V);
	vec3 specularTotal = GetSpecular(L, sunlightColor, N, V);

	// Shade for point lights.
	for (int i = 0; i < numPointLights; i++)
	{
		vec3 direction = pointLights[i].position.xyz - vPosition.xyz;

		// Point light attenuation.
		float distance = length(direction);
		direction = direction/distance;

		vec3 color = pointLights[i].color.xyz / (distance * distance); // Apply attenuation.
		color *= pointLights[i].intensity; // Apply light intensity.

		diffuseTotal += GetDiffuse(direction, color, N, V);
		specularTotal += GetSpecular(direction, color, N, V);
	}

	// Shade for spotlights.
	for (int j = 0; j < numSpotLights; j++)
	{
		vec3 direction = spotLights[j].position.xyz - vPosition.xyz;

		// Calculate spotlight cone by cutoffs.
		float theta = dot(normalize(direction), normalize(-spotLights[j].direction.xyz));
		float epsilon = (spotLights[j].innerCutoff - spotLights[j].outerCutoff);
		float intensity = clamp((theta - spotLights[j].outerCutoff) / epsilon, 0.0, 1.0);

		float distance = length(direction);
		direction = direction / distance;

		vec3 color = spotLights[j].color.xyz / (distance * distance); // Apply attenuation.
		color *= spotLights[j].intensity;// Apply light intensity.
		color *= intensity; // Apply spotlight cone.

		diffuseTotal += GetDiffuse(direction, color, N, V);
		specularTotal += GetSpecular(direction, color, N, V);
	}

	// Apply shading, textures, and material properties.
	vec3 ambient = ambientColor * Ka * diffSample;
	vec3 diffuse = Kd * diffuseTotal * diffSample;
	vec3 specular = Ks * specularTotal * specSample;
	vec3 result = ambient + diffuse + specular;

	// Mix shading result with fog effect.
	float fogDistance = length(vViewPosition);
	float fogAmount = smoothstep(0.1, 25.0, fogDistance);

	if (drawFog == true)
		fragColor = mix(vec4(result, 1.0), vec4(ambientColor, 1.0), fogAmount);
	else
		fragColor = vec4(result, 1.0);
}
