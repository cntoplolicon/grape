#version 330


struct BaseLight                                                                    
{                                                                                   
    vec3 color;
    float ambientIntensity;                                                         
    float diffuseIntensity;                                                         
};                                                                                  
struct Attenuation                                                                  
{                                                                                   
    float constant;                                                                 
    float linear;                                                                   
    float quadratic;                                                                      
}; 

struct PointLight
{                                                                                           
    BaseLight base;
    vec3 position;
    Attenuation attenuation;
};

uniform sampler2D colorSampler;
uniform sampler2D normalSampler;
uniform sampler2D positionSampler;
uniform mat4 cameraMatrix;
uniform PointLight pointLight;
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

vec4 PointLightContribute(PointLight light, vec3 cameraSpacePosition, vec3 cameraSpaceNormal)
{
    vec3 cameraSpaceLightPos = (cameraMatrix * vec4(light.position, 1.0)).xyz;
    vec3 direction = cameraSpacePosition - cameraSpaceLightPos;
    float dist = length(direction);
    float attenuation = light.attenuation.constant + light.attenuation.linear * dist +
        light.attenuation.quadratic * dist * dist;
    //return LightContribute(light.base, direction, cameraSpacePosition, cameraSpaceNormal) / attenuation;
    return LightContribute(light.base, direction, cameraSpacePosition, cameraSpaceNormal);
}

void main()
{
    vec2 texCoord = gl_FragCoord.xy / screenSize;
    vec3 color = texture(colorSampler, texCoord).xyz;
    vec3 cameraSpacePosition = texture(positionSampler, texCoord).xyz;
    vec3 cameraSpaceNormal = texture(normalSampler, texCoord).xyz;
    fragColor = vec4(color, 1.0) * PointLightContribute(pointLight, cameraSpacePosition, cameraSpaceNormal); 
}

