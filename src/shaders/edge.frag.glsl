#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;

ivec2 resolution = textureSize(texture0, 0);

#ifndef SAMPLER_FNC
#if __VERSION__ >= 300
#define SAMPLER_FNC(TEX, UV) texture(TEX, UV)
#else
#define SAMPLER_FNC(TEX, UV) texture2D(TEX, UV)
#endif
#endif

#ifndef SAMPLER_TYPE
#define SAMPLER_TYPE sampler2D
#endif

#ifndef FNC_SAMPLECLAMP2EDGE
#define FNC_SAMPLECLAMP2EDGE
vec4 sampleClamp2edge(SAMPLER_TYPE tex, vec2 st, vec2 texResolution) {
    vec2 pixel = 1.0/texResolution;
    return SAMPLER_FNC( tex, clamp(st, pixel, 1.0-pixel) );
}

vec4 sampleClamp2edge(SAMPLER_TYPE tex, vec2 st) {
    return SAMPLER_FNC( tex, clamp(st, vec2(0.01), vec2(0.99) ) );
}

vec4 sampleClamp2edge(SAMPLER_TYPE tex, vec2 st, float edge) {
    return SAMPLER_FNC( tex, clamp(st, vec2(edge), vec2(1.0 - edge) ) );
}
#endif

#ifndef EDGESOBEL_TYPE
#ifdef EDGE_TYPE
#define EDGESOBEL_TYPE EDGE_TYPE
#else
#define EDGESOBEL_TYPE float
#endif
#endif

#ifndef EDGESOBEL_SAMPLER_FNC
#ifdef EDGE_SAMPLER_FNC
#define EDGESOBEL_SAMPLER_FNC(TEX, UV) EDGE_SAMPLER_FNC(TEX, UV)
#else
#define EDGESOBEL_SAMPLER_FNC(TEX, UV) sampleClamp2edge(TEX, UV).r
#endif
#endif

#ifndef FNC_EDGESOBEL
#define FNC_EDGESOBEL
EDGESOBEL_TYPE edgeSobel(in SAMPLER_TYPE tex, in vec2 st, in vec2 offset) {
    // get samples around pixel
    EDGESOBEL_TYPE tleft = EDGESOBEL_SAMPLER_FNC(tex, st + vec2(-offset.x, offset.y));
    EDGESOBEL_TYPE left = EDGESOBEL_SAMPLER_FNC(tex, st + vec2(-offset.x, 0.));
    EDGESOBEL_TYPE bleft = EDGESOBEL_SAMPLER_FNC(tex, st + vec2(-offset.x, -offset.y));
    EDGESOBEL_TYPE top = EDGESOBEL_SAMPLER_FNC(tex, st + vec2(0., offset.y));
    EDGESOBEL_TYPE bottom = EDGESOBEL_SAMPLER_FNC(tex, st + vec2(0., -offset.y));
    EDGESOBEL_TYPE tright = EDGESOBEL_SAMPLER_FNC(tex, st + offset);
    EDGESOBEL_TYPE right = EDGESOBEL_SAMPLER_FNC(tex, st + vec2(offset.x, 0.));
    EDGESOBEL_TYPE bright = EDGESOBEL_SAMPLER_FNC(tex, st + vec2(offset.x, -offset.y));
    EDGESOBEL_TYPE x = tleft + 2. * left + bleft - tright - 2. * right - bright;
    EDGESOBEL_TYPE y = -tleft - 2. * top - tright + bleft + 2. * bottom + bright;
    return sqrt((x * x) + (y * y));
}
#endif

void main()
{
    EDGESOBEL_TYPE a = edgeSobel(texture0, fragTexCoord, (1. / resolution) * 7.);
    a = a * step(.05, a) * .4;
    finalColor = vec4(a);
}
