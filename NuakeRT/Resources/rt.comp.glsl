#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;

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

struct XYRect
{
    float x0;
    float x1;
    float y0;
    float y1;
    float k;
    vec4 albedo;
    int material;
    float fuzz;
    float refraction;
};

struct XZRect
{
    float x0;
    float x1;
    float z0;
    float z1;
    float k;
    vec4 albedo;
    int material;
    float fuzz;
    float refraction;
};

struct YZRect
{
    float y0;
    float y1;
    float z0;
    float z1;
    float k;
    vec4 albedo;
    int material;
    float fuzz;
    float refraction;
};


const int SPHERE_AMOUNT = 2;

layout(std430, binding = 2) readonly buffer myScene {
    Sphere data[];
};

layout(std430, binding = 3) readonly buffer cumulative {
    int frameID;
};

layout(std430, binding = 1) readonly buffer camera {
    vec4 positionAndFov;
    vec4 lookAt;
    int sphereAmount;
    float aperture;
    float focusDistance;
};

// random number generator
vec2 randState;
float rand2D()
{
    randState.x = fract(sin(dot(randState.xy, vec2(12.989, 78.2383))) * 43758.5453 + fract(lookAt.w) * 100);
    randState.y = fract(sin(dot(randState.xy, vec2(12.989, 78.233))) * 43758.5453 - fract(lookAt.w) * 100);;
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
    vec3 emitted;
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
    vec3 emitted;
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

bool XYRectHit(XYRect rect, Ray ray, float t_min, float t_max, inout HitRecord rec)
{
    float t = (rect.k - ray.origin.z) / ray.direction.z;

    if( t < t_min || t > t_max)
        return false;

    float x = ray.origin.x + t * ray.direction.x;
    float y = ray.origin.y + t * ray.direction.y;
    if (x < rect.x0 || x > rect.x1 || y < rect.y0 || y > rect.y1)
        return false;

    //rec.u = (x - rect.x0) / (rect.x1 - rect.x0);
    // v
    rec.albedo = rect.albedo.xyz;
    rec.material = rect.material;
    vec3 outwardNormal = vec3(0, 0, 1);
    rec.n = outwardNormal;
    rec.t = t;
    rec.p = RayAt(ray, t);
    rec.fuzz = rect.fuzz;
    rec.refraction = rect.refraction;
    return true;
}

bool XZRectHit(XZRect rect, Ray ray, float t_min, float t_max, inout HitRecord rec)
{
    float t = (rect.k - ray.origin.y) / ray.direction.y;

    if( t < t_min || t > t_max)
        return false;

    float x = ray.origin.x + t * ray.direction.x;
    float z = ray.origin.z + t * ray.direction.z;
    if (x < rect.x0 || x > rect.x1 || z < rect.z0 || z > rect.z1)
        return false;

    //rec.u = (x - rect.x0) / (rect.x1 - rect.x0);
    // v
    rec.albedo = rect.albedo.xyz;
    rec.material = rect.material;
    vec3 outwardNormal = vec3(0, 1, 0);
    rec.n = outwardNormal;
    rec.t = t;
    rec.p = RayAt(ray, t);
    rec.fuzz = rect.fuzz;
    rec.refraction = rect.refraction;
    return true;
}

bool YZRectHit(YZRect rect, Ray ray, float t_min, float t_max, inout HitRecord rec)
{
    float t = (rect.k - ray.origin.x) / ray.direction.x;

    if( t < t_min || t > t_max)
        return false;

    float y = ray.origin.y + t * ray.direction.y;
    float z = ray.origin.z + t * ray.direction.z;
    if (y < rect.y0 || y > rect.y1 || z < rect.z0 || z > rect.z1)
        return false;

    //rec.u = (x - rect.x0) / (rect.x1 - rect.x0);
    // v
    rec.albedo = rect.albedo.xyz;
    rec.material = rect.material;
    vec3 outwardNormal = vec3(0, 0, 1);
    rec.n = outwardNormal;
    rec.t = t;
    rec.p = RayAt(ray, t);
    rec.fuzz = rect.fuzz;
    rec.refraction = rect.refraction;
    return true;
}



float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

bool IntersectWorld(Ray ray, float t_min, float t_max, inout HitRecord rec)
{
    HitRecord record = {vec3(0, 0, 0), vec3(0, 0, 0), 0, -1, vec3(0, 0, 0), 0 ,0, vec3(0)};

    bool hitAnything = false;
    float closestSoFar = t_max;
    for(int i = 0; i < sphereAmount; i++)
    {
        if(hitSphere(data[i], ray, t_min, closestSoFar, record))
        {
            hitAnything = true;
            closestSoFar = record.t;
            rec = record;
        }
    }
    YZRect rectangle = YZRect(0, 5, 0, 5, 5, vec4(0, 1, 0, 1),        0, 0.9f, 2.5f);
    YZRect rectangle2 = YZRect(0, 5, 0, 5, 0, vec4(1, 0, 0, 1),       0, 0.1f, 2.5f);
    XZRect rectangle3 = XZRect(2, 3, 2, 3, 4.99, vec4(1, 1, 1, 1),    3, 0.1f, 2.5f);
    XZRect rectangle4 = XZRect(0, 5, 0, 5, 0, vec4(0.5, 0.5, 0.5, 1), 0, 0.1f, 2.5f);
    XZRect rectangle5 = XZRect(0, 5, 0, 5, 5, vec4(0.5, 0.5, 0.5, 1), 1, 0.1f, 2.5f);
    XYRect rectangle6 = XYRect(0, 5, 0, 5, 0, vec4(1, 0.0, 0.0, 1),   1, 0.1f, 2.5f);


    if(YZRectHit(rectangle, ray, t_min, closestSoFar, record))
    {
        hitAnything = true;
        closestSoFar = record.t;
        rec = record;
    }

    if(YZRectHit(rectangle2, ray, t_min, closestSoFar, record))
    {
        hitAnything = true;
        closestSoFar = record.t;
        rec = record;
    }

    if(XZRectHit(rectangle3, ray, t_min, closestSoFar, record))
    {
        hitAnything = true;
        closestSoFar = record.t;
        rec = record;
    }
    if(XZRectHit(rectangle4, ray, t_min, closestSoFar, record))
    {
        hitAnything = true;
        closestSoFar = record.t;
        rec = record;
    }
    if(XZRectHit(rectangle5, ray, t_min, closestSoFar, record))
    {
        hitAnything = true;
        closestSoFar = record.t;
        rec = record;
    }
    if(XYRectHit(rectangle6, ray, t_min, closestSoFar, record))
    {
        hitAnything = true;
        closestSoFar = record.t;
        rec = record;
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
        attenuation = isectInfo.albedo ;

        return true;
    } 
    else if(materialType == 1)
    {
        float fuzz = isectInfo.fuzz;

        vec3 reflected = reflect(normalize(wo.direction), isectInfo.n);

        wi.origin = isectInfo.p;
        wi.direction = reflected + fuzz * random_in_unit_sphere();
        attenuation = isectInfo.albedo  ;

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
    else if(materialType == 3)
    {
        attenuation = vec3(0, 0, 0);
        wi.emitted = vec3(1, 1, 1) ;
        return false;
    }
    
    return true;
}

vec3 skyColor(Ray ray)
{
    vec3 unit_direction = normalize(ray.direction);
    float t = 0.5 * (unit_direction.y + 1.0);

    return vec3(0); //(1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

vec4 Radiance(Ray ray)
{
    HitRecord record = {vec3(0, 0, 0), vec3(0, 0, 0), 0, -1, vec3(0, 0, 0), 0, 0, vec3(1)};
    vec3 color = vec3(1, 1, 1);
    vec3 emitted = vec3(0);
    for(int i = 0; i < 64; i++)
    {
        if(IntersectWorld(ray, 0.1, 99, record))
        {
            Ray wi = Ray(vec3(0), vec3(0), vec3(0, 0, 0));

            vec3 attenuation = vec3(0, 0, 0);
            

            bool wasScattered = MaterialBSDF(record, ray, wi, attenuation);
            
            vec3 emit = wi.emitted;
            emitted += i == 0 ? emit : color * emit;

            ray.origin = wi.origin;
            ray.direction = wi.direction;

            if(wasScattered)
            {
                color = i == 0 ? attenuation : color * attenuation;
                
            }
            else{
                return vec4(color , 1);
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

struct Camera
{
    vec3 origin;
    vec3 w;
    vec3 u;
    vec3 v;
    vec3 lowerLeftCorner;
    vec3 horizontal;
    vec3 vertical;
    float lensRadius;
};

Camera CreateCamera(vec3 lookFrom, vec3 lookAtCam, vec3 vUp, float fov, float aspect, float aperture, float focusDistance)
{
    float theta = fov * 3.14159265359/180;
    float halfHeight = tan(theta / 2.0);
    float viewportHeight = 2.0 * halfHeight;
    float viewportWidth = (aspect * 2.0) * halfHeight;

    vec3 origin = lookFrom;
    vec3 w = normalize(origin - lookAtCam);
    vec3 u = normalize(cross(vUp, w));
    vec3 v = cross(w, u);
    vec3 horizontal = focusDistance * viewportWidth * u;
    vec3 vertical = focusDistance * viewportHeight * v;
    vec3 lowerLeftCorner = origin - (horizontal / 2.0) - (vertical / 2.0) - focusDistance * w;
    float lensRadius = aperture / 2.0;

    return Camera(origin, w, u, v, lowerLeftCorner, horizontal, vertical, lensRadius);
}

Ray GetRay(Camera cam, vec2 uv)
{
    vec3 rd = cam.lensRadius * random_in_unit_disk();
    vec3 offset = cam.u * rd.x + cam.v * rd.y;
    return Ray(cam.origin + offset ,
        normalize(cam.lowerLeftCorner + uv.x * cam.horizontal + uv.y * cam.vertical - cam.origin - offset)
        , vec3(0));
}

void main()
{
    // Ambient color
    vec4 pixel = vec4(0.075, 0.133, 0.173, 1.0);

    // coords of pixel
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	
    vec4 previous = imageLoad(screen, pixel_coords);

    // Size of image
	vec2 dims = imageSize(screen);
    float aspectRatio = dims.x / dims.y;
    randState = pixel_coords / dims;

    Camera cam = CreateCamera(positionAndFov.xyz, lookAt.xyz, vec3(0, 1, 0), positionAndFov.w, aspectRatio, 
    aperture, focusDistance);

    // Get UV
    vec2 uv = pixel_coords / dims;
    pixel.x = uv.x;
    pixel.y = uv.y;

    const int samplePerPixel = 4;
    vec4 finalColor = vec4(0, 0, 0, 1);
    for(int i = 0; i < samplePerPixel; i++)
    {
        float px = pixel_coords.x;
        float py = pixel_coords.y;

        float randf  = rand(vec2(px  + i + lookAt.w , py  + i + lookAt.w))  ;
        float randf2 = rand(vec2(px  - i + lookAt.w, py - i + lookAt.w))  ;

        float u = (px + randf  ) / (dims.x );
        float v = (py + randf2 ) / (dims.y );

        vec2 uv = vec2(u, v);

        Ray ray = GetRay(cam, vec2(uv));
        finalColor += Radiance(ray);
    }

    vec3 color = finalColor.rgb * (1.0 / (samplePerPixel));
    if (frameID > 1) {
        vec3 inColor = previous.xyz;
        inColor *= frameID;
        inColor += color;
        inColor /= (frameID + 1);
        color = clamp(inColor, 0.0, 1.0);
    }
    else
    {
        color = finalColor.rgb;
    }

	imageStore(screen, pixel_coords, vec4(color, 1));
}
