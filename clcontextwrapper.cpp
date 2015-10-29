#include "clcontextwrapper.h"

#include <iostream>
#include <unordered_map>
#include <sstream>

#if __APPLE__
#include <OpenCL/cl.h>
#include <OpenCL/cl_ext.h>
#include <OpenCL/gcl.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/CGLContext.h>
#include <OpenGL/CGLCurrent.h>
#endif

#if !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#include <CL/opencl.h>
#include <CL/cl_ext.h>
#include <CL/cl_gl_ext.h>
#include <Windows.h>
#include <gl/GL.h>

typedef CL_API_ENTRY cl_int (CL_API_CALL *clGetGLContextInfoKHR_fn)(
    const cl_context_properties *properties,
    cl_gl_context_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret);

#define clGetGLContextInfoKHR clGetGLContextInfoKHR_proc
static clGetGLContextInfoKHR_fn clGetGLContextInfoKHR;
#endif

static std::string getError(cl_int error);
static inline void logError(const std::string & error, const std::string & errorDetail );

struct KernelInfo
{
    cl_kernel   kernel;
    size_t      workGroupSize;
};

struct CLContextWrapperPrivate
{
    cl_context          context;
    cl_command_queue    commandQueue;
    cl_device_id        deviceId;
    cl_program          computeProgram;

    std::unordered_map<std::string, KernelInfo> kernels;
    std::unordered_map<BufferId, cl_mem> buffers;
    BufferId nextBufferId;

    bool setKernelArg(cl_kernel kernel, KernelArg arg, int index)
    {
        size_t argSize = 0;
        void * data = arg.data;

        switch(arg.type)
        {
        case KernelArgType::GLOBAL:
        case KernelArgType::OPENGL:
            argSize = sizeof(cl_mem);
            data = &buffers.at(*static_cast<BufferId*>(arg.data));
            break;
        default:
            argSize = arg.byteSize;
            break;
        }

        cl_uint uindex = static_cast<cl_uint>(index);

        int err = clSetKernelArg(kernel, uindex, argSize, data);

        if(err)
        {
            logError("Error: Failed to set kernel arg!", getError(err));
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
        return "Invalid Device type";
    case CL_INVALID_WORK_GROUP_SIZE:
        return "Invalid work group size";
    case CL_INVALID_WORK_ITEM_SIZE:
        return "Invalid work item size";
    case CL_INVALID_GLOBAL_OFFSET:
        return "Invalid Global Offset";
    case CL_INVALID_EVENT_WAIT_LIST:
        return "Invalid Event Wait List";
    case CL_INVALID_GLOBAL_WORK_SIZE:
        return "Invalid Global Work Size";
    case CL_INVALID_GL_OBJECT:
        return "Invalid OpenGL Object";
    case CL_INVALID_MEM_OBJECT:
        return "Invalid Mem Object. (Invalid Buffer)";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        return "Invalid Image Format Descriptor";
    default:
    {
        std::stringstream ss;
        ss << "Erro number : " << std::to_string(error);
        return  ss.str();
    }
    }
}

static inline cl_mem_flags getMemFlags(BufferType type)
{
    switch (type) {
    case BufferType::READ_ONLY:
        return CL_MEM_READ_ONLY;
        break;
    case BufferType::WRITE_ONLY:
        return CL_MEM_WRITE_ONLY;
        break;
    case BufferType::READ_AND_WRITE:
        return CL_MEM_READ_WRITE;
        break;
    default:
        return CL_MEM_READ_WRITE;
        break;
    }
}

static inline void logError(const std::string & error, const std::string & errorDetail = "")
{
    std::cout << error << std::endl;
    std::cout << errorDetail << std::endl;
}

CLContextWrapper::CLContextWrapper() : _hasCreatedContext(false), _deviceType(DeviceType::NONE)
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

    if(_this->commandQueue)
    {
        clReleaseCommandQueue(_this->commandQueue);
    }
    if(_this->context)
    {
        clReleaseContext(_this->context);
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
        logError("Error: Failed get platorm id" , getError(err));
        return false;
    }

