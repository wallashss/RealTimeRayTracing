#include "clcontextwrapper.h"

#include <iostream>
#include <unordered_map>
#include <sstream>

#if __APPLE__
#include <OpenCL/cl.h>
#endif

#if !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#include <CL/opencl.h>
#endif

static std::string getError(cl_int error);

struct KernelInfo
{
    cl_kernel   kernel;
    size_t      workGroupSize;
};

struct CLContextWrapperPrivate
{
    cl_context          computeContext;
    cl_command_queue    computeCommands;
    cl_device_id        deviceId;
    cl_program          computeProgram;

    std::unordered_map<std::string, KernelInfo> kernels;
    std::unordered_map<BufferId, cl_mem> buffers;
    BufferId nextBufferId;

    bool setKernelArg(cl_kernel kernel, KernelArg arg, int index)
    {
        size_t argSize = 0;
        void * data = arg.data;
        if(arg.type == KernelArgType::GLOBAL)
        {
            argSize = sizeof(cl_mem);
            data = &buffers.at(*static_cast<BufferId*>(arg.data));
        }
        else
        {
            argSize = arg.byteSize;
        }

        cl_uint uindex = static_cast<cl_uint>(index);

        int err = clSetKernelArg(kernel, uindex, argSize, data);

        if(err)
        {
            std::cout << "Error: Failed to set kernel arg!" << std::endl;
            std::cout << getError(err);
            return false;
        }
        return true;
    }
};

static std::string getError(cl_int error)
{
    switch (error)
    {
    case CL_SUCCESS:
        return "Success";
    case CL_INVALID_CONTEXT:
        return "Invalid Context";
    case CL_INVALID_VALUE:
        return "Invalid Value";
    case CL_INVALID_BUFFER_SIZE:
        return "Invalid Buffer Size";
    case CL_INVALID_HOST_PTR:
        return "Invalid Host Pointer";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        return "Failure to allocate memory for object";
    case CL_OUT_OF_HOST_MEMORY:
        return "Out of host memory";
    case CL_INVALID_PROGRAM_EXECUTABLE:
        return "Invalid Program Executable";
    case CL_INVALID_COMMAND_QUEUE:
        return "Invalid Command Queue";
    case CL_INVALID_KERNEL:
        return "Invalid Kernel";
    case CL_INVALID_KERNEL_ARGS:
        return "Invalid Kernel Arguments";
    case CL_INVALID_WORK_DIMENSION:
        return "Invalid Work Dimension";
    case CL_INVALID_PLATFORM:
        return "Invalid Platform";
    case CL_DEVICE_NOT_FOUND:
        return "Device not found";
    case CL_INVALID_DEVICE_TYPE:
        return "Invalide Device type";
    default:
    {
        std::stringstream ss;
        ss << "Erro number :" << std::to_string(error);
        return  ss.str();
    }
    }
}

CLContextWrapper::CLContextWrapper() : _hasCreatedContext(false)
{
    _this = new CLContextWrapperPrivate;
    _this->nextBufferId = 1;
}

CLContextWrapper::~CLContextWrapper()
{

    for(auto it : _this->kernels)
    {
        clReleaseKernel(it.second.kernel);
    }
    for(auto it : _this->buffers)
    {
        clReleaseMemObject(it.second);
    }

    if(_this->computeCommands)
    {
        clReleaseCommandQueue(_this->computeCommands);
    }
    if(_this->computeContext)
    {
        clReleaseContext(_this->computeContext);
    }

    delete _this;
}

