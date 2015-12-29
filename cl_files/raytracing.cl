__constant float MINIMUM_INTERSECT_DISTANCE = 1e-7f;


static void swap(float * a, float * b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

static float3 reflect(float3 I, float3 N)
{
    return I - 2.0f * dot(N, I) * N;
}

static float3 refract(float3 I, float3 N, float eta)
{
    float k = 1.0f - eta * eta * (1.0f - dot(N, I) * dot(N, I));
    if (k < 0.0f)
    {
        return float3(0,0,0);
    }
    else
    {
        return eta * I - (eta * dot(N, I) + sqrt(k)) * N;
    }
}

static float3 getNormalFromSphere(float8 sphere, float3 position)
{
    return normalize((float3)(position.x -sphere.lo.x, position.y - sphere.lo.y, position.z - sphere.lo.z));
}

static float4 getColorFromPlane (float16 plane, float3 point)
{
    float tileSize = plane.lo.lo.w;
    int xt =(int)round(point.x /tileSize);
    int yt =(int)round(point.z /tileSize);

    bool evenX = xt % 2 == 0;
    bool evenY = yt % 2 == 0;

    if(evenX && evenY)
    {
        return plane.hi.lo;
    }
    else if(!evenX && evenY)
    {
        return plane.hi.hi;
    }
    else if(evenX && !evenY)
    {
        return plane.hi.hi;

    }
    else if(!evenX && !evenY)
    {
        return plane.hi.lo;
    }
    else
    {
        return plane.hi.hi;
    }
}

static float3 getNormalFromPlane(float8 plane)
{
    return plane.hi.xyz;
}

static bool hasInterceptedPlane(float8 plane, float3 ray, float3 origin, float3 * touchPoint)
{
    float3 n = plane.hi.xyz;
    float denom = dot(n, ray);
    if (denom > 1e-6f)
    {
        float3 position = plane.lo.xyz;
        float3 p0l0 = position - origin;
        float d = dot(p0l0, n) / denom;

        *touchPoint = origin + (ray) * d;

        return (d >= 0);
    }
    return false;
}


#if 0
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

#else

static bool solveQuadratic(const float a, const float b, const float c, float *x0, float *x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0)
    {
        return false;
    }
    else if (discr == 0)
    {
        *x0 = *x1 = - 0.5f * b / a;
    }
    else
    {
        float q = (b > 0) ?
            -0.5f * (b + sqrt(discr)) :
            -0.5f * (b - sqrt(discr));
        *x0 = q / a;
        *x1 = c / q;
    }
    if (*x0 > *x1)
    {
        swap(x0, x1);
    }

    return true;
}

static bool hasInterceptedSphere(float8 sphere, float3 dir, float3 orig, float3 * touchPoint)
{
    float t0, t1, t;
    float3 center = sphere.lo.xyz;
    float radius2 = sphere.lo.w*sphere.lo.w;
    float3 L = orig - center;
    float a = dot(dir, dir);
    float b = 2 * dot(L, dir);
    float c = dot(L, L) - radius2;
    if (!solveQuadratic(a, b, c, &t0, &t1))
    {
        return false;
    }

    if (t0 > t1)
    {
        swap(&t0, &t1);
    }

    if (t0 < 0)
    {
        t0 = t1; // if t0 is negative, let's use t1 instead
        if (t0 < 0)
        {
            return false; // both t0 and t1 are negative
        }
    }

    t = t0;
    *touchPoint = orig + (dir*(float)t);
    return true;
}

#endif


static float schlickApproximation(float n1, float n2, float3 incident, float3 normal)
{
    float r0 = (n1-n2)/(n1+n2);
    r0 = r0 * r0;
    float cosI = -dot(normal,incident);
    float cosX = cosI;
    if(n1 > n2)
    {
        const float n = n1/n2;
        const float sinT2 = n*n*(1.0f - cosI*cosI);
        if(sinT2 >1.0f)
        {
            // Total Internal Reflection!
            return 1.0f;
        }
        cosX = sqrt(1.0f-sinT2);
    }
    const float x = 1.0f-cosX;
    return r0+(1.0f-r0)*x*x*x*x*x;
}

