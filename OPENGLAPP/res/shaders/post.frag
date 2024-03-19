#version 410 core

// Post processing effects.

out vec4 fragColor;

in vec2 vTexCoords;

uniform sampler2D colorTarget;

vec4 BoxBlur(vec2 texCoord)
{
	vec2 texel = 1.0f / textureSize(colorTarget, 0);

	// Sample neighboring pixels then average.
	vec4 color = texture(colorTarget, texCoord);
	color += texture(colorTarget, texCoord + texel * vec2(-1, 1));
	color += texture(colorTarget, texCoord + texel * vec2(-1, 0));
	color += texture(colorTarget, texCoord + texel * vec2(-1, -1));
	color += texture(colorTarget, texCoord + texel * vec2(0, 1));
	color += texture(colorTarget, texCoord + texel * vec2(0, -1));
	color += texture(colorTarget, texCoord + texel * vec2(1, 1));
	color += texture(colorTarget, texCoord + texel * vec2(1, 0));
	color += texture(colorTarget, texCoord + texel * vec2(1, -1));

	return color / 9;
}

vec4 Distort(vec2 texCoord)
{
	vec2 mid = vec2(0.5);

	// Distorts the texture coordinate from centre.
	float distanceFromCentre = distance(texCoord, mid);
	vec2 normalizedCoord = normalize(texCoord - mid);
	float bias = distanceFromCentre + sin(distanceFromCentre * 15) * 0.05f;

	vec2 newCoord = mid + bias * normalizedCoord;
	return texture(colorTarget, newCoord);
}

vec4 Default(vec2 texCoord)
{
	return texture(colorTarget, texCoord); // Just output texture.
}

void main()
{
	vec2 texSize = textureSize(colorTarget, 0);
	vec2 texelSize = 1.0f / texSize;

	vec2 scale = (texSize - texelSize) / texSize;
	vec2 texCoord = vTexCoords / scale + texelSize * 0.5f; // Fix texture coordinates. (half-texel) <-> (1.0 - half-texel)

	fragColor = Distort(texCoord);
}