bool CLContextWrapper::createContext(DeviceType deviceType)
{
    // Check if has already created a context
    if(_hasCreatedContext)
    {
        return false;
    }

    cl_int err = CL_SUCCESS;

    // Get Platform
    cl_uint numPlatforms = 0;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);
    if(numPlatforms == 0)
    {
        return false;
    }

    // Get the first found platform
    cl_platform_id platform;

    err = clGetPlatformIDs(1, &platform, nullptr);

    if(err)
    {
        std::cout << "Error: Failed get platorm id" << std::endl;
        std::cout << getError(err);
        return false;
    }

    // Get Device Info
    cl_device_type  computeDeviceType = deviceType == DeviceType::GPU_DEVICE ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU; // for now, only these two types
    cl_device_id computeDeviceId;
    err = clGetDeviceIDs(platform, computeDeviceType, 1, &computeDeviceId, NULL);
    if (err != CL_SUCCESS)
    {
        std::cout << "Error: Failed to locate a compute device!" << std::endl;
        std::cout << getError(err) << std::endl;
        return false;
    }

    size_t returned_size = 0;
    size_t max_workgroup_size = 0;
    err = clGetDeviceInfo(computeDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_workgroup_size, &returned_size);
    if (err != CL_SUCCESS)
    {
        std::cout << "Error: Failed to retrieve device info!" << std::endl;
        return false;
    }

    cl_char vendorName[1024] = {0};
    cl_char deviceName[1024] = {0};
    err = clGetDeviceInfo(computeDeviceId, CL_DEVICE_VENDOR, sizeof(vendorName), vendorName, &returned_size);
    err|= clGetDeviceInfo(computeDeviceId, CL_DEVICE_NAME, sizeof(deviceName), deviceName, &returned_size);
    if (err != CL_SUCCESS)
    {
        std::cout << "Error: Failed to retrieve device info!" << std::endl;
        return false;
    }

    std::cout << "Connecting to " <<  vendorName << " - " << deviceName << "..." << std::endl;

    // Create Context
    _this->computeContext = clCreateContext(0, 1, &computeDeviceId, NULL, NULL, &err);
    if (!_this->computeContext)
    {
        printf("Error: Failed to create a compute ComputeContext!\n");
        return false;
    }

    // Create Command Queue
    _this->computeCommands = clCreateCommandQueue(_this->computeContext, computeDeviceId, 0, &err);
    if (!_this->computeCommands)
    {
        printf("Error: Failed to create a command ComputeCommands!\n");
        return false;
    }


    _this->deviceId = computeDeviceId;
    std::cout << "Successfully created context " << std::endl;

    _hasCreatedContext = true;

    return true;
}

bool CLContextWrapper::hasCreatedContext() const
{
    return _hasCreatedContext;
}

