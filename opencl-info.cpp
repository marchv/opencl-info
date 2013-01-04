// copyright 2013 Jens Schwarzer (schwarzer@schwarzer.dk)
// OpenCL info dump

// TODO: make nice alignment by using alignas for 'buf' once g++ 4.8 is available

#include <iostream>
#include <tuple>
#include <vector>

//#include "CL/cl.hpp" - too much exception handling :S
#include "CL/cl.h"

using std::cout;

// macros to reduce pollution
#define B(w) if (static_cast<uint64_t>(*buf) & w) cout << #w " ";
#define C(w) case w: cout << #w; break;
#define P(w, typ) std::make_tuple(w, #w, typ)

typedef enum {
    kCharArray,
    kClBool,
    kClCommandQueueProperties,
    kClDeviceAffinityDomain,
    kClDeviceExecCapabilities,
    kClDeviceFpConfig,
    kClDeviceId,
    kClDeviceLocalMemType,
    kClDeviceMemCacheType,
    kClDevicePartitionPropertyArray,
    kClDeviceType,
    kClPlatformId,
    kClUint,
    kClUlong,
    kSizeT,
    kSizeTArray,
    } ClTypeEnum;

static void print_error(cl_int ret, bool abort, unsigned abort_line = 0) {
    switch (ret)
        {
        case CL_SUCCESS: return;
        C(CL_DEVICE_NOT_FOUND);
        C(CL_INVALID_DEVICE);
        C(CL_INVALID_DEVICE_TYPE);
        C(CL_INVALID_PLATFORM);
        C(CL_INVALID_VALUE);
        C(CL_OUT_OF_HOST_MEMORY);
        C(CL_OUT_OF_RESOURCES);
        default: cout << "Unknown error: " << ret; break;
        }
    if (abort) {
        cout << "\nAborted in line " << abort_line << "!\n";
        exit(-1);
        }
    }

