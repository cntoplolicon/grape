#version 330

const int MAX_POINT_LIGHTS = 2;                                                     

in vec3 cameraSpacePosition;
in vec3 cameraSpaceNormal;
in vec2 texCoord0;

out vec4 fragColor;

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

uniform mat4 modelViewMatrix;
uniform float specularIntensity;
uniform float shiness;
uniform sampler2D textureSampler;
uniform DirectionalLight directionalLight;
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

vec4 LightContribute(BaseLight light, vec3 lightDireciton)
{
    vec3 normal = normalize(cameraSpaceNormal);
    lightDireciton = normalize(lightDireciton);

    vec4 color = vec4(light.color, 1.0);
    float cosIncidence = dot(-lightDireciton, normal);
    cosIncidence = clamp(cosIncidence, 0.0, 1.0);
    vec3 viewDirection = normalize(-cameraSpacePosition);
    vec3 refectDirection = normalize(reflect(lightDireciton, normal));
    float specularFactor = pow(dot(viewDirection, refectDirection), shiness);
    specularFactor = cosIncidence == 0.0 ? 0.0 : specularFactor;
    return color * light.ambientIntensity + 
        color * cosIncidence * light.diffuseIntensity +
        color * specularFactor * specularIntensity;
}

vec4 DirectionalLightContribute(DirectionalLight light)
{
    return LightContribute(light.base, light.direction);
}

vec4 PointLightContribute(PointLight light)
{
    vec3 cameraSpaceLightPos = (modelViewMatrix * vec4(light.position, 1.0)).xyz;
    vec3 direction = cameraSpacePosition - cameraSpaceLightPos;
    float dist = length(direction);
    float attenuation = light.attenuation.constant + light.attenuation.linear * dist +
        light.attenuation.quadratic * dist * dist;
    return LightContribute(light.base, direction) / attenuation;
}

void main()
{
    vec4 lightFactor = DirectionalLightContribute(directionalLight);
    for (int i = 0; i < numPointLights; i++) {
        lightFactor += PointLightContribute(pointLights[i]);
    }
    fragColor = texture(textureSampler, texCoord0) * lightFactor;
}

