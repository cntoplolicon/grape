#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 position0;
out vec3 normal0;
out vec2 texCoord0;

void main()
{
    mat4 modelViewMatrix = viewMatrix * modelMatrix;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    position0 = (modelViewMatrix * vec4(position, 1.0)).xyz;

    mat4 modelViewMatrixForNormal = modelViewMatrix;
    modelViewMatrixForNormal = inverse(modelViewMatrixForNormal);
    modelViewMatrixForNormal = transpose(modelViewMatrixForNormal);
    normal0 = (modelViewMatrixForNormal * vec4(normal, 0.0)).xyz;
    texCoord0 = texCoord;
}