    // Get Device Info
    cl_device_type computeDeviceType = deviceType == DeviceType::GPU_DEVICE ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU; // for now, only these two types
    cl_device_id computeDeviceId;
    err = clGetDeviceIDs(platform, computeDeviceType, 1, &computeDeviceId, NULL);
    if (err != CL_SUCCESS)
    {
        logError("Error: Failed to locate a compute device!" , getError(err));
        return false;
    }

    size_t returned_size = 0;
    size_t max_workgroup_size = 0;
    err = clGetDeviceInfo(computeDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_workgroup_size, &returned_size);
    if (err != CL_SUCCESS)
    {
        logError("Error: Failed to retrieve device info!", getError(err));
        return false;
    }

    cl_char vendorName[1024] = {0};
    cl_char deviceName[1024] = {0};
    err = clGetDeviceInfo(computeDeviceId, CL_DEVICE_VENDOR, sizeof(vendorName), vendorName, &returned_size);
    err|= clGetDeviceInfo(computeDeviceId, CL_DEVICE_NAME, sizeof(deviceName), deviceName, &returned_size);
    if (err != CL_SUCCESS)
    {
        logError("Error: Failed to retrieve device info!", getError(err));
        return false;
    }

    std::cout << "Connecting to " <<  vendorName << " - " << deviceName << "..." << std::endl;

    // Create Context
    cl_context context = clCreateContext(0, 1, &computeDeviceId, NULL, NULL, &err);
    if (!_this->context || err)
    {
        logError("Error: Failed to create a compute ComputeContext!", getError(err));
        return false;
    }

    // Create Command Queue
    cl_command_queue commandQueue = clCreateCommandQueue(context, computeDeviceId, 0, &err);
    if (!_this->commandQueue)
    {
        logError("Error: Failed to create a command ComputeCommands!", getError(err));
        return false;
    }


    _this->deviceId = computeDeviceId;
    _this->context = context;
    _this->commandQueue = commandQueue;

    std::cout << "Successfully created OpenCL context " << std::endl;

    _hasCreatedContext = true;
    _deviceType = deviceType;

    return true;
}

#if __APPLE__

bool CLContextWrapper::createContextWithOpengl()
{

    cl_platform_id platform;

    cl_int err = clGetPlatformIDs(1, &platform, nullptr);

    if(err)
    {
        logError("Error: Failed get platorm id" , getError(err));
        return false;
    }

    // Get Device Info
    cl_device_id computeDeviceId;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &computeDeviceId, NULL);
    if (err != CL_SUCCESS)
    {
        logError("Error: Failed to locate a compute device!" , getError(err));
        return false;
    }

    CGLContextObj kCGLContext = CGLGetCurrentContext();
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);

    // Create CL context properties, add handle & share-group enum
    cl_context_properties properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 0
    };
     // Create a context with device in the CGL share group

    cl_context context = clCreateContext(properties, 0, &computeDeviceId, nullptr, 0, &err);

    if(err)
    {
        logError("Error creating OpenCL shared with with shared Opengl", getError(err));
        return false;
    }

    // Create Command Queue
    auto commandQueue = clCreateCommandQueue(context, computeDeviceId, 0, &err);
    if (!commandQueue)
    {
        logError("Error: Failed to create a command ComputeCommands with shared Opengl!", getError(err));
        return false;
    }

    _this->context = context;
    _this->deviceId = computeDeviceId;
    _this->commandQueue = commandQueue;

    _hasCreatedContext = true;
    _deviceType = DeviceType::GPU_DEVICE;
    return true;
}

