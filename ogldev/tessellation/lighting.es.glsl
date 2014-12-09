#version 410 core

layout(triangles, equal_spacing) in;

in vec3 position_es_in[];
in vec3 normal_es_in[];
in vec2 texCoord_es_in[];

out vec3 position_fs_in;
out vec3 normal_fs_in;
out vec2 texCoord_fs_in;

uniform sampler2D displacementMapSampler;
uniform float displacementFactor;
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
    texCoord_fs_in = interpolate2D(texCoord_es_in[0], texCoord_es_in[1], texCoord_es_in[2]);
    normal_fs_in = interpolate3D(normal_es_in[0], normal_es_in[1], normal_es_in[2]);
    normal_fs_in = normalize(normal_fs_in);
    position_fs_in = interpolate3D(position_es_in[0], position_es_in[1], position_es_in[2]);

    float displacement = texture(displacementMapSampler, texCoord_fs_in.xy).x;
    position_fs_in += normal_fs_in * displacement * displacementFactor;
    gl_Position = projectionMatrix * vec4(position_fs_in, 1.0);
}

