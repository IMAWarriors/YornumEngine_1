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


// ---------------- HASH ----------------
float hash12(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y);
}

// ---------------- SMOOTH NOISE (NO LOOP) ----------------
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash12(i);
    float b = hash12(i + vec2(1,0));
    float c = hash12(i + vec2(0,1));
    float d = hash12(i + vec2(1,1));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(a,b,u.x), mix(c,d,u.x), u.y);
}

// ---------------- MAIN ----------------
void main()
{
    vec4 base = texture(texture0, fragTexCoord) * fragColor;

    // --- WORLD SPACE (UNCHANGED) ---
    vec2 screenUv = (gl_FragCoord.xy - u_viewport.xy) / max(u_viewport.zw, vec2(1.0));
    screenUv.y = 1.0 - screenUv.y;

    vec2 worldPx = (screenUv - 0.5) * (u_resolution / max(u_zoom, 0.001)) + u_camera_pos;

    // --- BASE COORD ---
    vec2 p = worldPx * 0.05;

    // --- CHEAP FLOW DISTORTION (NOT TRUE WARP) ---
    float n1 = noise(p + vec2(0.0, time * 0.05));
    float n2 = noise(p * 1.3 + vec2(5.2, -3.1));

    p += vec2(n1 - 0.5, n2 - 0.5) * 0.8;

    // --- MULTI-DIRECTIONAL BRUSHES (BLENDED, NOT CELLS) ---
    vec2 d1 = normalize(vec2(0.8, 0.6));
    vec2 d2 = normalize(vec2(-0.6, 0.9));
    vec2 d3 = normalize(vec2(-0.9, -0.4));

    float s1 = sin(dot(p, d1) * 6.0);
    float s2 = sin(dot(p, d2) * 5.0);
    float s3 = sin(dot(p, d3) * 4.5);

    // smooth brush shapes
    s1 = smoothstep(-0.3, 0.5, s1);
    s2 = smoothstep(-0.4, 0.4, s2);
    s3 = smoothstep(-0.35, 0.45, s3);

    // blend using noise (THIS removes patchwork)
    float blend = noise(p * 0.7);

    float strokes = mix(mix(s1, s2, blend), s3, blend * 0.5);

    // --- INK DENSITY (WASH) ---
    float wash = noise(p * 0.6 + vec2(2.0, 8.0));
    wash = smoothstep(0.2, 0.9, wash);

    // --- BREAKUP (VERY IMPORTANT) ---
    float breakup = noise(p * 1.8);
    strokes *= 0.6 + breakup * 0.6;

    // --- FINAL MASK ---
    float paintMask = mix(wash, strokes, 0.65);
    paintMask = smoothstep(0.2, 0.95, paintMask);

    vec3 col = base.rgb;

    // --- INK DARKEN ---
    col *= 1.0 - paintMask * 0.28;

    // --- PAPER WARMTH ---
    col = mix(col, col * vec3(1.04, 1.01, 0.94), paintMask * 0.22);

    // --- SUBTLE GRAIN ---
    float grain = hash12(gl_FragCoord.xy + time * 8.0);
    col += (grain - 0.5) * 0.015;

    finalColor = vec4(clamp(col, 0.0, 1.0), base.a);
}