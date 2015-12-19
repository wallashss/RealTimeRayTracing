__constant float MINIMUM_INTERSECT_DISTANCE = 1e-7f;


static void swap(float * a, float * b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

static float3 getNormalFromSphere(float8 sphere, float3 position)
{
    return normalize((float3)(position.x -sphere.lo.x, position.y - sphere.lo.y, position.z - sphere.lo.z));
}

static float3 getNormalFromPlane(float16 plane)
{
    float3 position = plane.lo.lo.xyz;
    float3 pointA   = plane.lo.hi.xyz;
    float3 pointB   = plane.hi.lo.xyz;
    return normalize(cross(pointA - position, pointB -position));
}


static bool hasInterceptedPlane(float16 plane, float3 ray, float3 origin, float3 * touchPoint)
{
    float3 n = getNormalFromPlane(plane);
    float denom = dot(n, ray);
    if (denom > 1e-6f)
    {
        float3 position = plane.lo.lo.xyz;
        float3 p0l0 = position - origin;
        float d = dot(p0l0, n) / denom;

        *touchPoint = origin + (ray) * d;

        return (d >= 0);
    }
    return false;
}

static bool hasInterceptedSphere(float8 sphere, float3 ray, float3 origin, float3 * touchPoint)
{
    float rayDirX = ray.x;
    float rayDirY = ray.y;
    float rayDirZ = ray.z;

    float rayOriginX = origin.x;
    float rayOriginY = origin.y;
    float rayOriginZ = origin.z;

    float sphereCenterX = sphere.lo.x;
    float sphereCenterY = sphere.lo.y;
    float sphereCenterZ = sphere.lo.z;
    float radius =        sphere.lo.w;

    float A = rayDirX * rayDirX + rayDirY * rayDirY + rayDirZ * rayDirZ;
    float B = 2 * (rayDirX * (rayOriginX - sphereCenterX) + rayDirY * (rayOriginY - sphereCenterY) + rayDirZ * (rayOriginZ - sphereCenterZ));
    float C = (rayOriginX - sphereCenterX)*(rayOriginX - sphereCenterX) + (rayOriginY - sphereCenterY)*(rayOriginY - sphereCenterY) + (rayOriginZ - sphereCenterZ)*(rayOriginZ - sphereCenterZ) - radius*radius;

    float delta = B * B - 4 * A * C;
    if ( delta < 0 )
    {
        return false;
    }

    float root = sqrt( delta );
    float t1 = (-B + root) / (2 * A);
    float t2 = (-B - root) / (2 * A);

    if ( t1 < 0 && t2 < 0 )
    {
        return false;
    }

    if ( t2 < t1 )   // t2 is the biggest
    {
        swap( &t1, &t2 );
    }

    float minT = t1;
    if ( t1 < 0 )
    {
        minT = t2;
    }
    else if ( t1 < MINIMUM_INTERSECT_DISTANCE )
    {
        minT = t2;
    }


    if ( minT < MINIMUM_INTERSECT_DISTANCE )
    {
        return false;
    }

    *touchPoint = origin + (ray*(float)minT);
    return true;
}

static float3 reflect(float3 I, float3 N)
{
    return I - 2.0f * dot(N, I) * N;
}

static float4 phong(float3 viewDir, float3 position, float3 normal, float4 diffuseColor, float3 light, float4 lightColor)
{
    float4 outColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float3 lightDir = normalize( light - position);
    float lightDistance = 1.0f;//distance(light, position);

    float quadraticAttenuation = 1.0f;
    float linearAttenuation    = 1.0f;
    float constantAttenuation  = 1.0f;
    float attenuation = 1.0f / (constantAttenuation
                             + linearAttenuation * lightDistance
                             + quadraticAttenuation * lightDistance * lightDistance);


     float4 ambientColor = (float4)(0.1f, 0.1f, 0.1f, 0.1f) * lightColor * diffuseColor;
     float diff = max(0.0f, dot(normal,lightDir));

     float3 reflection = normalize(reflect(-lightDir,normal));

     float specular = max(0.0f, dot(reflection, normalize(viewDir)));

     specular = pow(specular,50);

     outColor += ambientColor;
     outColor += diffuseColor*diff;
     outColor += lightColor*specular*attenuation; // * drawableSpecular
    return outColor;
}

static float4 traceRay(float3 eye,
                       float3 ray,
                       __global float * spheres,
                       int numSpheres,
                       __global float * planes,
                       int numPlanes,
                       __global float * lights,
                       int numLights)
{
    float4 outColor = (float4)(0.0f,0.0f,0.0f,1.0f);
    bool hasHit = false;
    float3 closestPoint;
    float3 normal;
    float minDist = 10e7f;
    float3 touchPoint;
    float8 sphere;
    float16 plane;
    float4 objectColor;

    for(int i = 0 ; i < numSpheres ; i++)
    {
        sphere = vload8(i, spheres);

        if(hasInterceptedSphere(sphere, ray, eye, &touchPoint))
        {
            float dist = fast_distance(touchPoint, eye);
            if(dist < minDist)
            {
                minDist = dist;
                closestPoint = touchPoint;

                objectColor = sphere.hi;
                normal = getNormalFromSphere(sphere, touchPoint);
                hasHit = true;
            }
        }
    }

    for(int i = 0 ; i < numPlanes ; i++)
    {
        plane = vload16(i, planes);
        if(hasInterceptedPlane(plane, ray, eye, &touchPoint))
        {
            float dist = fast_distance(touchPoint, eye);
            if(dist < minDist)
            {
                minDist = dist;
                closestPoint = touchPoint;

                objectColor = plane.hi.hi;
                normal = getNormalFromPlane(plane);
                hasHit = true;
            }
        }
    }

    if(hasHit)
    {
        for(int i = 0 ; i < numLights; i++)
        {
            float8 light = vload8(i, lights);
            float4 lightColor = light.hi;
            float3 viewDir = eye - closestPoint;

            float4 phongColor = phong(viewDir, closestPoint, normal, objectColor, light.lo.xyz, lightColor);
            outColor += phongColor;
        }
    }
    return outColor;
}


__kernel void rayTracing(__write_only image2d_t texture,
                         __global float * spheres,
                         int numSpheres,
                         __global float * planes,
                         int numPlanes,
                         __global float * lights,
                         int numLights,
                         float eyeX, float eyeY, float eyeZ,
                         int width, int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    float3 ray = (float3)(x-width/2,height/2-y,1000);
    ray = normalize(ray);

    float3 eye = (float3)(eyeX, eyeY, eyeZ);

    float4 color = traceRay(eye, ray, spheres, numSpheres, planes, numPlanes, lights, numLights);

    write_imagef(texture, (int2)(x, height-y), color);
}
