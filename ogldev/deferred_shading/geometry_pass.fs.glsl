#version 330

layout (location = 0) out vec3 positionOut;
layout (location = 1) out vec3 colorOut;
layout (location = 2) out vec3 normalOut;

in vec3 position0;
in vec3 normal0;
in vec2 texCoord0;

uniform sampler2D textureSampler;

void main()
{
    positionOut = position0;
    colorOut = texture(textureSampler, texCoord0).xyz;
    normalOut = normal0;
}

