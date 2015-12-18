__constant float MINIMUM_INTERSECT_DISTANCE = 1e-7;


__kernel void testKernel(__global const int *input,
                         __global int       *output)
{
//    const uint local_id = get_local_id(0);
    const uint globalId = get_global_id(0);


    output[globalId] = input[globalId] + 1;
}


void swap(float * a, float * b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

bool hasIntercepted(float4 * sphere, float3 ray, float3 origin, float3 * touchPoint)
{
    float rayDirX = ray.x;
    float rayDirY = ray.y;
    float rayDirZ = ray.z;

    float rayOriginX = origin.x;
    float rayOriginY = origin.y;
    float rayOriginZ = origin.z;

    float sphereCenterX = (*sphere).x;
    float sphereCenterY = (*sphere).y;
    float sphereCenterZ = (*sphere).z;
    float radius =        (*sphere).w;

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

float4 traceRay(float3 eye,
                float3 ray,
                __global float * spheres,
                __global float * colorsSpheres,
                int numSpheres)
{
    float4 outColor = (float4)(0.0f,0.0f,0.0f,1.0f);
    for(int i = 0 ; i < numSpheres ; i++)
    {
        float4 sphere = vload4(i, spheres);
        float3 touchPoint = (float3)(0.0f, 0.0f, 0.0f);
        if(hasIntercepted(&sphere, ray, eye, &touchPoint))
        {
            float3 sphereColor = vload3(i, colorsSpheres);
            return (float4)(sphereColor.x, sphereColor.y, sphereColor.z, 1.0f);
        }
    }
    return outColor;
}


__kernel void rayTracing(__write_only image2d_t texture,
                         __global float * spheres,
                         __global float * colorsSpheres,
                         int numSpheres,
                         float eyeX, float eyeY, float eyeZ,
                         int width, int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    float3 ray = (float3)(x-width/2,height/2-y,1000);
    ray = normalize(ray);

    float3 eye = (float3)(eyeX, eyeY, eyeZ);

    float4 color = traceRay(eye, ray, spheres, colorsSpheres, numSpheres);

    write_imagef(texture, (int2)(x, y), color);


//    float a = ((float) x)/(float) width;
//    float b = ((float) y)/(float) height;
    //write_imagef(texture, (int2)(x, y), (float4)(a, b, 1.0f, 1.f));



//     if((x < width/2) && y < height/2)
//    if(width == 640)
//    {
//        write_imagef(texture, (int2)(x, y), (float4)(0.0f, 1.0f, 1.0f, 1.f));
//    }
//    else
//    {
//        write_imagef(texture, (int2)(x, y), (float4)(0.0f, 0.0f, 0.0f, 1.f));
//    }


}


__kernel void testTexture(__write_only image2d_t texture)
{
//    const uint local_id = get_local_id(0);
//    const uint globalId = get_global_id(0);
//    const uint globalId = get_global_id(1);
//    const uint group_id = get_global_id(0) / get_local_size(0);
//    const uint group_size = get_local_size(0);

       int x = get_global_id(0);
       int y = get_global_id(1);

//    texture[globalId] = globalId % 2 == 0 ? 0xFF00FFFF : 0xFFFFFFFF;

    if(x == y)
    {
        write_imagef(texture, (int2)(x, y), (float4)(1.0f, 1.0f, 1.0f, 1.f));
    }
    else
    {
        write_imagef(texture, (int2)(x, y), (float4)(0.0f, 0.0f, 0.0f, 1.f));
    }


}
