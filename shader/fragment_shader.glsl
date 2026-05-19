#version 330 core

in float fragBrightness;
in vec2 fragUV;
in float overlay;

out vec4 color;

uniform sampler2D atlasTexture;

void main()
{
    color = fragBrightness * texture(atlasTexture, fragUV);

    if(overlay >= 0.99f)
    {
        color += vec4(0.15f, 0.15f, 0.15f, 0);
    }
}