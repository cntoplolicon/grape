#version 330

layout (location = 0) in vec3 position; 

out vec3 cameraSpacePosition;

uniform mat4 modelViewMatrix; 
uniform mat4 projectionMatrix; 

void main() 
{ 
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    cameraSpacePosition = (modelViewMatrix * vec4(position, 1.0)).xyz;
}

