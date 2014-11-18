#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 inDiffuseColor;
layout(location = 2) in vec3 normal;

out vec4 diffuseColor;
out vec3 vertexNormal;
out vec3 cameraSpacePosition;

uniform mat4 modelViewMatrix;
uniform mat3 modelViewMatrixForNormal;

uniform Projection
{
	mat4 projectionMatrix;
};

void main()
{
	vec4 tempCamPosition = (modelViewMatrix * vec4(position, 1.0));
	gl_Position = projectionMatrix * tempCamPosition;

	vertexNormal = modelViewMatrixForNormal * normal;
	diffuseColor = inDiffuseColor;
	cameraSpacePosition = vec3(tempCamPosition);
}