static void print_info(const std::string &str, ClTypeEnum cl_type, cl_int ret, const uint64_t *buf, const size_t param_value_size_ret) {
    cout << str << ": ";
    if (ret == CL_SUCCESS) {
        switch (cl_type)
            {
            case kCharArray: cout << reinterpret_cast<const char *>(buf); break;
            case kClBool:    cout << std::boolalpha << static_cast<bool>(*buf); break;
            case kClCommandQueueProperties:
                B(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
                B(CL_QUEUE_PROFILING_ENABLE);
                break;
                // case kClDeviceAffinityDomain:
            case kClDeviceExecCapabilities:
                B(CL_EXEC_KERNEL);
                B(CL_EXEC_NATIVE_KERNEL);
                break;
            case kClDeviceFpConfig:
                B(CL_FP_DENORM);
                B(CL_FP_INF_NAN);
                B(CL_FP_ROUND_TO_NEAREST);
                B(CL_FP_ROUND_TO_ZERO);
                B(CL_FP_ROUND_TO_INF);
                B(CL_FP_FMA);
                B(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT);
                B(CL_FP_SOFT_FLOAT);
                break;
                // case kClDeviceId:
            case kClDeviceLocalMemType:
                switch (static_cast<cl_device_local_mem_type>(*buf))
                    {
                    C(CL_NONE);
                    C(CL_LOCAL);
                    C(CL_GLOBAL);
                    }
                break;
            case kClDeviceMemCacheType:
                switch (static_cast<cl_device_fp_config>(*buf))
                    {
                    C(CL_NONE);
                    C(CL_READ_ONLY_CACHE);
                    C(CL_READ_WRITE_CACHE);
                    }
                break;
                // case kClDevicePartitionPropertyArray:
            case kClDeviceType:
                B(CL_DEVICE_TYPE_CPU);
                B(CL_DEVICE_TYPE_GPU);
                B(CL_DEVICE_TYPE_ACCELERATOR);
                B(CL_DEVICE_TYPE_DEFAULT);
                B(CL_DEVICE_TYPE_CUSTOM);
                break;
                // case kClPlatformId:
            case kClUint:     cout << static_cast<unsigned int>(*buf); break;
            case kClUlong:    cout << static_cast<unsigned long>(*buf); break;
            case kSizeT:      cout << static_cast<size_t>(*buf); break;
            case kSizeTArray:
                // array-dimension derived implicitly from size
                for (unsigned i = 0; i < param_value_size_ret / sizeof(size_t); ++i) {
                    cout << reinterpret_cast<const size_t*>(buf)[i] << " ";
                    }
                break;
            default: cout << "<IMPLEMENTATION MISSING>";
            }
        }
    else {
        print_error(ret, false);
        }
    cout << "\n";
    }

int main() {
    cl_int ret;

    cl_uint num_platforms;
    ret = clGetPlatformIDs(0, nullptr, &num_platforms);
    print_error(ret, true, __LINE__);
    cout << "Number of platforms: " << num_platforms << "\n";

    cl_platform_id *platforms = new cl_platform_id[num_platforms];
    ret = clGetPlatformIDs(num_platforms, platforms, nullptr);
    print_error(ret, true, __LINE__);

    for (unsigned i = 0; i < num_platforms; ++i) {
        cout << "OpenCL platform: #" << i << "\n";

        // dump platform information
        const std::vector<std::tuple<cl_platform_info, std::string, ClTypeEnum>> kClPlatformInfoList = {
            P(CL_PLATFORM_PROFILE, kCharArray),
            P(CL_PLATFORM_VERSION, kCharArray),
            P(CL_PLATFORM_NAME, kCharArray),
            P(CL_PLATFORM_VENDOR, kCharArray),
            P(CL_PLATFORM_EXTENSIONS, kCharArray),
            };

        for (auto info : kClPlatformInfoList) {
            const size_t kBufLen = 100;
            uint64_t buf[kBufLen];
            size_t param_value_size_ret;
            ret = clGetPlatformInfo(platforms[i], std::get<0>(info), kBufLen, buf, &param_value_size_ret);
            print_info(std::get<1>(info), std::get<2>(info), ret, buf, param_value_size_ret);
            }

        cl_uint num_devices;
        ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
        print_error(ret, true, __LINE__);
        cout << "Number of devices: " << num_devices << "\n";

        cl_device_id *devices = new cl_device_id[num_devices];
        ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, devices, nullptr);
        print_error(ret, true, __LINE__);

        for (unsigned j = 0; j < num_devices; ++j) {
            cout << "OpenCL platform/device: #" << i << "/" << j << "\n";

            // dump device information
            const std::vector<std::tuple<cl_device_info, std::string, ClTypeEnum>> cl_device_info_list = {
                P(CL_DEVICE_TYPE, kClDeviceType),
                P(CL_DEVICE_VENDOR_ID, kClUint),
                P(CL_DEVICE_MAX_COMPUTE_UNITS, kClUint),
                P(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, kClUint),
                P(CL_DEVICE_MAX_WORK_ITEM_SIZES, kSizeTArray),
                P(CL_DEVICE_MAX_WORK_GROUP_SIZE, kSizeT),
                P(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, kClUint),
                P(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, kClUint),
                P(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, kClUint),
                P(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, kClUint),
                P(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, kClUint),
                P(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, kClUint),
                P(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, kClUint),
                P(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, kClUint),
                P(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, kClUint),
                P(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, kClUint),
                P(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, kClUint),
                P(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, kClUint),
                P(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, kClUint),
                P(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, kClUint),
                P(CL_DEVICE_MAX_CLOCK_FREQUENCY, kClUint),
                P(CL_DEVICE_ADDRESS_BITS, kClUint),
                P(CL_DEVICE_MAX_MEM_ALLOC_SIZE, kClUlong),
                P(CL_DEVICE_IMAGE_SUPPORT, kClBool),
                P(CL_DEVICE_MAX_READ_IMAGE_ARGS, kClUint),
                P(CL_DEVICE_MAX_WRITE_IMAGE_ARGS, kClUint),
                P(CL_DEVICE_IMAGE2D_MAX_WIDTH, kSizeT),
                P(CL_DEVICE_IMAGE2D_MAX_HEIGHT, kSizeT),
                P(CL_DEVICE_IMAGE3D_MAX_WIDTH, kSizeT),
                P(CL_DEVICE_IMAGE3D_MAX_HEIGHT, kSizeT),
                P(CL_DEVICE_IMAGE3D_MAX_DEPTH, kSizeT),
                P(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, kSizeT),
                P(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, kSizeT),
                P(CL_DEVICE_MAX_SAMPLERS, kClUint),
                P(CL_DEVICE_MAX_PARAMETER_SIZE, kSizeT),
                P(CL_DEVICE_MEM_BASE_ADDR_ALIGN, kClUint),
                P(CL_DEVICE_SINGLE_FP_CONFIG, kClDeviceFpConfig),
                P(CL_DEVICE_DOUBLE_FP_CONFIG, kClDeviceFpConfig),
                P(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, kClDeviceMemCacheType),
                P(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, kClUint),
                P(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, kClUlong),
                P(CL_DEVICE_GLOBAL_MEM_SIZE, kClUlong),
                P(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, kClUlong),
                P(CL_DEVICE_MAX_CONSTANT_ARGS, kClUint),
                P(CL_DEVICE_LOCAL_MEM_TYPE, kClDeviceLocalMemType),
                P(CL_DEVICE_LOCAL_MEM_SIZE, kClUlong),
                P(CL_DEVICE_ERROR_CORRECTION_SUPPORT, kClBool),
                P(CL_DEVICE_HOST_UNIFIED_MEMORY, kClBool),
                P(CL_DEVICE_PROFILING_TIMER_RESOLUTION, kSizeT),
                P(CL_DEVICE_ENDIAN_LITTLE, kClBool),
                P(CL_DEVICE_AVAILABLE, kClBool),
                P(CL_DEVICE_COMPILER_AVAILABLE, kClBool),
                P(CL_DEVICE_LINKER_AVAILABLE, kClBool),
                P(CL_DEVICE_EXECUTION_CAPABILITIES, kClDeviceExecCapabilities),
                P(CL_DEVICE_QUEUE_PROPERTIES, kClCommandQueueProperties),
                P(CL_DEVICE_BUILT_IN_KERNELS, kCharArray),
                P(CL_DEVICE_PLATFORM, kClPlatformId),
                P(CL_DEVICE_NAME, kCharArray),
                P(CL_DEVICE_VENDOR, kCharArray),
                P(CL_DRIVER_VERSION, kCharArray),
                P(CL_DEVICE_PROFILE, kCharArray),
                P(CL_DEVICE_VERSION, kCharArray),
                P(CL_DEVICE_OPENCL_C_VERSION, kCharArray),
                P(CL_DEVICE_EXTENSIONS, kCharArray),
                P(CL_DEVICE_PRINTF_BUFFER_SIZE, kSizeT),
                P(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, kClBool),
                P(CL_DEVICE_PARENT_DEVICE, kClDeviceId),
                P(CL_DEVICE_PARTITION_MAX_SUB_DEVICES, kClUint),
                P(CL_DEVICE_PARTITION_PROPERTIES, kClDevicePartitionPropertyArray),
                P(CL_DEVICE_PARTITION_AFFINITY_DOMAIN, kClDeviceAffinityDomain),
                P(CL_DEVICE_PARTITION_TYPE, kClDevicePartitionPropertyArray),
                P(CL_DEVICE_REFERENCE_COUNT, kClUint),
                };

            for (auto info : cl_device_info_list) {
                const size_t kBufLen = 100;
                uint64_t buf[kBufLen];
                size_t param_value_size_ret;
                ret = clGetDeviceInfo(devices[i], std::get<0>(info), kBufLen, buf, &param_value_size_ret);
                print_info(std::get<1>(info), std::get<2>(info), ret, buf, param_value_size_ret);
                }
            }

        delete [] devices;
        }

    delete [] platforms;

    return 0;
    }
