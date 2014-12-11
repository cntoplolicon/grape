#version 330

layout (triangles_adjacency) in;
layout (line_strip, max_vertices = 6) out;

in vec3 cameraSpacePosition[];

uniform vec3 cameraSpaceLightPosition;

void EmitLine(int i, int j)
{
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();

    gl_Position = gl_in[j].gl_Position;
    EmitVertex();

    EndPrimitive();
}

void main()
{
    vec3 e1 = cameraSpacePosition[2] - cameraSpacePosition[0];
    vec3 e2 = cameraSpacePosition[4] - cameraSpacePosition[0];
    vec3 e3 = cameraSpacePosition[1] - cameraSpacePosition[0];
    vec3 e4 = cameraSpacePosition[3] - cameraSpacePosition[2];
    vec3 e5 = cameraSpacePosition[4] - cameraSpacePosition[2];
    vec3 e6 = cameraSpacePosition[5] - cameraSpacePosition[0];

    vec3 normal = cross(e1,e2);
    vec3 lightDirection = cameraSpaceLightPosition - cameraSpacePosition[0];

    if (dot(normal, lightDirection) > 0.00001) {
        normal = cross(e3, e1);
        if (dot(normal, lightDirection) <= 0) {
            EmitLine(0, 2);
        }

        normal = cross(e4, e5);
        lightDirection = cameraSpaceLightPosition - cameraSpacePosition[2];

        if (dot(normal, lightDirection) <=0) {
            EmitLine(2, 4);
        }

        normal = cross(e2, e6);
        lightDirection = cameraSpaceLightPosition - cameraSpacePosition[4];

        if (dot(normal, lightDirection) <= 0) {
            EmitLine(4, 0);
        }
    }
}

