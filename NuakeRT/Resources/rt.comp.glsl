#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba8, binding = 0) uniform image2D screen;

struct Sphere
{
    vec4 center;
    int material;
    float padding;
	float padding2;
	float padding3;
    vec4 albedo;
    float refraction;
};


const int SPHERE_AMOUNT = 2;

layout(std430, binding = 2) readonly buffer myScene {
    Sphere data[];
};

layout(std430, binding = 1) readonly buffer camera {
    vec4 positionAndFov;
    vec3 lookAt;
};

// random number generator
vec2 randState;
float rand2D()
{
    randState.x = fract(sin(dot(randState.xy, vec2(12.989, 78.233))) * 43758.5453);
    randState.y = fract(sin(dot(randState.xy, vec2(12.989, 78.233))) * 43758.5453);;
    return randState.x;
}
#define PI 			3.1415926535
// random point on unit disk (for depth of field camera)
vec3 random_in_unit_disk()
{
    float spx = 2.0 * rand2D() - 1.0;
    float spy = 2.0 * rand2D() - 1.0;

    float r, phi;


    if(spx > -spy)
    {
        if(spx > spy)
        {
            r = spx;
            phi = spy / spx;
        }
        else
        {
            r = spy;
            phi = 2.0 - spx / spy;
        }
    }
    else
    {
        if(spx < spy)
        {
            r = -spx;
            phi = 4.0f + spy / spx;
        }
        else
        {
            r = -spy;

            if(spy != 0.0)
                phi = 6.0 - spx / spy;
            else
                phi = 0.0;
        }
    }

    phi *= PI / 4.0;


    return vec3(r * cos(phi), r * sin(phi), 0.0f);
}

// random direction in unit sphere (for lambert brdf)
vec3 random_in_unit_sphere()
{
    float phi = 2.0 * PI * rand2D();
    float cosTheta = 2.0 * rand2D() - 1.0;
    float u = rand2D();

    float theta = acos(cosTheta);
    float r = pow(u, 1.0 / 3.0);

    float x = r * sin(theta) * cos(phi);
    float y = r * sin(theta) * sin(phi);
    float z = r * cos(theta);

    return vec3(x, y, z);
}

struct Ray 
{
    vec3 origin;
    vec3 direction;
};

struct HitRecord
{
    vec3 p;
    vec3 n;
    float t;
    int material;
    vec3 albedo;
    float fuzz;
    float refraction;
};



vec3 RayAt(Ray ray, float t)
{
    return ray.origin + t * ray.direction;
}

bool hitSphere(Sphere sphere, Ray ray, float t_min, float t_max, inout HitRecord rec)
{
    vec3 oc = ray.origin - sphere.center.xyz;
    float a = dot(ray.direction, ray.direction);
    float b = dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.center.w * sphere.center.w;

    float discriminant = b * b - a * c;
    if (discriminant > 0.0f)
    {
        float temp = (-b - sqrt(discriminant)) / a;

        if (temp < t_max && temp > t_min)
        {
            rec.t                = temp;
            rec.p                = ray.origin + rec.t * ray.direction;
            rec.n           = (rec.p - sphere.center.xyz) / sphere.center.w;
            rec.material     = sphere.material;
            rec.albedo           = sphere.albedo.xyz;
            rec.fuzz             = sphere.albedo.a;
            rec.refraction  = sphere.refraction;

            return true;
        }


        temp = (-b + sqrt(discriminant)) / a;

        if (temp < t_max && temp > t_min)
        {
            rec.t                = temp;
            rec.p                = ray.origin + rec.t * ray.direction;
            rec.n           = (rec.p - sphere.center.xyz) / sphere.center.w;
            rec.material     = sphere.material;
            rec.albedo           = sphere.albedo.xyz;
            rec.fuzz             = sphere.albedo.w;
            rec.refraction  = sphere.refraction;

            return true;
        }
    }

    return false;
}


float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

bool IntersectWorld(Ray ray, float t_min, float t_max, inout HitRecord rec)
{
    HitRecord record = {vec3(0, 0, 0), vec3(0, 0, 0), 0, -1, vec3(0, 0, 0), 0 ,0};

    bool hitAnything = false;
    float closestSoFar = t_max;

    for(int i = 0; i < SPHERE_AMOUNT; i++)
    {
        if(hitSphere(data[i], ray, t_min, closestSoFar, record))
        {
            hitAnything = true;
            closestSoFar = record.t;
            rec = record;
        }
    }
    return hitAnything;
}

bool refractVec(vec3 v, vec3 n, float ni_over_nt, out vec3 refracted)
{
    vec3 uv = normalize(v);

    float dt = dot(uv, n);

    float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1.0f - dt * dt);

    if (discriminant > 0.0f)
    {
        refracted = ni_over_nt*(uv - n * dt) - n * sqrt(discriminant);

        return true;
    }
    else
        return false;
}

// Schlick's approximation for approximating the contribution of the Fresnel factor
// in the specular reflection of light from a non-conducting surface between two media
//
// Theta is the angle between the direction from which the incident light is coming and
// the normal of the interface between the two media
float schlick(float cos_theta, float n2)
{
    const float n1 = 1.0f;  // refraction index for air

    float r0s = (n1 - n2) / (n1 + n2);
    float r0 = r0s * r0s;

    return r0 + (1.0f - r0) * pow((1.0f - cos_theta), 5.0f);
}

