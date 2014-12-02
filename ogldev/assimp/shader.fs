#version 330

in vec2 texCoord0;

out vec4 fragColor;

uniform sampler2D textureSampler;

void main()
{
    fragColor = texture(textureSampler, texCoord0);
}

