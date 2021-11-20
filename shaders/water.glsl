//afl_ext 2018

#define DRAG_MULT 0.048
#define ITERATIONS_RAYMARCH 10
#define ITERATIONS_NORMAL 30
#define WATER_DEPTH 0.5

// returns vec2 with wave height in X and its derivative in Y
vec2 wavedx(vec2 position, vec2 direction, float speed, float frequency, float timeshift) {
    float x = dot(direction, position) * frequency + timeshift * speed;
    float wave = exp(sin(x) - 1.0);
    float dx = wave * cos(x);
    return vec2(wave, -dx);
}

float getwaves(vec2 position, int iterations){
    float iter = 0.0;
    float phase = 15.0;
    float speed = 4.0;
    float weight = 1.0;
    float w = 0.0;
    float ws = 0.0;
    for(int i=0;i<iterations;i++){
        vec2 p = vec2(sin(iter), cos(iter));
        vec2 res = wavedx(position, p, speed, phase, uni_time);
        position += normalize(p) * res.y * weight * DRAG_MULT;
        w += res.x * weight;
        iter += 12.0;
        ws += weight;
        weight = mix(weight, 0.0, 0.2);
        phase *= 1.18;
        speed *= 1.07;
    }
    return w / ws;
}

float raymarchwater(vec3 camera, vec3 start, vec3 end, float depth){
    vec3 pos = start;
    float h = 0.0;
    float hupper = depth;
    float hlower = 0.0;
    vec2 zer = vec2(0.0);
    vec3 dir = normalize(end - start);
    for(int i=0;i<318;i++){
        h = getwaves(pos.xz * 0.1, ITERATIONS_RAYMARCH) * depth - depth;
        if(h + 0.01 > pos.y) {
            return distance(pos, camera);
        }
        pos += dir * (pos.y - h);
    }
    return -1.0;
}

float H = 0.0;
vec3 normal(vec2 pos, float e, float depth){
    vec2 ex = vec2(e, 0);
    H = getwaves(pos.xy * 0.1, ITERATIONS_NORMAL) * depth;
    vec3 a = vec3(pos.x, H, pos.y);
    return normalize(cross(normalize(a-vec3(pos.x - e, getwaves(pos.xy * 0.1 - ex.xy * 0.1, ITERATIONS_NORMAL) * depth, pos.y)),
    normalize(a-vec3(pos.x, getwaves(pos.xy * 0.1 + ex.yx * 0.1, ITERATIONS_NORMAL) * depth, pos.y + e))));
}
mat3 rotmat(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat3(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
    oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
    oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

float intersectPlane(vec3 origin, vec3 direction, vec3 point, vec3 normal)
{
    return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0);
}

vec3 getatm(vec3 ray, float roughness){
    vec3 sharp = mix(vec3( 0.0293, 0.0698, 0.1717) * 10.0, vec3(3.0), pow(1.0 - ray.y, 8.0));
    vec3 rough = vec3(vec3( 0.0293, 0.0698, 0.1717) + vec3(1.0));
    return mix(sharp, rough, roughness);
}

float sun(vec3 ray){
    return pow(max(0.0, dot(ray, normalize(vec3(1.0, 1.0, 0.0)))), 668.0) * 110.0;
}

vec3 getColor(vec3 origin, vec3 ray, inout float depth) {

    vec3 wfloor = vec3(0.0, -WATER_DEPTH, 0.0);
    vec3 wceil = vec3(0.0, 0.0, 0.0);
    vec3 orig = origin;
    float hihit = intersectPlane(orig, ray, wceil, vec3(0.0, 1.0, 0.0));
    float lohit = intersectPlane(orig, ray, wfloor, vec3(0.0, 1.0, 0.0));
    vec3 hipos = orig + ray * hihit;
    vec3 lopos = orig + ray * lohit;
    float dist = raymarchwater(orig, hipos, lopos, WATER_DEPTH);
    depth = dist;
    vec3 pos = orig + ray * dist;

    vec3 N = normal(pos.xz, 0.01, WATER_DEPTH);
    vec2 velocity = N.xz * (1.0 - N.y);
    vec3 R = reflect(ray, N);
    float roughness = 1.0 - 1.0 / (dist * 0.01 + 1.0);
    N = normalize(mix(N, vec3(0.0, 1.0, 0.0), roughness));
    R = normalize(mix(R, N, roughness));
    R.y = abs(R.y);
    float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));

    vec3 C = fresnel * (getatm(R, roughness) + sun(R)) * 2.0;

    return C;
}

vec3 gammacorrect(vec3 c){
    return pow(c, vec3(1.0 / 2.4));
}

vec3 GetWater(vec3 origin, vec3 ray, inout float depth){
    vec3 C = getColor(origin, ray, depth) * 0.3;
    return gammacorrect(C);
}