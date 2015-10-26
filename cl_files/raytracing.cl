__kernel void testKernel(__global const int *input,
                         __global int       *output)
{
//    const uint local_id = get_local_id(0);
    const uint globalId = get_global_id(0);
//    const uint group_id = get_global_id(0) / get_local_size(0);
//    const uint group_size = get_local_size(0);

    output[globalId] = input[globalId] + 1;
}
