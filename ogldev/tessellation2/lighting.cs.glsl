#version 410 core

layout (vertices = 1) out;

in vec3 position_cs_in[];
in vec3 normal_cs_in[];
in vec2 texCoord_cs_in[];

struct OutputPatch
{
    vec3 b030;
    vec3 b021;
    vec3 b012;
    vec3 b003;
    vec3 b102;
    vec3 b201;
    vec3 b300;
    vec3 b210;
    vec3 b120;
    vec3 b111;
    vec3 normal[3];
    vec2 texCoord[3];
};

out patch OutputPatch outputPatch;

uniform float tessellationLevel;

void CalcPositions()
{
    vec3 p1 = position_cs_in[0];
    vec3 p2 = position_cs_in[1];
    vec3 p3 = position_cs_in[2];

    vec3 n1 = normal_cs_in[0];
    vec3 n2 = normal_cs_in[1];
    vec3 n3 = normal_cs_in[2];

    float w12 = dot(p2 - p1, n1);
    float w21 = dot(p1 - p2, n2);
    float w13 = dot(p3 - p1, n1);
    float w31 = dot(p1 - p3, n3);
    float w23 = dot(p3 - p2, n2);
    float w32 = dot(p2 - p3, n3);

    outputPatch.b300 = p1;
    outputPatch.b030 = p2;
    outputPatch.b003 = p3;

    outputPatch.b210 = (2 * p1 + p2 - w12 * n1) / 3.0;
    outputPatch.b120 = (2 * p2 + p1 - w21 * n2) / 3.0;
    outputPatch.b021 = (2 * p2 + p3 - w23 * n2) / 3.0;
    outputPatch.b012 = (2 * p3 + p2 - w32 * n3) / 3.0;
    outputPatch.b102 = (2 * p3 + p1 - w31 * n3) / 3.0;
    outputPatch.b201 = (2 * p1 + p3 - w13 * n1) / 3.0;

    vec3 e = (outputPatch.b210 + outputPatch.b120 + outputPatch.b021 + 
            outputPatch.b012 + outputPatch.b102 + outputPatch.b201) / 6.0;
    vec3 v = (p1 + p2 + p3) / 3.0;
    outputPatch.b111 = e + (e - v) / 2.0;
}

void main()
{
    for (int i = 0 ; i < 3 ; i++) {
        outputPatch.normal[i] = normal_cs_in[i];
        outputPatch.texCoord[i] = texCoord_cs_in[i];
    }

    CalcPositions();

    gl_TessLevelOuter[0] = tessellationLevel;
    gl_TessLevelOuter[1] = tessellationLevel;
    gl_TessLevelOuter[2] = tessellationLevel;
    gl_TessLevelInner[0] = tessellationLevel;
}

