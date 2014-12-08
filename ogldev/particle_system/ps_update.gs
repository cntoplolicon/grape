#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

in float type0[];
in vec3 position0[];
in vec3 velocity0[];
in float age0[];

out float type1;
out vec3 position1;
out vec3 velocity1;
out float age1;

uniform float deltaTime;
uniform float time;
uniform sampler1D randomTextureSampler;
uniform float launcherLifetime;
uniform float shellLifetime;
uniform float secondShellLifetime;

const float PARTICLE_TYPE_LAUNCHER = 0.0f;
const float PARTICLE_TYPE_SHELL = 1.0f;
const float PARTICLE_TYPE_SECONDARY_SHELL = 2.0f;

vec3 GetRandomDirection(float texCoord)
{
     vec3 direction = texture(randomTextureSampler, texCoord).xyz;
     direction -= vec3(0.5, 0.5, 0.5);
     return direction;
}

void main()
{
    float age = age0[0] + deltaTime;

    if (type0[0] == PARTICLE_TYPE_LAUNCHER) {
        if (age >= launcherLifetime) {
            type1 = PARTICLE_TYPE_SHELL;
            position1 = position0[0];
            vec3 direction = GetRandomDirection(time / 1000.0);
            direction.y = max(direction.y, 0.5);
            velocity1 = normalize(direction) / 20.0;
            age1 = 0.0;
            EmitVertex();
            EndPrimitive();
            age = 0.0;
        }

        type1 = PARTICLE_TYPE_LAUNCHER;
        position1 = position0[0];
        velocity1 = velocity0[0];
        age1 = age;
        EmitVertex();
        EndPrimitive();
    } else {
        float deltaSecs = deltaTime / 1000.0f;
        float t1 = age0[0] / 1000.0;
        float t2 = age / 1000.0;
        vec3 deltaPosition = deltaSecs * velocity0[0];
        vec3 deltaVelocity = vec3(deltaSecs) * (0.0, -9.81, 0.0);

        if (type0[0] == PARTICLE_TYPE_SHELL)  {
	        if (age < shellLifetime) {
	            type1 = PARTICLE_TYPE_SHELL;
	            position1 = position0[0] + deltaPosition;
	            velocity1 = velocity0[0] + deltaVelocity;
	            age1 = age;
	            EmitVertex();
	            EndPrimitive();
	        } else {
                for (int i = 0 ; i < 10 ; i++) {
                     type1 = PARTICLE_TYPE_SECONDARY_SHELL;
                     position1 = position0[0];
                     vec3 direction = GetRandomDirection((time + i)/1000.0);
                     velocity1 = normalize(direction) / 20.0;
                     age1 = 0.0f;
                     EmitVertex();
                     EndPrimitive();
                }
            }
        }
        else {
            if (age < secondShellLifetime) {
                type1 = PARTICLE_TYPE_SECONDARY_SHELL;
                position1 = position0[0] + deltaPosition;
                velocity1 = velocity0[0] + deltaVelocity;
                age1 = age;
                EmitVertex();
                EndPrimitive();
            }
        }
    }
}

