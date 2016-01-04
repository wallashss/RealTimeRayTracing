#pragma once


#include <functional>
#include <string>
#include <vector>


struct CLContextWrapperPrivate;

enum class DeviceType
{
    GPU_DEVICE,
    CPU_DEVICE,
    NONE
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
        globalOffset[0] = globalOffset[1] = globalOffset[2] = 0;
    }
};

enum class TextureWrapMode
{
    CLAMP,
    REPEAT
};

enum class TextureFilterMode
{
    LINEAR,
    NEAREST
};


struct TextureParams
{
    TextureWrapMode wrapMode;
    TextureFilterMode filterMode;

};

struct KernelArg
{
    size_t        byteSize;
    void          *data;

    template <typename T>
    KernelArg(T* aData) : byteSize(sizeof(T)), data(aData)
    {

    }

    KernelArg(void* aData, size_t size) : byteSize(size), data(aData)
    {

    }

    static KernelArg getShared(size_t size)
    {
        return KernelArg(0, size);
    }
};



typedef void * BufferId; // TODO: Make an assert to ensure cl_mem = void *
typedef unsigned int GLTextureId;

class CLContextWrapper
{
public:
    CLContextWrapper();

    ~CLContextWrapper();

    // Context Creation

    bool createContext(DeviceType deviceType = DeviceType::CPU_DEVICE);

    bool createContextWithOpengl();

    // Context status

    bool hasCreatedContext() const;

    DeviceType getContextDeviceType() const;

    // Kernel

    bool createProgramFromSource(const std::string & source);

    bool prepareKernel(const std::string & kernelName);

    size_t getWorkGroupSize(const std::string & kernelName) const;

    bool dispatchKernel(const std::string& kernelName, NDRange range);

    bool dispatchKernel(const std::string& kernelName, NDRange range, const std::vector<KernelArg>& args);

    bool setKernelArg(const std::string & kernelName, KernelArg args, int index);

    void finish();

    // OpenCL Buffers

    template <typename T>
    BufferId createBuffer(size_t count, T * hostData = nullptr, BufferType type = BufferType::READ_AND_WRITE)
    {
        return createBufferWithBytes(sizeof(T)*count, hostData, type);
    }

    BufferId createBufferWithBytes(size_t bytesSize, void * hostData = nullptr, BufferType type = BufferType::READ_AND_WRITE);

    template <typename T>
    bool uploadArrayToBuffer(BufferId id, size_t count, T * data, size_t offset = 0,  const bool blocking = true)
    {
        return uploadToBuffer(id, sizeof(T) * count, data, offset, blocking);
    }

    bool uploadToBuffer(BufferId id, size_t bytesSize, void * data, size_t offset = 0, const bool blocking = true);

    template <typename T>
    bool dowloadArrayFromBuffer(BufferId id, size_t count, T * data, size_t offset = 0, const bool blocking = true)
    {
        return dowloadFromBuffer(id, sizeof(T)* count, data, offset, blocking);
    }

    bool dowloadFromBuffer(BufferId id, size_t bytesSize, void * data, size_t offset = 0, const bool blocking = true);

    // OpenGL

    BufferId shareGLTexture(const GLTextureId id, BufferType type);

    void executeSafeAndSyncronized(BufferId * textureToLock, unsigned int count, std::function<void()> exec);


    // Static util
    static std::vector<std::string> listAvailablePlatforms();

private:
    CLContextWrapperPrivate * _this;
    bool _hasCreatedContext;
    DeviceType _deviceType;

};