bool MaterialBSDF(HitRecord isectInfo, Ray wo, out Ray wi, inout vec3 attenuation)
{
    int materialType = isectInfo.material;

    if(materialType == 0)
    {
        vec3 target = isectInfo.p + isectInfo.n + random_in_unit_sphere();

        wi.origin = isectInfo.p;
        wi.direction = target - isectInfo.p;

        attenuation = isectInfo.albedo;

        return true;
    } 
    else if(materialType == 1)
    {
        float fuzz = isectInfo.fuzz;

        vec3 reflected = reflect(normalize(wo.direction), isectInfo.n);

        wi.origin = isectInfo.p;
        wi.direction = reflected + fuzz * random_in_unit_sphere();

        attenuation = isectInfo.albedo;

        return (dot(wi.direction, isectInfo.n) > 0.0f);
    }
    else 
    if(materialType == 2)
    {
        vec3 outward_normal;
        vec3 reflected = reflect(wo.direction, isectInfo.n);

        float ni_over_nt;

        attenuation = vec3(1.0f, 1.0f, 1.0f);
        vec3 refracted;
        float reflect_prob;
        float cosine;

        float rafractionIndex = isectInfo.refraction;

        if (dot(wo.direction, isectInfo.n) > 0.0f)
        {
            outward_normal = -isectInfo.n;
            ni_over_nt = rafractionIndex;
           
            cosine = dot(wo.direction, isectInfo.n) / length(wo.direction);
            cosine = sqrt(1.0f - rafractionIndex * rafractionIndex * (1.0f - cosine * cosine));
        }
        else
        {
            outward_normal = isectInfo.n;
            ni_over_nt = 1.0f / rafractionIndex;
            cosine = -dot(wo.direction, isectInfo.n) / length(wo.direction);
        }
        if (refractVec(wo.direction, outward_normal, ni_over_nt, refracted))
            reflect_prob = schlick(cosine, rafractionIndex);
        else
            reflect_prob = 1.0f;
        if (rand2D() < reflect_prob)
        {
            wi.origin = isectInfo.p;
            wi.direction = reflected;
        }
        else
        {
            wi.origin = isectInfo.p;
            wi.direction = refracted;
        }

        return true;
    }
    
    return false;
}

vec3 skyColor(Ray ray)
{
    vec3 unit_direction = normalize(ray.direction);
    float t = 0.5 * (unit_direction.y + 1.0);

    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

vec4 Radiance(Ray ray)
{
    HitRecord record = {vec3(0, 0, 0), vec3(0, 0, 0), 0, -1, vec3(0, 0, 0), 0, 0};
    vec3 color = vec3(1, 1, 1);

    for(int i = 0; i < 8; i++)
    {
        if(IntersectWorld(ray, 0.1, 99, record))
        {
            Ray wi = Ray(vec3(0, 0, 0), vec3(0, 0, 0));

            vec3 attenuation = vec3(0, 0, 0);

            bool wasScattered = MaterialBSDF(record, ray, wi, attenuation);

            ray.origin = wi.origin;
            ray.direction = wi.direction;

            if(wasScattered)
                color *= attenuation;
            else{
                color *= vec3(0, 0, 0);
                break;
            }
        }
        else
        {
            color *= skyColor(ray);
            break;
        }
    }

    return vec4(color, 1);
}


Ray create_camera_ray(vec2 uv, vec3 camPos, vec3 lookAt, float zoom) {
    vec3 f = normalize(lookAt - camPos);
    vec3 r = normalize(cross(vec3(0, 1, 0), f));
    vec3 u = normalize(cross(f, r));

    vec3 c = camPos + f * zoom;
    vec3 i = c + uv.x * -r + uv.y * u;
    vec3 dir = i - camPos;
    return Ray(camPos, dir);
}

void main()
{
    // Ambient color
    vec4 pixel = vec4(0.075, 0.133, 0.173, 1.0);

    // coords of pixel
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	
    // Size of image
	vec2 dims = imageSize(screen);
    float aspectRatio = dims.x / dims.y;
    randState = pixel_coords / dims;

    float theta = positionAndFov.w * 0.017453;
    float h = tan(theta/2.f);

    float viewportHeight = 2.0f * h;
    float viewportWidth = aspectRatio * viewportHeight;


    vec3 camPos = positionAndFov.xyz;
    vec3 forward = normalize(camPos - lookAt);
    vec3 right = normalize(cross(forward, vec3(0, 1, 0)));
    vec3 up = cross(forward, right);

    vec3 horizontal = vec3(viewportWidth, 0, 0) * right;
    vec3 vertical = vec3(0, viewportHeight, 0) * up;
    
    vec3 lowerLeftCorner = camPos - horizontal/2 - vertical/2 - forward;

    // Get UV
    vec2 uv = pixel_coords / dims;
    pixel.x = uv.x;
    pixel.y = uv.y;

    const int samplePerPixel = 200;
    vec4 finalColor = vec4(0, 0, 0, 1);
    for(int i = 0; i < samplePerPixel; i++)
    {
        float px = pixel_coords.x;
        float py = pixel_coords.y;

        float randf  = rand(vec2(px + i, py + i)) ;
        float randf2 = rand(vec2(px - i, py - i)) ;

        float u = (px ) / (dims.x );
        float v = (py  ) / (dims.y );

        //float u = (px ) / (dims.x );
        //float v = (py  ) / (dims.y );

        
        Ray ray = Ray(
            camPos + (u * horizontal) + (v * vertical), 
            lowerLeftCorner + (u * horizontal) + (v * vertical) - camPos
        );

        finalColor += Radiance(ray);
    }

	imageStore(screen, pixel_coords, finalColor * (1.0 / (samplePerPixel)));
}
