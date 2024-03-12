#version 410 core

out vec4 fragColor;

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;
in vec3 vTangent;
in vec3 vBiTangent;

uniform vec3 cameraPosition;

uniform vec3 ambientColor;
uniform vec3 lightDir;
uniform vec3 lightColor;

uniform float specular;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;

void main()
{
	vec3 N = normalize(vNormal);
	vec3 L = normalize(lightDir);
	vec3 T = normalize(vTangent);
	vec3 B = normalize(vBiTangent);

	mat3 TBN = mat3(T,B,N);

	float lambertTerm = max(0, min(1, dot(N, -L)));

	vec3 V = normalize(cameraPosition - vPosition.xyz);
	vec3 R = reflect(L, N);

	float specularTerm = pow(max(0, dot(R, V)), specular);

	vec3 diffSample = texture(diffuseTex, vTexCoords).rgb;
	vec3 specSample = texture(specularTex, vTexCoords).rgb;
	vec3 normSample = texture(normalTex, vTexCoords).rgb;

	N = TBN * (normSample * 2 - 1);

	vec3 ambient = ambientColor * Ka * diffSample;
	vec3 diffuse = lightColor * Kd * lambertTerm * diffSample;
	vec3 specular = lightColor * Ks * specularTerm * specSample;

	fragColor = vec4(ambient + diffuse + specular, 1.0);
}
