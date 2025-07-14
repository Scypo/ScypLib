#version 450 core

in vec2 vTexCoord;
in float vTexSlot;
in vec4 vColorTint;

out vec4 FragColor;
uniform sampler2D uTextures[32];
uniform float uTime;

void main()
{
    int slot = int(vTexSlot);
    vec4 texColor = texture(uTextures[slot], vTexCoord);

    float wave = sin(vTexCoord.x * 20.0 + uTime) * 0.5 + 0.5;
    float wave2 = sin(vTexCoord.y * 30.0 + uTime * 1.5) * 0.5 + 0.5;

    vec3 colorShift = vec3(wave, wave2, 1.0 - wave);

    vec4 finalColor = vec4(texColor.rgb * colorShift, texColor.a);
    if (finalColor.a < 0.1) discard;

    FragColor = finalColor;
}