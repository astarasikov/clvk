// Copyright (c) 2009-2011 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#ifndef __linux__
#include "stdafx.h"
#else
#include <string.h>
#endif

#include <iostream>
#include "basic.hpp"
#include "cmdparser.hpp"
#include "oclobject.hpp"

#include <android/log.h>
#define  LOG_TAG    "clvk-test"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define printf LOGE
#define fputs(str, fd) LOGE("%s", str)
#define fprintf(fd, ...) LOGE(__VA_ARGS__)

using namespace std;


// generate random data for input array
void generateInput(cl_int* p_input, size_t arraySize)
{
    const size_t minElement = 0;
    const size_t maxElement = arraySize + 1;

    srand(12345);

    // random initialization of input
    for (size_t i = 0; i < arraySize; ++i)
    {
        p_input[i] = 9999 - i;//(cl_int)((float) (maxElement - minElement) * (rand() / (float) RAND_MAX));
    }
}

// reference sort implementation
void ExecuteSortReference(cl_int* p_input, cl_int arraySize, cl_bool sortAscending)
{
    cl_int numStages = 0;
    cl_uint temp;

    cl_int stage;
    cl_int passOfStage;

    for (temp = arraySize; temp > 1; temp >>= 1)
    {
        ++numStages;
    }
	//numStages = log2(arraySize)

    for (stage = 0; stage < numStages; ++stage)
    {
        int dirMask = 1 << stage;
		//dirMask = floor_to_power_of_two(idx)
		//1, 2, 4, 8

        for (passOfStage = 0; passOfStage < stage + 1; ++passOfStage)
        {
			//within current "stage"

            const cl_uint shift = stage - passOfStage; //[stage ... 0]
            const cl_uint distance = 1 << shift;
            const cl_uint lmask = distance - 1; //xfff => *11111

            for(int g_id=0; g_id < arraySize >> 1; g_id++)
            {
                cl_uint leftId = (( g_id >> shift ) << (shift + 1)) + (g_id & lmask);
                cl_uint rightId = leftId + distance;

                cl_uint left  = p_input[leftId];
                cl_uint right = p_input[rightId];

                cl_uint greater;
                cl_uint lesser;

                if(left > right)
                {
                    greater = left;
                    lesser  = right;
                }
                else
                {
                    greater = right;
                    lesser  = left;
                }

                cl_bool dir = sortAscending;
                if( ( g_id & dirMask) == dirMask )
                    dir = !dir;

                if(dir)
                {
                    p_input[leftId]  = lesser;
                    p_input[rightId] = greater;
                }
                else
                {
                    p_input[leftId]  = greater;
                    p_input[rightId] = lesser;
                }
            }
        }
    }
}

