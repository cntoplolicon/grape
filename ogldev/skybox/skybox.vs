#version 330
                                                                                    
layout (location = 0) in vec3 position;                                             

uniform mat4 modelViewMatrix;                                                                  
uniform mat4 projectionMatrix;                                                                  

out vec3 texCoord0;                                                                 

void main()
{
    vec4 clipSpacePos = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    gl_Position = clipSpacePos.xyww;
    texCoord0 = position;
}

