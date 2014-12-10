#version 410 core

layout(triangles, equal_spacing) in;

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

in patch OutputPatch outputPatch;

out vec3 position_fs_in;
out vec3 normal_fs_in;
out vec2 texCoord_fs_in;

uniform mat4 projectionMatrix;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
    texCoord_fs_in = interpolate2D(outputPatch.texCoord[0], outputPatch.texCoord[1], outputPatch.texCoord[2]);
    normal_fs_in = interpolate3D(outputPatch.normal[0], outputPatch.normal[1], outputPatch.normal[2]);
    normal_fs_in = normalize(normal_fs_in);

    float w = gl_TessCoord.x;
    float u = gl_TessCoord.y;
    float v = gl_TessCoord.z;

    position_fs_in = 
        outputPatch.b300 * 1.0 * w * w * w +
        outputPatch.b030 * 1.0 * u * u * u +
        outputPatch.b003 * 1.0 * v * v * v +
        outputPatch.b210 * 3.0 * w * w * u +
        outputPatch.b120 * 3.0 * w * u * u +
        outputPatch.b201 * 3.0 * w * w * v +
        outputPatch.b021 * 3.0 * u * u * v +
        outputPatch.b102 * 3.0 * w * v * v +
        outputPatch.b012 * 3.0 * u * v * v +
        outputPatch.b111 * 6.0 * w * u * v;

    gl_Position = projectionMatrix * vec4(position_fs_in, 1.0);
}

