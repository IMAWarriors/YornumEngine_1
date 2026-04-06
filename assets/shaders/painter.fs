#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;

uniform vec2 u_camera_pos;
uniform vec2 u_resolution;
uniform float u_zoom;
uniform float time;

// ---------------- HASH ----------------
float hash(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

// ---------------- NOISE ----------------
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// ---------------- FBM ----------------
float fbm(vec2 p)
{
    float v = 0.0;
    float a = 0.5;

    for (int i = 0; i < 5; i++)
    {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }

    return v;
}

// ---------------- DOMAIN WARP ----------------
vec2 warp(vec2 p)
{
    vec2 q = vec2(
        fbm(p + vec2(0.0, 0.0)),
        fbm(p + vec2(5.2, 1.3))
    );

    vec2 r = vec2(
        fbm(p + 4.0 * q + vec2(1.7, 9.2)),
        fbm(p + 4.0 * q + vec2(8.3, 2.8))
    );

    return p + r * 3.0;
}

// ---------------- MAIN ----------------
void main()
{
    vec4 base = texture(texture0, fragTexCoord) * fragColor;

    // ✅ CORRECT WORLD POSITION
    vec2 screen = fragTexCoord * u_resolution;
    vec2 world = (screen - u_resolution * 0.5) / u_zoom + u_camera_pos;

    // ---------------- PAINTER EFFECT ----------------

    // Scale controls brush size
    vec2 p = world * 0.008;

    // Flowing warp (brush strokes)
    vec2 flow = warp(p * 1.5);

    float ink = fbm(flow);

    // Directional strokes
    float stroke = sin(flow.x * 3.0 + ink * 4.0);

    // Blend structure
    float density = mix(ink, stroke, 0.4);

    // Ink pooling / banding
    density = floor(density * 5.0) / 5.0;

    vec3 col = base.rgb;

    // Darken like ink wash
    col *= (1.0 - density * 0.6);

    // Paper tone
    vec3 paper = vec3(0.96, 0.93, 0.85);
    col = mix(col, paper, 0.3);

    // Ink color shift
    col.r += density * 0.03;
    col.g += density * 0.01;
    col.b -= density * 0.04;

    // Subtle paper grain
    float grain = noise(world * 0.5);
    col *= 0.95 + grain * 0.05;

    finalColor = vec4(col, base.a);
}