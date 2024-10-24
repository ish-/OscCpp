in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;

const vec2 resolution = vec2(1280, 720);

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    finalColor = texelColor * .995;
}