bool CLContextWrapper::createProgramFromSource(const std::string & source)
{
    cl_int err = 0;

    // Create program
    const char * sourcePtr = source.c_str();

    _this->computeProgram = clCreateProgramWithSource(_this->computeContext, 1,  &sourcePtr, nullptr, &err);
    if (!_this->computeProgram || err != CL_SUCCESS)
    {
        std::cout << source << std::endl;
        std::cout << "Error: Failed to create compute program! " << std::endl;
        return false;
    }

    // Build the program executable
    err = clBuildProgram(_this->computeProgram, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t length;
        char buildLog[2048];
        std::cout <<  source << std::endl;
        std::cout << "Error: Failed to build program executable!" << std::endl;
        clGetProgramBuildInfo(_this->computeProgram, _this->deviceId, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, &length);
        std::cout << buildLog << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Succesfully created program" << std::endl;
    return true;
}


bool CLContextWrapper::prepareKernel(const std::string & kernelName)
{
    cl_int err;

    // Create Kernel
    cl_kernel newKernel = clCreateKernel(_this->computeProgram, kernelName.c_str(), &err);

    if (!newKernel || err != CL_SUCCESS)
    {
        std::cout << "Error: Failed to create compute kernel!" << std::endl;
        return false;
    }

    // Get workgroup size
    size_t wgSize;
    err = clGetKernelWorkGroupInfo(newKernel, _this->deviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgSize, NULL);
    if(err)
    {
        std::cout << "Error: Failed to get kernel work group size" << std::endl;
        return false;
    }

    KernelInfo info;
    info.kernel = newKernel;
    info.workGroupSize = wgSize;

    _this->kernels[kernelName] = info;

    std::cout << "Succesfully prepared kernel" << std::endl;

    return true;
}

size_t CLContextWrapper::getWorkGroupSize(const std::string & kernelName) const
{
    auto it =_this->kernels.find(kernelName);
    if(it != _this->kernels.end())
    {
        return it->second.workGroupSize;
    }
    return 0;
}


BufferId CLContextWrapper::createBuffer(size_t bytesSize, void * hostData, BufferType type)
{
    cl_int err;
    cl_mem_flags flags =  CL_MEM_READ_WRITE;

    switch (type) {
    case BufferType::READ_ONLY:
        flags = CL_MEM_READ_ONLY;
        break;
    case BufferType::WRITE_ONLY:
        flags = CL_MEM_WRITE_ONLY;
        break;
    case BufferType::READ_AND_WRITE:
        flags = CL_MEM_READ_WRITE;
        break;
    default:
        break;
    }

    if(hostData)
    {
        flags |= CL_MEM_COPY_HOST_PTR;
    }

    cl_mem buffer =  clCreateBuffer(_this->computeContext, flags, bytesSize, hostData,  &err);
    if(!buffer)
    {
        std::cout << "Error: Failed to allocate buffer" << std::endl;
        std::cout << getError(err) << std::endl;
        return 0;
    }

    auto newId = _this->nextBufferId;
    _this->buffers[_this->nextBufferId++] = buffer;
    return newId;
}

bool CLContextWrapper::uploadToBuffer(BufferId id, size_t bytesSize, void * data, size_t offset,  const bool blocking)
{
    cl_int err = 0;

    auto it = _this->buffers.find(id);
    if(it == _this->buffers.end())
    {
        std::cout << "Error: Buffer Id not created" << std::endl;
        return false;
    }

    err = clEnqueueWriteBuffer(_this->computeCommands,
                               it->second,
                               blocking ? CL_TRUE : CL_FALSE,
                               offset,
                               bytesSize,
                               data,
                               0, NULL, NULL);

    if(err)
    {
        std::cout << "Error: Failed to upload data to buffer" << std::endl;
        std::cout << getError(err) << std::endl;
        return false;
    }
    return true;
}


bool CLContextWrapper::dowloadFromBuffer(BufferId id, size_t bytesSize, void * data, size_t offset , const bool blocking)
{
    cl_int err = 0;

    auto it = _this->buffers.find(id);
    if(it == _this->buffers.end())
    {
        std::cout << "Error: Buffer Id not created" << std::endl;
        return false;
    }

    err = clEnqueueReadBuffer(_this->computeCommands,
                              it->second,
                              blocking ? CL_TRUE : CL_FALSE,
                              offset,
                              bytesSize,
                              data,
                              0, NULL, NULL);

    if(err)
    {
        std::cout << "Error: Failed to download data from buffer" << std::endl;
        std::cout << getError(err) << std::endl;
        return false;
    }
    return true;
}

bool CLContextWrapper::dispatchKernel(const std::string& kernelName, NDRange range)
{
    return dispatchKernel(kernelName, range, std::vector<KernelArg>());
}

bool CLContextWrapper::dispatchKernel(const std::string& kernelName, NDRange range,const std::vector<KernelArg>& args)
{
    cl_int err = 0;

    // TODO: check if global size and local size are evenly

    auto it =_this->kernels.find(kernelName);
    if(it == _this->kernels.end())
    {
        std::cout << "Error: Failed to find kernel to dispatch" << std::endl;
        return false;
    }

    cl_kernel kernel = it->second.kernel;

    if(!args.empty())
    {

        bool allOk = true;
        for(decltype(args.size()) i = 0; i < args.size() ; i++)
        {
            bool ok =_this->setKernelArg(kernel, args[i], static_cast<int>(i));
            if(!ok)
            {
                std::cout << "Failed to setup arg for index " << i << std::endl;
            }
            allOk &= ok;
        }
        if(!allOk)
        {
            return false;
        }
    }

    err = clEnqueueNDRangeKernel(_this->computeCommands,
                                 kernel,
                                 range.workDim,
                                 range.globalOffset,
                                 range.globalSize,
                                 range.localSize,
                                 0, nullptr, nullptr );

    if(err)
    {
        std::cout << "Error: Failed to dispatch kernel" << std::endl;
        std::cout << getError(err) << std::endl;
        return false;
    }

    std::cout << "Successfully dispatched kernel" << std::endl;
    return true;
}

bool CLContextWrapper::setKernelArg(const std::string & kernelName, KernelArg arg, int index)
{
    auto it = _this->kernels.find(kernelName);
    if(it == _this->kernels.end())
    {
        std::cout << "Kernel not found! " << std::endl;
        return false;
    }

    return _this->setKernelArg(it->second.kernel, arg, index);
}

std::vector<std::string> CLContextWrapper::listAvailablePlatforms()
{
    cl_int err = 0;
    cl_uint numPlatforms = 0;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);

    if(numPlatforms ==0)
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> platformList;
    for(cl_uint i =0; i < numPlatforms ; i++)
    {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        err = clGetPlatformIDs(numPlatforms, platforms, nullptr);
        for (unsigned i = 0; i < numPlatforms; ++i)
        {
            char platformName[100];
            err = clGetPlatformInfo(platforms[i],
                                       CL_PLATFORM_VENDOR,
                                       sizeof(platformName),
                                       platformName,
                                       nullptr);
            platformList.push_back(platformName);
        }
    }
    return platformList;

}
