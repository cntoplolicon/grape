#version 330

layout (location = 0) in vec3 position;

uniform mat4 modelViewMatrix;
out vec3 cameraSpacePosition;

void main()
{
    cameraSpacePosition = (modelViewMatrix * vec4(position, 1.0)).xyz;
}