// OpenCL sort implementation
float ExecuteSortKernel(OpenCLBasic &oclobjects, OpenCLProgramOneKernel &executable, cl_int* p_input, cl_int arraySize, cl_uint sortAscending)
{
    double   perf_start;
    double   perf_stop;

    cl_int err = CL_SUCCESS;
    cl_int numStages = 0;
    cl_uint temp;

    cl_int stage;
    cl_int passOfStage;

    // create OpenCL buffer using input array memory
    cl_mem cl_input_buffer =
        clCreateBuffer
        (
            oclobjects.context,
            CL_MEM_USE_HOST_PTR,
            zeroCopySizeAlignment(sizeof(cl_int) * arraySize, oclobjects.device),
            p_input,
            &err
        );
    SAMPLE_CHECK_ERRORS(err);


    if (cl_input_buffer == (cl_mem)0)
    {
        throw Error("Failed to create input data Buffer\n");
    }

    for (temp = arraySize; temp > 2; temp >>= 1)
    {
        numStages++;
    }

    err=  clSetKernelArg(executable.kernel, 0, sizeof(cl_mem), (void *) &cl_input_buffer);
    SAMPLE_CHECK_ERRORS(err);

    err = clSetKernelArg(executable.kernel, 3, sizeof(cl_uint), (void *) &sortAscending);
    SAMPLE_CHECK_ERRORS(err);

    perf_start=time_stamp();
    for (stage = 0; stage < numStages; stage++)
    {
        err = clSetKernelArg(executable.kernel, 1, sizeof(cl_uint), (void *) &stage);
        SAMPLE_CHECK_ERRORS(err);

        for (passOfStage = stage; passOfStage >= 0; passOfStage--)
        {
            err = clSetKernelArg(executable.kernel, 2, sizeof(cl_uint), (void *) &passOfStage);
            SAMPLE_CHECK_ERRORS(err);

            // set work-item dimensions
            size_t gsz = arraySize / (2*4);
            size_t global_work_size[1] = { passOfStage ? gsz : gsz << 1 };    //number of quad items in input array

            // execute kernel
            err = clEnqueueNDRangeKernel(oclobjects.queue, executable.kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
            SAMPLE_CHECK_ERRORS(err);
        }
    }

    err = clFinish(oclobjects.queue);

    SAMPLE_CHECK_ERRORS(err);

    perf_stop=time_stamp();

    void* tmp_ptr = NULL;

    tmp_ptr = clEnqueueMapBuffer(oclobjects.queue, cl_input_buffer, true, CL_MAP_READ, 0, sizeof(cl_int) * arraySize , 0, NULL, NULL, &err);
    SAMPLE_CHECK_ERRORS(err);

    if(tmp_ptr!=p_input)
    {
        throw Error("clEnqueueMapBuffer failed to return original pointer\n");
    }

    err = clFinish(oclobjects.queue);
    SAMPLE_CHECK_ERRORS(err);

    err = clEnqueueUnmapMemObject(oclobjects.queue, cl_input_buffer, tmp_ptr, 0, NULL, NULL);
    SAMPLE_CHECK_ERRORS(err);

    err = clReleaseMemObject(cl_input_buffer);
    SAMPLE_CHECK_ERRORS(err);

    return (float)(perf_stop - perf_start);
}

extern "C" int intel_BitonicSort_main (int argc, const char** argv)
{
    int ret = EXIT_SUCCESS;
    cl_int* p_input = NULL;
    cl_int* p_ref = NULL;
    try
    {
        // Define and parse command-line arguments.
        //printf("%s:%d\n", __func__, __LINE__);
        CmdParserCommon cmdparser(argc, argv);
        //printf("%s:%d\n", __func__, __LINE__);

        CmdOption<bool> reverse_sort(cmdparser,0,"reverse-sort","","performs descending sort (default is ascending).",false);
        //printf("%s:%d\n", __func__, __LINE__);

        CmdOption<int> array_size(cmdparser,'s',"size","<integer>","input/output array size.",(1 << 10));
        cmdparser.parse();

        // Immediatly exit if user wanted to see the usage information only.
        if(cmdparser.help.isSet())
        {
            puts("help argument is set. exiting");
            return EXIT_SUCCESS;
        }

        // validate user input parameters, if any
        {
            size_t size = array_size.getValue();
            cl_uint sz_log_b = 0;
            if( size < 1024 )
            {
                puts("size < 1024; exiting\n");
                throw Error("Input size should be no less than 1024!\n");
            }
            if((size&(size-1)) || (size&3))
            {
                puts("Input size should be (2^N)*4! exiting\n");
                throw Error("Input size should be (2^N)*4!\n");
            }
        }
        //printf("%s:%d\n", __func__, __LINE__);

        // Create the necessary OpenCL objects up to device queue.
        OpenCLBasic oclobjects(
            cmdparser.platform.getValue(),
            cmdparser.device_type.getValue(),
            cmdparser.device.getValue()
        );
        //printf("%s:%d\n", __func__, __LINE__);

        // Build kernel
        OpenCLProgramOneKernel executable(oclobjects,L"/data/data/io.github.astarasikov.clvkandroid.clvktests/BitonicSort.cl","","BitonicSort");
        //printf("%s:%d\n", __func__, __LINE__);


        printf("Sort order is %s\n", reverse_sort.getValue() ?  "descending" : "ascending" );
        printf("Input size is %d items\n", array_size.getValue());

        cl_uint dev_alignment = zeroCopyPtrAlignment(oclobjects.device);
        size_t aligned_size = zeroCopySizeAlignment(sizeof(cl_int) * array_size.getValue(), oclobjects.device);
        printf("OpenCL data alignment is %d bytes.\n", dev_alignment);
        p_input = (cl_int*)aligned_malloc(aligned_size, dev_alignment);
        p_ref = (cl_int*)aligned_malloc(aligned_size, dev_alignment);
        if(!(p_input && p_ref))
        {
            throw Error("Could not allocate buffers on the HOST!");
        }

        // random input
        generateInput(p_input, array_size.getValue());
        memcpy(p_ref, p_input, sizeof(cl_int) * array_size.getValue());

        // sort input array in a given direction
        printf("Executing OpenCL kernel...\n");
        float ocl_time = ExecuteSortKernel(oclobjects, executable, p_input, array_size.getValue(), !reverse_sort.getValue());
        //printf("%s:%d\n", __func__, __LINE__);

        printf("Executing reference...\n");
        ExecuteSortReference(p_ref, array_size.getValue(), !reverse_sort.getValue());
        //printf("%s:%d\n", __func__, __LINE__);

        bool result = true;
        printf("Performing verification...\n");
        for (cl_int i = 0; i < array_size.getValue(); i++)
        {
            if (p_input[i] != p_ref[i])
            {
                printf("[%d]: input %d != ref %d\n", i, p_input[i], p_ref[i]);
                result = false;
                break;
            }
        }

		for (cl_int i = 0; i < array_size.getValue() && i < 32; i++)
		{
			//printf("%d \n", p_input[i]);
		}

        if(!result)
        {
            printf("ERROR: Verification failed.\n");
            ret = EXIT_FAILURE;
        }
        else
        {
            printf("Verification succeeded.\n");
        }
        printf("NDRange perf. counter time %f ms.\n", 1000.0f*ocl_time);
    }
    catch(const CmdParser::Error& error)
    {
        cerr
            << "[ ERROR ] In command line: " << error.what() << "\n"
            << "Run " << argv[0] << " -h for usage info.\n";
        puts(error.what());
        ret = EXIT_FAILURE;
    }
    catch(const Error& error)
    {
        cerr << "[ ERROR ] Sample application specific error: " << error.what() << "\n";
        puts(error.what());
        ret = EXIT_FAILURE;
    }
    catch(const exception& error)
    {
        cerr << "[ ERROR ] " << error.what() << "\n";
        puts(error.what());
        ret = EXIT_FAILURE;
    }
    catch(...)
    {
        cerr << "[ ERROR ] Unknown/internal error happened.\n";
        puts("[ ERROR ] Unknown/internal error happened.");
        ret = EXIT_FAILURE;
    }

    aligned_free( p_ref );
    aligned_free( p_input );
    return ret;
}

