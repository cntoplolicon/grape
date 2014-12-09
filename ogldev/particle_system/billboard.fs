#version 330

out vec4 fragColor;

uniform sampler2D billboardSampler;
in vec2 texCoord;

void main()
{
    fragColor = texture(billboardSampler, texCoord);
    if (fragColor.r > 0.9 && fragColor.g > 0.9 && fragColor.b > 0.9) {
        discard;
    }
}

