#version 330

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in vec3 cameraSpacePosition[];
out vec2 texCoord;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform float billboardSize;

void main()
{
    vec3 up = (modelViewMatrix * vec4(0.0, 1.0, 0.0, 0.0)).xyz;
    vec3 eye = normalize(cameraSpacePosition[0]);
    vec3 right = cross(up, eye);

    float size = billboardSize / 2.0;

    vec3 pos = cameraSpacePosition[0] - right * size - up * size;
    gl_Position = projectionMatrix * vec4(pos, 1.0);
    texCoord = vec2(0.0, 0.0);
    EmitVertex();

    pos = cameraSpacePosition[0] - right * size + up * size;
    gl_Position = projectionMatrix * vec4(pos, 1.0);
    texCoord = vec2(0.0, 1.0);
    EmitVertex();

    pos = cameraSpacePosition[0] + right * size - up * size;
    gl_Position = projectionMatrix * vec4(pos, 1.0);
    texCoord = vec2(1.0, 0.0);
    EmitVertex();

    pos = cameraSpacePosition[0] + right * size + up * size;
    gl_Position = projectionMatrix * vec4(pos, 1.0);
    texCoord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}