static float4 phong(float3 viewDir, float3 position, float3 normal, float4 diffuseColor, float3 light, float4 lightColor)
{
    float4 outColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float3 lightDir = normalize( light - position);
    float lightDistance = fast_distance(light, position);

    float k = 0.005f;
    float attenuation = 1.0f / (1.0f + k * lightDistance * lightDistance);

    float4 ambientColor = (float4)(0.1f, 0.1f, 0.1f, 0.1f) * lightColor * diffuseColor;
    float diff = max(0.0f, dot(normal,lightDir));

    float3 reflection = normalize(reflect(-lightDir,normal));

    float specular = max(0.0f, dot(reflection, normalize(viewDir)));

    specular = pow(specular,50);

    outColor += ambientColor;
    outColor += (diffuseColor*diff + lightColor*specular) * attenuation;
    return outColor;
}

static void loadSpheresToLocal(__global const float *spheres,
                               __local  float *temp,
                                        int localIdx,
                                        int numSpheres)
{
    if(localIdx < numSpheres)
    {
        float8 tempSphere = vload8(localIdx, spheres);
        vstore8(tempSphere, localIdx, temp);
    }

    barrier(CLK_LOCAL_MEM_FENCE); // wait for spheres loading
}

static void loadPlanesToLocal(__global const float *planes,
                               __local  float *temp,
                                        int localIdx,
                                        int numPlanes)
{
    if(localIdx < numPlanes)
    {
        float16 tempPlane = vload16(localIdx, planes);
        vstore16(tempPlane, localIdx, temp);
    }

    barrier(CLK_LOCAL_MEM_FENCE); // wait for spheres loading
}

