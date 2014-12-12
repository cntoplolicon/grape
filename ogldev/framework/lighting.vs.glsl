#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

uniform mat4 modelViewMatrix;
uniform mat4 modelViewMatrixForNormal;
uniform mat4 projectionMatrix;

out vec2 texCoord0;
out vec3 cameraSpacePosition;
out vec3 cameraSpaceNormal;

void main()
{
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    cameraSpaceNormal = (modelViewMatrixForNormal * vec4(normal, 0.0)).xyz;
    cameraSpacePosition = (modelViewMatrix * vec4(position, 1.0)).xyz;
    texCoord0 = texCoord;
}

