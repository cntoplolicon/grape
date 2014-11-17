#version 330

smooth in vec4 cameraSpacePosition;
smooth in vec3 cameraSpaceNormal;

uniform vec3 lightPos;
uniform vec4 lightIntensity;
uniform vec4 ambientIntensity;
uniform float lightAttenuation;

out vec4 outputColor;

void main()
{
    vec3 lightDiff = lightPos - vec3(cameraSpacePosition);
    float lightDistSqr = dot(lightDiff, lightDiff);
    vec3 dirToLight = lightDiff / inversesqrt(lightDistSqr);
    float lightFactor = 1.0 / (1.0 + lightDistSqr * lightAttenuation);
	
	float cosAngIncidence = dot(cameraSpaceNormal, dirToLight);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);
	
	outputColor = lightFactor * lightIntensity * cosAngIncidence + ambientIntensity;
}

