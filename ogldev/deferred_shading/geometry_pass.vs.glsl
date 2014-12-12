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
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    position0 = (modelMatrix * vec4(position, 1.0)).xyz;
    texCoord0 = texCoord;
    normal0 = normal;
}

