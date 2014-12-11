#version 330

const int MAX_POINT_LIGHTS = 2;                                                     
const int MAX_SPOT_LIGHTS = 2;                                                      

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

struct SpotLight                                                                            
{                                                                                           
    PointLight base;
    vec3 direction;
    float cutoff;
}; 

uniform mat4 modelViewMatrix;
uniform float specularIntensity;
uniform float shiness;
uniform sampler2D textureSampler;
uniform sampler2D normalSampler;
uniform DirectionalLight directionalLight;
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numSpotLights;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

vec4 LightContribute(BaseLight light, vec3 lightDireciton, vec3 normal)
{
    vec4 color = vec4(light.color, 1.0);
    float cosIncidence = dot(-lightDireciton, normal);
    cosIncidence = clamp(cosIncidence, 0.0, 1.0);
    vec3 viewDirection = normalize(-cameraSpacePosition);
    vec3 refectDirection = normalize(reflect(lightDireciton, normal));
    float specularFactor = pow(dot(viewDirection, refectDirection), shiness);
    specularFactor = cosIncidence == 0.0 ? 0.0 : specularFactor;
    specularFactor = max(0.0, specularFactor);
    return color * light.ambientIntensity + 
        color * cosIncidence * light.diffuseIntensity +
        color * specularFactor * specularIntensity;
}

vec4 DirectionalLightContribute(DirectionalLight light, vec3 normal)
{
    return LightContribute(light.base, normalize(light.direction), normal);
}

vec4 PointLightContribute(PointLight light, vec3 normal)
{
    vec3 cameraSpaceLightPos = (modelViewMatrix * vec4(light.position, 1.0)).xyz;
    vec3 direction = cameraSpacePosition - cameraSpaceLightPos;
    float dist = length(direction);
    float attenuation = light.attenuation.constant + light.attenuation.linear * dist +
        light.attenuation.quadratic * dist * dist;
    return LightContribute(light.base, direction / dist, normal) / attenuation;
}

vec4 SpotLightContribute(SpotLight light, vec3 normal)
{
    vec3 cameraSpaceLightPos = (modelViewMatrix * vec4(light.base.position, 1.0)).xyz;
    vec3 direction = normalize(cameraSpacePosition - cameraSpaceLightPos);
    vec3 cameraSpaceLightDir = normalize((modelViewMatrix * vec4(light.direction, 0.0)).xyz);
    vec4 color = PointLightContribute(light.base, normal);

    float spotFactor = dot(direction, cameraSpaceLightDir);
    float cutoff = cos(radians(light.cutoff));
    spotFactor = max(cutoff, spotFactor);

    return color * (1.0 - (1.0 - spotFactor) / (1.0 - cutoff));
}

void main()
{
    vec3 normal = normalize(cameraSpaceNormal);
    vec4 lightFactor = DirectionalLightContribute(directionalLight, normal);
    for (int i = 0; i < numPointLights; i++) {
        lightFactor += PointLightContribute(pointLights[i], normal);
    }
    for (int i = 0; i < numSpotLights; i++) {
        lightFactor += SpotLightContribute(spotLights[i], normal);
    }
    fragColor = texture(textureSampler, texCoord0) * lightFactor;
}

