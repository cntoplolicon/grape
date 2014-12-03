#version 330

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

uniform float specularIntensity;
uniform float shiness;
uniform sampler2D textureSampler;
uniform DirectionalLight directionalLight;

vec4 LightContribute(BaseLight light, vec3 lightDireciton)
{
    vec4 color = vec4(light.color, 1.0);
    float cosIncidence = dot(-lightDireciton, cameraSpaceNormal);
    cosIncidence = clamp(cosIncidence, 0.0, 1.0);
    vec3 viewDirection = normalize(-cameraSpacePosition);
    vec3 refectDirection = normalize(reflect(lightDireciton, cameraSpaceNormal));
    float specularFactor = pow(dot(viewDirection, refectDirection), shiness);
    specularFactor = cosIncidence == 0.0 ? 0.0 : specularFactor;
    specularFactor = clamp(specularFactor, 0.0, 1.0);
    return color * light.ambientIntensity + 
        color * cosIncidence * light.diffuseIntensity +
        color * specularFactor * specularIntensity;
}

vec4 DirectionalLightContribute(DirectionalLight light)
{
    return LightContribute(light.base, light.direction);
}

void main()
{
    vec4 lightFactor = DirectionalLightContribute(directionalLight);
    fragColor = texture(textureSampler, texCoord0) * lightFactor;
}

