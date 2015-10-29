__kernel void testKernel(__global const int *input,
                         __global int       *output)
{
//    const uint local_id = get_local_id(0);
    const uint globalId = get_global_id(0);
//    const uint group_id = get_global_id(0) / get_local_size(0);
//    const uint group_size = get_local_size(0);

    output[globalId] = input[globalId] + 1;
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
