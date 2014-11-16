#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 diffuseColor;
layout(location = 2) in vec3 normal;

smooth out vec4 cameraSpacePosition;
smooth out vec3 cameraSpaceNormal;
smooth out vec4 interpDiffuseColor;

uniform mat4 modelViewMatrix;
uniform mat3 modelViewMatrixForNormal;

uniform Projection
{
	mat4 projectionMatrix;
};

void main()
{
	cameraSpacePosition = (modelViewMatrix * vec4(position, 1.0));
	gl_Position = projectionMatrix * cameraSpacePosition;

	cameraSpaceNormal = normalize(modelViewMatrixForNormal * normal);

    interpDiffuseColor = diffuseColor;
}

