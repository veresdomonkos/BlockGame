#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in float brightness;
layout(location = 2) in vec2 UV;

out float fragBrightness;
out vec2 fragUV;
out float overlay;

uniform mat4 view;
uniform mat4 projection;
uniform ivec3 selection;
uniform float fun;

void main()
{
    if((selection.x + 1 == position.x || selection.x == position.x) && (selection.z + 1 == position.z || selection.z == position.z) && (selection.y + 1 == position.y || selection.y == position.y))
    {
        overlay = 1;
    }
    else
    {
        overlay = 0;
    }

    fragBrightness = brightness;
    fragUV = UV;
    gl_Position = projection * view * vec4(position, 1.0);
}