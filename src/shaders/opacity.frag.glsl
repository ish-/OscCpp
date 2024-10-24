#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;

ivec2 resolution = textureSize(texture0, 0);

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    finalColor = texelColor * .995;
}