static float4 traceRay(float3 eye,
                       float3 ray,
                       __global const float * spheres,
                       int numSpheres,
                       __global const float * planes,
                       int numPlanes,
                       __global const float * lights,
                       int numLights,
                       __local float * temp,
                       __local float * temp2,
                       float3 * reflectedRay,
                       float3 * refractedRay,
                       float * R,
                       float * T,
                       float3 * touchPos,
                       int    * lastSphereIdx,
                       int    * lastPlaneIdx)
{
    float4 outColor = (float4)(0.0f,0.0f,0.0f,1.0f);
    bool hasHit = false;
    float3 closestPoint;
    float3 normal;
    float minDist = 10e7f;
    float3 touchPoint;
    float4 objectColor;

    // Local index to load local memory
    int x = get_local_id(0);
    int y = get_local_id(1);
    int localIdx = x * get_local_size(1) + y;


    // Load planes to local memory
    loadPlanesToLocal(planes, temp, localIdx, numPlanes);

    // Check for planes intersection
    int currentPlaneIdx = -1;
    for(int i = 0 ; i < numPlanes ; i++)
    {
        if(isequal(length(ray) , 0.0f))
        {
            break;
        }
        if(*lastPlaneIdx == i)
        {
            continue;
        }

        float16 plane = vload16(i, temp);
        if(hasInterceptedPlane(plane.lo, ray, eye, &touchPoint))
        {
            float dist = fast_distance(touchPoint, eye);
            if(dist < minDist)
            {
                minDist = dist;
                closestPoint = touchPoint;

                float tileSize = plane.lo.lo.w;
                if(isnotequal(tileSize, 0.0f))
                {
                    objectColor = getColorFromPlane(plane, closestPoint);
                }
                else
                {
                    objectColor = plane.hi.lo;
                }

                normal = getNormalFromPlane(plane.lo);
                hasHit = true;
                currentPlaneIdx = i;
            }
        }
    }
    if(!hasHit)
    {
        *lastPlaneIdx = -1;
    }
    else
    {
        *lastPlaneIdx = currentPlaneIdx;
    }
    barrier(CLK_LOCAL_MEM_FENCE); // we may be accessing planes in local memory

    loadSpheresToLocal(spheres, temp, localIdx, numSpheres);

    // Check for spheres intersection
    bool hasHitSphere = false;
    int currentSphereIdx = -1;
    for(int i = 0 ; i < numSpheres ; i++)
    {
        if(isequal(length(ray) , 0.0f) )
        {
            break;
        }
        if((*lastSphereIdx) == i)
        {
            continue;
        }
        float8 sphere = vload8(i, temp);

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
                currentSphereIdx = i;
                hasHitSphere = true;
            }
        }
    }
    if(!hasHitSphere)
    {
        *lastSphereIdx = -1;
    }
    else
    {
        *lastSphereIdx = currentSphereIdx;
        *lastPlaneIdx = -1;
    }

    // Load lights to local memory
    if(localIdx < numLights)
    {
        float8 tempLight = vload8(localIdx, lights);
        vstore8(tempLight, localIdx, temp2);
    }
    barrier(CLK_LOCAL_MEM_FENCE); // wait for lights loading

    // Calculate color
    if(hasHit)
    {
// TODO Reflection and refraction
//        if(isgreater(objectColor.w, 0.0f))
//        {
//            *reflectedRay = reflect(ray, normal);
//            *touchPos = closestPoint;
//        }
//        else if(isless(objectColor.w, 0.0f))
//        {

//            float n1 = 1.0f; // Air...
//            float n2 = -objectColor.w;

//            *R = schlickApproximation(n1, n2, ray, normal);
//            *T = 1.0f-*R;

//            *reflectedRay = reflect(ray, normal);
//            *refractedRay = refract(ray, normal, n1/n2);

////            float3 oppositeSideOrigin = touchPoint + 10000.0f * transmissedRay;
////            float3 oppositeTouchPoint;

////            float3 transmissionColor = traceRay(oppositeTouchPoint, normalize(transmissedRay), n-1, closestDrawable);

////            float3 reflexionColor = traceRay(closestPoint,glm::normalize(reflectedRay), n-1,closestDrawable);

////            return  ((R*reflexionColor)+(T*transmissionColor))*newColor;

//        }
//        else
//        {
//            *reflectedRay = float3(0, 0, 0);
//            *touchPos = closestPoint;
//        }

        for(int i = 0 ; i < numLights; i++)
        {
            float8 light = vload8(i, temp2);
            float3 lightPos = light.lo.xyz;
            float4 lightColor = light.hi;

            // Check if point is occluded (shadow) only for spheres
            float3 lightDir = normalize(closestPoint - lightPos);
            bool isInShadow = false;

            for(int j = 0 ; j < numSpheres ; j++)
            {
                float8 sphere = vload8(j, temp);
                float3 occludedPoint;

                if(j != *lastSphereIdx)
                {
                    if(hasInterceptedSphere(sphere, lightDir, lightPos, &occludedPoint))
                    {
                        if(fast_distance(occludedPoint, lightPos) < fast_distance(lightPos, closestPoint))
                        {
                            isInShadow = true;
                            break;
                        }
                    }
                }
            }

            // Calculate phong color if point is not in shadow
            if(!isInShadow)
            {
                float3 viewDir = eye - closestPoint;
                float4 phongColor = phong(viewDir, closestPoint, normal, objectColor, light.lo.xyz, lightColor);
                outColor += phongColor;
            }
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    return outColor;
}


__kernel void rayTracing(__write_only image2d_t texture,
                         __global const float * spheres,
                         const int numSpheres,
                         __global const float * planes,
                         const int numPlanes,
                         __global const float * lights,
                         const int numLights,
                         __local float * temp,
                         __local float * temp2,
                         const float eyeX, const float eyeY, const float eyeZ,
                         const int width, const int height)
{
    // Ray Setup
    int x = get_global_id(0);
    int y = get_global_id(1);

    float3 eye    = (float3)(eyeX, eyeY, eyeZ);
    float3 center = (float3)(0, 0.0f, 0);
    float3 up     = (float3)(0, 1.0f, 0);

    float3 dir   = normalize(center - eye) ;
    float3 right = normalize(cross(up, dir));

    float3 origin = eye - (dir * 1000.0f) ;

    float3 pixelPos = eye + (x-width/2) * right + (height/2-y) * up;
    float3 ray = normalize(pixelPos - origin) ;

    // Raytracing!
    float3 newRay = (float3)(0, 0, 0);
    float3 newRefracted = (float3)(0, 0, 0);
    float3 touchPos = (float3)(0, 0, 0);

    int currentSphereIdx = -1;
    int currentPlaneIdx  = -1;

    float R = 0, T = 0;

    float4 color = traceRay(eye, ray, spheres, numSpheres, planes, numPlanes, lights, numLights, temp, temp2, &newRay, &newRefracted, &R, &T, &touchPos, &currentSphereIdx, &currentPlaneIdx);


    // Gamma correction
    float3 gamma = (float3)(1.0f/2.2f, 1.0f/2.2f, 1.0f/2.2f);
    color = (float4)( pow(color.x, gamma.x),
                      pow(color.y, gamma.y),
                      pow(color.z, gamma.z),
                      color.w);

    // Write to texture
    write_imagef(texture, (int2)(x, height-y), color);
}
