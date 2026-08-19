#version 450

// Vertex Shader
#include "Passes/ScreenTriangle.glsl"

#ifdef FRAGMENT_SHADER

float InverseLerp(float A, float B, float T)
{
    return clamp((T - A) / (B - A), 0.0f, 1.0f);
}

vec3 InverseLerp(vec3 A, vec3 B, float T)
{
    return clamp((T - A) / (B - A), 0.0f, 1.0f);
}

layout(location = 0) in vec2 UV;
layout(location = 1) in vec2 UVProj;

out vec4 OutColor;

#ifdef DISPLAY_PASS
#include "Include/ToneMapping.glsl"

uniform sampler2D MotionVectors;

uniform vec2 RangeRed;
uniform vec2 RangeGreen;
uniform vec2 RangeBlue;

uniform bool UseOETF;

void main()
{
    vec4 finalColor = texture(MotionVectors, UV);

    // Handle ranges
    finalColor.x = InverseLerp(RangeRed.x, RangeRed.y, finalColor.x);
    finalColor.y = InverseLerp(RangeGreen.x, RangeGreen.y, finalColor.y);
    finalColor.z = InverseLerp(RangeBlue.x, RangeBlue.y, finalColor.z);
    
    // EOTF
    if (UseOETF)
    {
        finalColor.xyz = clamp(finalColor.xyz, 0, 1);

        finalColor.xyz = SRGB_OETF(finalColor.xyz, 2.2 /* PC (Or PC monitor) Gamma, TV is 2.4*/);
    }

    OutColor = finalColor;
}
#endif // DISPLAY_PASS

#ifdef MOTION_VECTORS_ARROWS_PASS

float sdSegment(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p-a;
    vec2 ba = b-a;

    float h = clamp(dot(pa,ba)/dot(ba,ba),0.0,1.0);

    return length(pa-ba*h);
}

uniform sampler2D MotionVectors;
uniform uvec2 viewportSize;

uniform uint gridSize;
uniform float motionScale;

void main()
{    
    vec2 pixel = gl_FragCoord.xy;
    vec2 cell = floor(pixel / vec2(gridSize));
    vec2 center = (cell + 0.5) * vec2(gridSize);
    
    vec3 motion = texelFetch(MotionVectors, ivec2(center), 0).xyz;
    vec2 pixelDisplacement = motion.xy * vec2(viewportSize);
    
    if (length(pixelDisplacement) < 1.0f) discard;

    // pixelDisplacement *= motionScale;
    // float shaft = sdSegment(pixel, center, center + pixelDisplacement);

    // Arrow basis
    vec2 dir = normalize(pixelDisplacement);
    vec2 perp = vec2(-dir.y, dir.x);

    // Local coordinates
    vec2 local = pixel - center;
    float x = dot(local, dir);
    float y = dot(local, perp);

    // Shaft
    float shaftLength = length(pixelDisplacement) * motionScale;
    float shaft = sdSegment(vec2(x,y), vec2(0,0), vec2(shaftLength,0));

    // Head
    float headLength = 5.0;
    float headWidth  = 3.0;
    vec2 tip = vec2(shaftLength + headLength,0);
    float head1 = sdSegment( vec2(x,y), tip, vec2(shaftLength, headWidth));
    float head2 = sdSegment( vec2(x,y), tip, vec2(shaftLength,-headWidth));

    // Thickness
    float d = min(shaft,min(head1,head2));
    float alpha = smoothstep(1.5,0.5,d);

    // Color
    float speed = length(motion);
    vec3 color = mix( vec3(0,0,1), vec3(1,0,0), clamp(speed*10.0,0.0,1.0));

    OutColor.xyz = color;
    OutColor.w = alpha;
}

#endif // MOTION_VECTORS_ARROWS_PASS

#endif // FRAGMENT_SHADER