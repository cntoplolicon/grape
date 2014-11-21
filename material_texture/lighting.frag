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

uniform sampler2D gaussianTexture;

void main()
{
    vec4 accumulateColor = lighting.ambientIntensity * material.diffuseColor;
    for (int i = 0; i < LIGHT_COUNT; i++) {
        vec3 lightDirection;
        vec4 lightIntensity;
        if (lighting.lights[i].cameraSpaceLightPosition.w == 0) {
            lightDirection = normalize(vec3(lighting.lights[i].cameraSpaceLightPosition));
            lightIntensity = lighting.lights[i].lightIntensity;
        } else {
            vec3 lightDiff = vec3(lighting.lights[i].cameraSpaceLightPosition) - cameraSpacePosition;
            float distSqr = dot(lightDiff, lightDiff);
            lightDirection = lightDiff * inversesqrt(distSqr);
            float attenuation = 1.0 / (1.0 + lighting.lightAttenuation * distSqr);
            lightIntensity = attenuation * lighting.lights[i].lightIntensity;
        }

        vec3 surfaceNormal = normalize(cameraSpaceNormal);
        float cosAngIncidence = dot(surfaceNormal, lightDirection);
        cosAngIncidence = clamp(cosAngIncidence, 0.0, 1.0);

        vec3 viewDirection = normalize(-cameraSpacePosition);
        vec3 halfAngle = normalize(viewDirection + lightDirection);
        vec2 textureCoord = vec2(material.specularShininess, dot(halfAngle, surfaceNormal));
        float gaussianTerm = texture(gaussianTexture, textureCoord).r;

        gaussianTerm = cosAngIncidence == 0.0 ? 0.0 : gaussianTerm;

        accumulateColor = accumulateColor + cosAngIncidence * material.diffuseColor * lightIntensity + 
            gaussianTerm * material.specularColor * lightIntensity;
    }

    vec4 gamma = vec4(1.0 / 2.2);
    outputColor = pow(accumulateColor, gamma);
}

