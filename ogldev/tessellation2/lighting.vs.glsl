#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

uniform mat4 modelViewMatrix;
uniform mat4 modelViewMatrixForNormal;

out vec3 position_cs_in;
out vec3 normal_cs_in;
out vec2 texCoord_cs_in;

void main()
{
    position_cs_in =( modelViewMatrix * vec4(position, 1.0)).xyz;
    normal_cs_in = normalize((modelViewMatrixForNormal * vec4(normal, 0.0)).xyz);
    texCoord_cs_in = texCoord;
}

