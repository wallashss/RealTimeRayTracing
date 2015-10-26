#pragma once


#include <string>
#include <vector>


struct CLContextWrapperPrivate;

enum class DeviceType
{
    GPU_DEVICE,
    CPU_DEVICE
};

enum class BufferType
{
    READ_ONLY,
    WRITE_ONLY,
    READ_AND_WRITE
};

struct NDRange
{
    unsigned int workDim;
    size_t globalSize[3];
    size_t localSize[3];
    size_t globalOffset[3];

    NDRange()
    {
        workDim = 0;
        globalSize[0] = globalSize[1] = globalSize[2] = 0;
        localSize[0] = localSize[1] = localSize[2] = 0;
        globalOffset[0] = globalOffset[1] = globalOffset[2];
    }
};

enum class KernelArgType
{
    GLOBAL,
    LOCAL,
    CONSTANT
};

struct KernelArg
{
    int           index;
    KernelArgType type;
    size_t        byteSize;
    void          *data;
};

typedef unsigned int BufferId;


class CLContextWrapper
{
public:
    CLContextWrapper();

    ~CLContextWrapper();

    bool createContext(DeviceType deviceType = DeviceType::CPU_DEVICE);

    bool hasCreatedContext() const;

    bool createProgramFromSource(const std::string & source);

    bool prepareKernel(const std::string & kernelName);

    size_t getWorkGroupSize(const std::string & kernelName) const;

    BufferId createBuffer(size_t bytesSize, void * hostData = nullptr, BufferType type = BufferType::READ_AND_WRITE);

    bool uploadToBuffer(BufferId id, size_t bytesSize, void * data, size_t offset = 0, const bool blocking = true);

    bool dowloadFromBuffer(BufferId id, size_t bytesSize, void * data, size_t offset = 0, const bool blocking = true);

    bool dispatchKernel(const std::string& kernelName, NDRange range);

    bool dispatchKernel(const std::string& kernelName, NDRange range, const std::vector<KernelArg>& args);

    bool setKernelArg(const std::string & kernelName, KernelArg args, int index);

private:
    CLContextWrapperPrivate * _this;
    bool _hasCreatedContext;


};

