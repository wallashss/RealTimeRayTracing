# Opencl qmake

OPENCL_PATH = /System/Library/Frameworks/OpenCL.framework/
OPENCLC = $${OPENCL_PATH}Libraries/openclc
OPENCL_OUT = $$_PRO_FILE_PWD_/opencl_build
# Just to see the files in your project
#OTHER_FILES += cl_files/*.cl

# add Framework
QMAKE_LFLAGS += -F $${OPENCL_PATH}
LIBS += -framework OpenCL


QMAKE_LFLAGS += -F /System/Library/Frameworks/OpenGL.framework/
LIBS += -framework OpenGL


# openclc to generate code from .cl
BEGIN_BUILD_CL =  $${OPENCLC} -x cl -cl-std=CL1.1  -cl-auto-vectorize-enable -gcl-bc-dir $${OPENCL_OUT} -emit-gcl
END_BUILD_CL = -gcl-output-dir $${OPENCL_OUT}

#Generate c sources and headers
files= $$files(cl_files/*.cl)
for(file, files):system($$BEGIN_BUILD_CL $$file $$END_BUILD_CL)

#Add source to project
files= $$files($${OPENCL_OUT}/*.cl.c)
for(file, files):SOURCES += $$file

#Add headers to project
files= $$files($${OPENCL_OUT}/*.cl.h)
for(file, files):HEADERS += $$file

#Generate Bitcode for each platform
files= $$files(cl_files/*.cl)

BEGIN_BUILD_CL =$${OPENCLC} -emit-llvm -c -arch


for(file, files){

#message($$replace(file, .cl, .gpu_64))



#cpu-32
ARCH = i386
filename = $${file}.$${ARCH}.bc
filename = $$basename(filename)
system($$BEGIN_BUILD_CL $$ARCH $$file -o $${OPENCL_OUT}/$$filename)


##cpu-64
ARCH = x86_64
filename = $${file}.$${ARCH}.bc
filename = $$basename(filename)
system($$BEGIN_BUILD_CL $$ARCH $$file -o $${OPENCL_OUT}/$$filename)

##gpu-32
ARCH = gpu_32
filename = $${file}.$${ARCH}.bc
filename = $$basename(filename)
system($$BEGIN_BUILD_CL $$ARCH $$file -o $${OPENCL_OUT}/$$filename)

##gpu-64
ARCH = gpu_64
filename = $${file}.$${ARCH}.bc
filename = $$basename(filename)
system($$BEGIN_BUILD_CL $$ARCH $$file -o $${OPENCL_OUT}/$$filename)

}
#/System/Library/Frameworks/OpenCL.framework/Libraries/openclc -emit-llvm -c -arch gpu_64 kernel.cl -o kernel.gpu64.bc
