#version 330

layout(std140) uniform;

in vec3 cameraSpacePosition;
in vec3 cameraSpaceNormal;

out vec4 outputColor;

uniform MaterialBlock
{
    vec4 diffuseColor;
    vec4 specularColor;
    float specularShininess;
} material;

struct Light
{
    vec4 cameraSpaceLightPosition;
    vec4 lightIntensity;
};

const int LIGHT_COUNT = 2;
uniform LightingBlock
{
	vec4 ambientIntensity;
    float lightAttenuation;
    Light lights[LIGHT_COUNT];
} lighting;

void main()
{
    outputColor = material.diffuseColor;
}

