#include <iostream>
#include <cuda_runtime.h>
#include "PGMImage.h"


__global__ void hello_cuda() {
    printf("Hello from CUDA!\n");
}

__global__ void demosaic_kernel(uint16_t* input, uint16_t* output, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    int idx = y * width + x;

    uint16_t R, G, B;

    // Use shared memory with padding to reduce bank conflicts
    __shared__ uint16_t sharedMem[16][16 + 1];

    // Load data into shared memory
    sharedMem[threadIdx.y][threadIdx.x] = input[idx];
    __syncthreads();

    // Perform demosaicing (same logic as before)
    // ...

    output[idx * 3 + 0] = R;
    output[idx * 3 + 1] = G;
    output[idx * 3 + 2] = B;
}

void BayerDemosaicCUDA(const std::string& inputFilename, const std::string& outputFilename) {
    try {
        // Load the PGM image using the PGMImage class
        PGMImage inputImage(inputFilename);

        uint16_t width = inputImage.width();
        uint16_t height = inputImage.height();
        uint16_t* raw_data = inputImage.data();

        if (!raw_data) {
            std::cerr << "Failed to read image data" << std::endl;
            return;
        }

        // Allocate device memory
        uint16_t* d_input;
        uint16_t* d_output;
        size_t inputSize = width * height * sizeof(uint16_t);
        size_t outputSize = width * height * 3 * sizeof(uint16_t);
        cudaMalloc(&d_input, inputSize);
        cudaMalloc(&d_output, outputSize);

        // Copy data to device
        cudaMemcpy(d_input, raw_data, inputSize, cudaMemcpyHostToDevice);

        // Define block and grid sizes
        dim3 blockSize(16, 16);
        dim3 gridSize((width + blockSize.x - 1) / blockSize.x, (height + blockSize.y - 1) / blockSize.y);

        // Launch the kernel
        demosaic_kernel << <gridSize, blockSize >> > (d_input, d_output, width, height);
        cudaDeviceSynchronize();

        // Allocate host memory for the output
        uint16_t* output_data = new uint16_t[width * height * 3];

        // Copy the result back to host
        cudaMemcpy(output_data, d_output, outputSize, cudaMemcpyDeviceToHost);

        // Create an output PGM image
        PGMImage outputImage(width, height, 3);

        // Copy the output data to the PGMImage data
        std::memcpy(outputImage.data(), output_data, outputSize);

        // Save the output image
        if (outputImage.Write(outputFilename)) {
            std::cout << "Image written successfully to " << outputFilename << std::endl;
        }
        else {
            std::cerr << "Failed to write image to " << outputFilename << std::endl;
        }

        // Free device memory
        cudaFree(d_input);
        cudaFree(d_output);

        // Free host memory
        delete[] output_data;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
    }
}

int main() {
    hello_cuda<<<1, 1>>>();
    cudaDeviceSynchronize();
    return 0;
}
