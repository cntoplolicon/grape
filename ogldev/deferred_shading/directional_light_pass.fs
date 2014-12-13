#version 330

struct BaseLight
{
    vec3 color;
    float ambientIntensity;                                                         
    float diffuseIntensity;                                                         
};

struct DirectionalLight                                                             
{                                                                                   
    BaseLight base;                                                                 
    vec3 direction;                                                                 
};

uniform sampler2D colorSampler;
uniform sampler2D normalSampler;
uniform sampler2D positionSampler;
uniform mat4 viewMatrix;
uniform DirectionalLight directionalLight;
uniform float specularIntensity;
uniform float shiness;
uniform vec2 screenSize;

out vec4 fragColor;

vec4 LightContribute(BaseLight light, vec3 lightDirection, vec3 cameraSpacePosition, vec3 cameraSpaceNormal)
{
    vec3 normal = normalize(cameraSpaceNormal);
    lightDirection = normalize(lightDirection);

    vec4 color = vec4(light.color, 1.0);
    float cosIncidence = dot(-lightDirection, normal);
    cosIncidence = clamp(cosIncidence, 0.0, 1.0);
    vec3 viewDirection = normalize(-cameraSpacePosition);
    vec3 refectDirection = normalize(reflect(lightDirection, normal));
    float specularFactor = pow(dot(viewDirection, refectDirection), shiness);
    specularFactor = cosIncidence == 0.0 ? 0.0 : specularFactor;
    specularFactor = max(0.0, specularFactor);
    return color * light.ambientIntensity + 
        color * cosIncidence * light.diffuseIntensity +
        color * specularFactor * specularIntensity;
}

vec4 DirectionalLightContribute(DirectionalLight light, vec3 cameraSpacePosition, vec3 cameraSpaceNormal)
{
    vec3 lightDirection = (viewMatrix * vec4(light.direction, 0.0)).xyz;
    return LightContribute(light.base, lightDirection, cameraSpacePosition, cameraSpaceNormal);
}

void main()
{
    vec2 texCoord = gl_FragCoord.xy / screenSize;
	vec3 color = texture(colorSampler, texCoord).xyz;
	vec3 cameraSpacePosition = texture(positionSampler, texCoord).xyz;
	vec3 cameraSpaceNormal = texture(normalSampler, texCoord).xyz;
	fragColor = vec4(color, 1.0) * DirectionalLightContribute(directionalLight, cameraSpacePosition, cameraSpaceNormal); 
}

