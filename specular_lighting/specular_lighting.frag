#version 330

in vec4 diffuseColor;
in vec3 vertexNormal;
in vec3 cameraSpacePosition;

out vec4 outputColor;

uniform vec4 lightIntensity;
uniform vec4 ambientIntensity;
uniform vec3 cameraSpaceLightPos;
uniform float lightAttenuation;
uniform float shininessFactor;

const vec4 specularColor = vec4(0.25, 0.25, 0.25, 1);

void main()
{ 
    vec3 lightDiff = cameraSpaceLightPos - vec3(cameraSpacePosition);
    float lightDistSqr = dot(lightDiff, lightDiff);
    vec3 lightDir = lightDiff / inversesqrt(lightDistSqr);
    float attenuationFactor = 1.0 / (1.0 + lightDistSqr * lightAttenuation);
    
	vec3 surfaceNormal = normalize(vertexNormal);
    float cosAngIncidence = dot(surfaceNormal, lightDir);
    cosAngIncidence = clamp(cosAngIncidence, 0, 1);
    
    vec3 viewDir = normalize(-cameraSpacePosition);
    vec3 halfAngle = normalize(lightDir + viewDir);
    float angleNormalHalf = acos(dot(halfAngle, surfaceNormal));
    float exponent = angleNormalHalf / shininessFactor;
    float gaussianTerm = exp(-(exponent * exponent));
    gaussianTerm = cosAngIncidence == 0.0 ? 0.0 : gaussianTerm;

    outputColor = attenuationFactor * lightIntensity * diffuseColor * cosAngIncidence +
        diffuseColor * ambientIntensity + 
        gaussianTerm * specularColor * attenuationFactor;
}

