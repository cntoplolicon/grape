#version 330

smooth in vec4 cameraSpacePosition;
smooth in vec3 cameraSpaceNormal;

uniform vec3 lightPos;
uniform vec4 lightIntensity;
uniform vec4 ambientIntensity;

out vec4 outputColor;

void main()
{
    vec3 dirToLight = normalize(lightPos - vec3(cameraSpacePosition));
	
	float cosAngIncidence = dot(cameraSpaceNormal, dirToLight);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);
	
	outputColor = lightIntensity * cosAngIncidence + ambientIntensity;
}

