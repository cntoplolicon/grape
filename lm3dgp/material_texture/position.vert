#version 330

layout(location = 0) in vec3 position;

uniform mat4 modelViewMatrix;

layout(std140) uniform Projection
{
    mat4 projectionMatrix;
};

void main()
{
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}

