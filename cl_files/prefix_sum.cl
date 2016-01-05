
// Naive Scan
#if 0
static int _prefixSum(__local int *temp,
                       int  n)
{
    int thid = get_local_id(0);

    for (int offset = 1; offset < n; offset *= 2)
    {
    if (thid >= offset)
    {
        temp[thid] += temp[n+thid - offset];
    }
    else
    {
        temp[thid] = temp[n+thid];
    }
        barrier(CLK_LOCAL_MEM_FENCE);
        temp[n+thid] = temp[thid];
    }
    return temp[thid];
}


__kernel void prefixSum(__global int * input,
                         __global int * output,
                         __local  int * temp,
                         int n)
{
   int globalId = get_global_id(0);
   int localId = get_local_id(0);

   temp[localId] =  localId > 0 ? input[globalId-1] :0;
   temp[n + localId] = temp[localId];

   barrier(CLK_LOCAL_MEM_FENCE);

   output[globalId] = _prefixSum(temp,n);
}

#else

// My own implementation (not efficient) of prefix sum
// Reference: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch39.html
__kernel void prefixSum(__global int * input,
                         __global int * output,
                         __local  int * temp,
                         int n)
{
    int thid = get_global_id(0);
    int lthid = get_local_id(0);
    int pout = 0, pin = 1;

//    temp[pout * n + thid] = (thid > 0) ? input[thid-1] : 0;
    temp[lthid] = input[thid];

    int offset = 1;
    for (int d = 1; d < n<<1 ; d <<= 1)
    {
        barrier(CLK_LOCAL_MEM_FENCE);
        // swap double buffer indices
        pout = 1 - pout;
        pin  = 1 - pout;
        // offset     = 2 ^ d
        // offset * 2 = 2 ^ (d + 1)

        // x[k + 2 ^ (d+1) - 1] = x [k + 2^d-1] + x[k+2^d +1-1];

        if((lthid + 1) % (offset * 2) == 0)
        {
            temp[(pout*n + lthid) ] = temp[(pin*n + lthid)] + temp[(pin*n + lthid) - offset];
        }
        else
        {
            temp[(pout*n + lthid)] = temp[pin * n + lthid]; // repass
        }
        offset <<= 1; // ~= d++
    }
    if(lthid == n-1)
    {
        temp[(pout*n + lthid) ] = 0;
    }
    offset = n;
    for (int d = 1; d < n <<1 ; d <<= 1)
    {
        barrier(CLK_LOCAL_MEM_FENCE);
        pout = 1 - pout;
        pin  = 1 - pout;

        if((lthid + 1) % (offset) == 0)
        {
            int t  = temp[(pin*n + lthid)];
            int t2 = temp[(pin*n + lthid) - offset/2];

            temp[(pout*n + lthid)]            = t + t2;
            temp[(pout*n + lthid) - offset/2] = t;
        }
        else if((lthid + 1) % (offset/2) == 0)
        {
        	// Ugly
        }
        else
        {
            temp[(pout*n + lthid)] = temp[pin * n + lthid]; // repass
        }
        offset >>= 1;
    }

    output[thid] = temp[pout*n + lthid]; // write output
}

#endif