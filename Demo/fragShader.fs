#version 150

uniform sampler2D sampler0;
in vec4 fragColor;
in vec2 fragTexCoords;

out vec4 color;

void main()
{
    color = texture(sampler0, vec2(1.0, 1.0) - fragTexCoords);
}

