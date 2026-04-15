#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;
uniform vec2 u_camera_pos;
uniform vec2 u_resolution;
uniform vec4 u_viewport;
uniform float u_zoom;


float hash12(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash12(i);
    float b = hash12(i + vec2(1.0, 0.0));
    float c = hash12(i + vec2(0.0, 1.0));
    float d = hash12(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm3(vec2 p)
{
    float n = 0.0;
    n += noise(p) * 0.57;
    n += noise(p * 2.03 + vec2(11.4, -8.2)) * 0.29;
    n += noise(p * 3.97 + vec2(-3.7, 4.9)) * 0.14;
    return n;
}

void main()
{
    vec4 base = texture(texture0, fragTexCoord) * fragColor;

     // Ignore near-empty pixels so effect primarily coats rendered world content.
    float coverage = smoothstep(0.02, 0.30, base.a);
    vec2 screenUv = (gl_FragCoord.xy - u_viewport.xy) / max(u_viewport.zw, vec2(1.0));
    screenUv.y = 1.0 - screenUv.y;

    vec2 worldPx = (screenUv - 0.5) * (u_resolution / max(u_zoom, 0.001)) + u_camera_pos;

    // Hybrid mapping: mostly world-anchored, slightly screen-anchored to break repeating texel feel.
    vec2 pWorld = worldPx * 0.047;
    vec2 pScreen = screenUv * vec2(28.0, 18.0);
    vec2 p = mix(pWorld, pScreen, 0.22);

    // Subtle animated drift so the effect feels organic but does not shimmer hard.
    vec2 drift = vec2(fbm3(p * 0.65 + vec2(0.0, time * 0.035)),
                      fbm3(p * 0.65 + vec2(9.7, -time * 0.030))) - 0.5;
    p += drift * 0.65;

    // Layered directional brush response (cheap: no loops).
    vec2 d1 = normalize(vec2(0.92, 0.38));
    vec2 d2 = normalize(vec2(-0.54, 0.84));
    vec2 d3 = normalize(vec2(-0.87, -0.49));

    float s1 = smoothstep(-0.45, 0.52, sin(dot(p, d1) * 5.8 + fbm3(p * 0.90) * 2.0));
    float s2 = smoothstep(-0.40, 0.48, sin(dot(p, d2) * 4.9 - fbm3(p * 1.07) * 1.7));
    float s3 = smoothstep(-0.36, 0.44, sin(dot(p, d3) * 4.4 + fbm3(p * 1.23) * 1.4));

    float blendA = fbm3(p * 0.72 + vec2(4.0, -3.0));
    float blendB = fbm3(p * 1.34 + vec2(-6.0, 2.0));
    float strokes = mix(mix(s1, s2, blendA), s3, blendB * 0.58);

    // Ink wash field + edge breakup.
    float wash = smoothstep(0.20, 0.92, fbm3(p * 0.55 + vec2(2.0, 8.0)));
    float breakup = smoothstep(0.12, 0.92, fbm3(p * 1.65 + vec2(-4.3, 3.7)));

    float paintMask = mix(wash, strokes * (0.70 + 0.45 * breakup), 0.66);
    paintMask = smoothstep(0.23, 0.95, paintMask);

    // Tone control: gentle wash + restrained value range so gameplay readability stays high.

    vec3 col = base.rgb;

    float washAmt = paintMask * coverage;

    // --- INK DARKEN ---
    col *= 1.0 - washAmt * 0.18;                                  // ink darkening
    col = mix(col, col * vec3(1.045, 1.020, 0.955), washAmt * 0.16); // paper warmth

    // Slight desaturation for "ancient ink" look, but keep color legibility.
    float luma = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(col, vec3(luma), 0.07 * washAmt);
    
    // Fine grain to avoid plasticky smoothness.
    float grain = hash12(gl_FragCoord.xy + vec2(time * 7.0, time * 5.0));
    col += (grain - 0.5) * (0.010 * coverage);

    

    finalColor = vec4(clamp(col, 0.0, 1.0), base.a);
}