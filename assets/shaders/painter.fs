#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;

// ---------------- NOISE ----------------
float hash(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    return fract(p.x * p.y);
}

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

// ---------------- STRONGER FBM ----------------
float fbm(vec2 p)
{
    float v = 0.0;
    float a = 0.5;

    for (int i = 0; i < 3; i++)
    {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }

    return v;
}

// ---------------- BRUSH LINES ----------------
float brush(vec2 uv)
{
    // directional stroke pattern
    float angle = 0.6; // fixed brush direction

    vec2 dir = vec2(cos(angle), sin(angle));
    float stroke = dot(uv, dir) * 10.0;

    return sin(stroke + fbm(uv * 4.0) * 6.0 + time * 0.3);
}

// ---------------- MAIN ----------------
void main()
{
    vec4 base = texture(texture0, fragTexCoord) * fragColor;

    vec2 uv = fragTexCoord;

    // ---------------- INK FIELD ----------------
    float n = fbm(uv * 6.0);

    // MAKE IT VISIBLE (important change)
    float ink = smoothstep(0.3, 0.7, n);

    // ---------------- BRUSH STRUCTURE ----------------
    float stroke = brush(uv);

    stroke = smoothstep(-0.2, 0.2, stroke);

    // combine ink + stroke
    float paint = mix(ink, stroke, 0.55);

    // ---------------- APPLY EFFECT ----------------
    vec3 col = base.rgb;

    // strong enough to SEE
    col *= (1.0 - paint * 0.35);

    // ink tint (slightly warm/aged)
    col = mix(col, col * vec3(0.92, 0.95, 1.02), paint);

    // paper wash (visible but not overpowering)
    col += paint * 0.08;

    // subtle grain (helps “paint feel” a lot)
    float grain = noise(uv * 200.0);
    col += (grain - 0.5) * 0.05;

    finalColor = vec4(col, base.a);
}