#else
bool CLContextWrapper::createContextWithOpengl()
{
    cl_platform_id platform;

    cl_int err = clGetPlatformIDs(1, &platform, nullptr);

    if(err)
    {
        logError("Error: Failed get platorm id" , getError(err));
        return false;
    }

    // Create CL context properties, add handle & share-group enum
    auto glContext = wglGetCurrentContext();
    auto glDc = wglGetCurrentDC();

    cl_context_properties properties[] = {
        CL_GL_CONTEXT_KHR,  (cl_context_properties) glContext,
        CL_WGL_HDC_KHR,     (cl_context_properties) glDc,
        CL_CONTEXT_PLATFORM,(cl_context_properties) platform,
        0
    };

    // Get Device Info
    // The easy way
    cl_device_id computeDeviceId;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &computeDeviceId, NULL);
    if (err)
    {
        // The "hard" way
        if (!clGetGLContextInfoKHR)
        {
            clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)
                                  clGetExtensionFunctionAddressForPlatform(platform, "clGetGLContextInfoKHR");
            if (!clGetGLContextInfoKHR)
            {
                logError("Error: Failed to locate a compute device!" , "Failed to query proc address for clGetGLContextInfoKHR");
                return false;
            }

            // Get the first
            err = clGetGLContextInfoKHR(properties, CL_DEVICES_FOR_GL_CONTEXT_KHR, sizeof(cl_device_id), &computeDeviceId, nullptr);

            if(!computeDeviceId)
            {
                logError("Error: Failed to locate a compute device!" , getError(err));
            }
        }
    }

    // Create a context with device in the CGL share group
    cl_context context = clCreateContext(properties, 1, &computeDeviceId, nullptr, 0, &err);

    if(err)
    {
        logError("Error creating OpenCL shared with with shared Opengl", getError(err));
        return false;
    }

    // Create Command Queue
    auto commandQueue = clCreateCommandQueue(context, computeDeviceId, 0, &err);
    if (!commandQueue)
    {
        logError("Error: Failed to create a command ComputeCommands with shared Opengl!", getError(err));
        return false;
    }

    _this->deviceId = computeDeviceId;
    _this->context = context;
    _this->commandQueue = commandQueue;

    _hasCreatedContext = true;
    _deviceType = DeviceType::GPU_DEVICE;
    return true;
    return false;
}

#endif

bool CLContextWrapper::hasCreatedContext() const
{
    return _hasCreatedContext;
}

DeviceType CLContextWrapper::getContextDeviceType() const
{
    return _deviceType;
}

bool CLContextWrapper::createProgramFromSource(const std::string & source)
{
    cl_int err = 0;

    // Create program
    const char * sourcePtr = source.c_str();

    _this->computeProgram = clCreateProgramWithSource(_this->context, 1,  &sourcePtr, nullptr, &err);
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

    std::cout << "Succesfully prepared kernel '" << kernelName << "'" << std::endl;

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

    err = clEnqueueNDRangeKernel(_this->commandQueue,
                                 kernel,
                                 range.workDim,
                                 range.globalOffset,
                                 range.globalSize,
                                 range.localSize,
                                 0, nullptr, nullptr );

    if(err)
    {
        logError("Error: Failed to dispatch kernel", getError(err));
        return false;
    }

    std::cout << "Successfully dispatched kernel \"" << kernelName <<"\"" << std::endl;
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


BufferId CLContextWrapper::createBuffer(size_t bytesSize, void * hostData, BufferType type)
{
    cl_int err;
    cl_mem_flags flags  = getMemFlags(type);

    if(hostData)
    {
        flags |= CL_MEM_COPY_HOST_PTR;
    }

    cl_mem buffer =  clCreateBuffer(_this->context, flags, bytesSize, hostData,  &err);
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

    err = clEnqueueWriteBuffer(_this->commandQueue,
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

    err = clEnqueueReadBuffer(_this->commandQueue,
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

BufferId CLContextWrapper::shareGLTexture(const GLTextureId textureId, BufferType type)
{
    auto flags = getMemFlags(type);

    cl_int err = 0;

    cl_mem mem = clCreateFromGLTexture(_this->context, flags, GL_TEXTURE_2D, 0, textureId, &err);

    if(err || !mem)
    {
        logError("Error: Failed to share texture with Opengl.", getError(err));
        return 0;
    }

    _this->buffers[_this->nextBufferId] = mem;
    const BufferId outBufferID = _this->nextBufferId;
    _this->nextBufferId++;

    return outBufferID;

}

void CLContextWrapper::executeSafeAndSyncronized(BufferId * textureToLock, unsigned int count, std::function<void()> exec)
{
    cl_mem * objs = new cl_mem[count];

    for(size_t i=0 ; i < count ;i++)
    {
        objs[i] = _this->buffers[textureToLock[i]];
    }

    clEnqueueAcquireGLObjects(_this->commandQueue, count, objs, 0, nullptr, nullptr);

    exec();

    clEnqueueReleaseGLObjects(_this->commandQueue, count, objs, 0, nullptr, nullptr);

    clFinish(_this->commandQueue);
    delete [] objs;
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


