#version 410 core

const int MAX_POINT_LIGHTS = 2;                                                     
const int MAX_SPOT_LIGHTS = 2;                                                      

in vec3 position_fs_in;
in vec3 normal_fs_in;
in vec2 texCoord_fs_in;

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

uniform mat4 viewMatrix;
uniform float specularIntensity;
uniform float shiness;
uniform sampler2D textureSampler;
uniform DirectionalLight directionalLight;
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numSpotLights;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

vec4 LightContribute(BaseLight light, vec3 lightDirection, vec3 normal)
{
    vec4 color = vec4(light.color, 1.0);
    float cosIncidence = dot(-lightDirection, normal);
    cosIncidence = clamp(cosIncidence, 0.0, 1.0);
    vec3 viewDirection = normalize(-position_fs_in);
    vec3 refectDirection = normalize(reflect(lightDirection, normal));
    float specularFactor = pow(dot(viewDirection, refectDirection), shiness);
    specularFactor = cosIncidence == 0.0 ? 0.0 : specularFactor;
    specularFactor = max(0.0, specularFactor);
    return color * light.ambientIntensity + 
        color * cosIncidence * light.diffuseIntensity +
        color * specularFactor * specularIntensity;
}

vec4 DirectionalLightContribute(DirectionalLight light, vec3 normal)
{
    vec3 lightDirection = (viewMatrix * vec4(light.direction, 0.0)).xyz;
    return LightContribute(light.base, normalize(lightDirection), normal);
}

vec4 PointLightContribute(PointLight light, vec3 normal)
{
    vec3 cameraSpaceLightPos = (viewMatrix * vec4(light.position, 1.0)).xyz;
    vec3 direction = position_fs_in - cameraSpaceLightPos;
    float dist = length(direction);
    float attenuation = light.attenuation.constant + light.attenuation.linear * dist +
        light.attenuation.quadratic * dist * dist;
    return LightContribute(light.base, direction / dist, normal) / attenuation;
}

vec4 SpotLightContribute(SpotLight light, vec3 normal)
{
    vec3 cameraSpaceLightPos = (viewMatrix * vec4(light.base.position, 1.0)).xyz;
    vec3 direction = normalize(position_fs_in - cameraSpaceLightPos);
    vec3 cameraSpaceLightDir = normalize((viewMatrix * vec4(light.direction, 0.0)).xyz);
    vec4 color = PointLightContribute(light.base, normal);

    float spotFactor = dot(direction, cameraSpaceLightDir);
    float cutoff = cos(radians(light.cutoff));
    spotFactor = max(cutoff, spotFactor);

    return color * (1.0 - (1.0 - spotFactor) / (1.0 - cutoff));
}

void main()
{
    vec3 normal = normalize(normal_fs_in);
    vec4 lightFactor = DirectionalLightContribute(directionalLight, normal);
    for (int i = 0; i < numPointLights; i++) {
        lightFactor += PointLightContribute(pointLights[i], normal);
    }
    for (int i = 0; i < numSpotLights; i++) {
        lightFactor += SpotLightContribute(spotLights[i], normal);
    }
    fragColor = texture(textureSampler, texCoord_fs_in) * lightFactor;
}

