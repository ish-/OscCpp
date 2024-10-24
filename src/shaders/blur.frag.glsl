#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;

const vec2 resolution = vec2(1280, 720);

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  color += texture(image, uv) * 0.1964825501511404;
  color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
  return color;
}

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

#ifndef FNC_GAUSSIAN
#define FNC_GAUSSIAN
float gaussian(float d, float s) { return exp(-(d*d) / (2.0 * s*s)); }
float gaussian( vec2 d, float s) { return exp(-( d.x*d.x + d.y*d.y) / (2.0 * s*s)); }
float gaussian( vec3 d, float s) { return exp(-( d.x*d.x + d.y*d.y + d.z*d.z ) / (2.0 * s*s)); }
float gaussian( vec4 d, float s) { return exp(-( d.x*d.x + d.y*d.y + d.z*d.z + d.w*d.w ) / (2.0 * s*s)); }
#endif

#ifndef GAUSSIANBLUR2D_TYPE
#ifdef GAUSSIANBLUR_TYPE
#define GAUSSIANBLUR2D_TYPE GAUSSIANBLUR_TYPE
#else
#define GAUSSIANBLUR2D_TYPE vec4
#endif
#endif

#ifndef GAUSSIANBLUR2D_SAMPLER_FNC
#ifdef GAUSSIANBLUR_SAMPLER_FNC
#define GAUSSIANBLUR2D_SAMPLER_FNC(TEX, UV) GAUSSIANBLUR_SAMPLER_FNC(TEX, UV)
#else
#define GAUSSIANBLUR2D_SAMPLER_FNC(TEX, UV) sampleClamp2edge(TEX, UV)
#endif
#endif

#ifndef FNC_GAUSSIANBLUR2D
#define FNC_GAUSSIANBLUR2D
GAUSSIANBLUR2D_TYPE gaussianBlur2D(in SAMPLER_TYPE tex, in vec2 st, in vec2 offset, const int kernelSize) {
    GAUSSIANBLUR2D_TYPE accumColor = GAUSSIANBLUR2D_TYPE(0.);

    #ifndef GAUSSIANBLUR2D_KERNELSIZE

        #if defined(PLATFORM_WEBGL)
            #define GAUSSIANBLUR2D_KERNELSIZE 20
            float kernelSizef = float(kernelSize);
        #else
            #define GAUSSIANBLUR2D_KERNELSIZE kernelSize
            float kernelSizef = float(GAUSSIANBLUR2D_KERNELSIZE);
        #endif

    #else
        float kernelSizef = float(GAUSSIANBLUR2D_KERNELSIZE);
    #endif

    float accumWeight = 0.;
    const float k = 0.15915494; // 1 / (2*PI)
    vec2 xy = vec2(0.0);
    for (int j = 0; j < GAUSSIANBLUR2D_KERNELSIZE; j++) {
        #if defined(PLATFORM_WEBGL)
        if (j >= kernelSize)
            break;
        #endif
        xy.y = -.5 * (kernelSizef - 1.) + float(j);
        for (int i = 0; i < GAUSSIANBLUR2D_KERNELSIZE; i++) {
            #if defined(PLATFORM_WEBGL)
            if (i >= kernelSize)
                break;
            #endif
            xy.x = -0.5 * (kernelSizef - 1.) + float(i);
            float weight = (k / kernelSizef) * gaussian(xy, kernelSizef);
            accumColor += weight * GAUSSIANBLUR2D_SAMPLER_FNC(tex, st + xy * offset);
            accumWeight += weight;
        }
    }
    return accumColor / accumWeight;
}
#endif

void main()
{
    // vec4 texelColor = blur13(texture0, fragTexCoord, resolution, vec2(1.));
    vec4 texelColor = gaussianBlur2D(texture0, fragTexCoord, (1. / resolution), 15);
    finalColor = texelColor;
}
