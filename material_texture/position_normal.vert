#version 330

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;

uniform mat4 modelViewMatrix;
uniform mat3 modelViewMatrixForNormal;

out vec3 cameraSpacePosition;
out vec3 cameraSpaceNormal;

layout(std140) uniform Projection
{
    mat4 projectionMatrix;
};

void main()
{
    vec4 tempCamPosition = modelViewMatrix * vec4(position, 1.0);
    gl_Position = projectionMatrix * tempCamPosition;
    cameraSpacePosition = vec3(tempCamPosition);

    cameraSpaceNormal = normalize(modelViewMatrixForNormal * normal);
}

