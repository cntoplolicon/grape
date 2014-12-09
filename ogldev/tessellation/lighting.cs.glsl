#version 410 core

layout (vertices = 3) out;

in vec3 position_cs_in[];
in vec3 normal_cs_in[];
in vec2 texCoord_cs_in[];

out vec3 position_es_in[];
out vec3 normal_es_in[];
out vec2 texCoord_es_in[];

float GetTessLevel(float dist0, float dist1)
{
    float avg = (dist0 + dist1) / 2.0;

    if (avg <= 2.0) {
        return 10.0;
    } else if (avg <= 5.0) {
        return 7.0;
    } else {
        return 3.0;
    }
}

void main()
{
    position_es_in[gl_InvocationID] = position_cs_in[gl_InvocationID];
    normal_es_in[gl_InvocationID] = normal_cs_in[gl_InvocationID];
    texCoord_es_in[gl_InvocationID] = texCoord_cs_in[gl_InvocationID];

    float d0 = length(position_cs_in[0]);
    float d1 = length(position_cs_in[1]);
    float d2 = length(position_cs_in[2]);

    gl_TessLevelOuter[0] = GetTessLevel(d1, d2);
    gl_TessLevelOuter[1] = GetTessLevel(d2, d0);
    gl_TessLevelOuter[2] = GetTessLevel(d0, d1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}

