#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
//#include <libcamera/libcamera.h>
//#include <libcamera/framebuffer_allocator.h>

#include "Halide.h"
#include "halide_image_io.h"
#include "tiffio.h"

#include <stdio.h>
#include <stdlib.h>
//custom tiff reader from Robs code
#include "TiffSrcFile.h"
#include <sstream> 

#include <vector>
#include<iostream>
#include <chrono>
#include<cmath>
#include<cstdint>
#include "PGMImage.h"

using namespace Halide;
using namespace Halide::Tools;
using namespace std;

//using namespace Halide::Runtime;


template <typename Func, typename... Args>
double timeFunction(Func func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    return duration.count();
}

void simpleBufferCopy(const std::vector<uint16_t>& image, int width, int height) {
    // Initialize Halide buffers
    Buffer<uint16_t> inBuf(width, height);
    Buffer<uint16_t> outBuf(width, height);

    // Copy the image data to the input buffer
    memcpy(inBuf.data(), image.data(), image.size() * sizeof(uint16_t));

    // For demonstration, let's just copy the input buffer to the output buffer
    outBuf.copy_from(inBuf);

    // Output the processed image (for demonstration purposes)
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            std::cout << outBuf(i, j) << " ";
        }
        std::cout << std::endl;
    }
}

void processImage(const std::vector<uint16_t>& image, int width, int height, int radius) {
    // Create an input and output buffer that will be used for every iteration of the loop.
    Buffer<uint16_t> inBuf(width, height);
    Buffer<uint16_t> outBuf(width, height);

    // Copy the image data to the input buffer
    memcpy(inBuf.data(), image.data(), image.size() * sizeof(uint16_t));

    Var x("x"), y("y");

    // Define the boundary condition to set pixels outside the image to 0
    Func clamped = BoundaryConditions::constant_exterior(inBuf, 0);

    // Compute the clamped coordinates for the averaging and use them to compute the area of the averaging region.
    Expr clamped_minX = clamp(x - radius, 0, inBuf.width() - 1);
    Expr clamped_maxX = clamp(x + radius, 0, inBuf.width() - 1);
    Expr clamped_minY = clamp(y - radius, 0, inBuf.height() - 1);
    Expr clamped_maxY = clamp(y + radius, 0, inBuf.height() - 1);
    Expr area = (clamped_maxX - clamped_minX + 1) * (clamped_maxY - clamped_minY + 1);
    Func avg("avg");

    try {
        RDom d(-radius, 2 * radius + 1, -radius, 2 * radius + 1);
        avg(x, y) = cast<uint16_t>(0); ///< Needed because you can't use a domain in a pure function definition
        avg(x, y) += cast<uint16_t>(clamped(x + d.x, y + d.y) / area);
        avg.update(0).unscheduled(); ///< This squashes a JIT compiler warning about the update being scheduled
    }
    catch (Halide::CompileError& e) {
        std::cerr << "Compile error when defining Halide code: " << e.what() << std::endl;
        return;
    }

    // Schedule the function for CPU execution
    avg.parallel(y).vectorize(x, 8);

    // Realize the function
    avg.realize(outBuf);

    // Copy the output buffer back to the host
    outBuf.copy_to_host();

    // Output the processed image (for demonstration purposes, you can save it or use it as needed)
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            std::cout << outBuf(i, j) << " ";
        }
        std::cout << std::endl;
    }
}

void demosaicHalide(Buffer<uint16_t> input, Buffer<uint16_t>& output) {
    Var x("x"), y("y");
    Func demosaic("demosaic");

    // Apply boundary conditions
    Func clamped = BoundaryConditions::repeat_edge(input);

    // Simple bilinear interpolation demosaicing
    demosaic(x, y) = (clamped(x, y) + clamped(x + 1, y) + clamped(x, y + 1) + clamped(x + 1, y + 1)) / 4;

    // Realize the function
    output = demosaic.realize({ input.width(), input.height() });
}

Buffer<uint16_t> convertToHalideBuffer(std::unique_ptr<uint16_t[]>& bufImg, int width, int height) {
    Buffer<uint16_t> halideBuffer(bufImg.get(), width, height);
    return halideBuffer;
}


void demosaicImage(const char* input_filename, const char* output_filename) {
    try {
        // Read the input PGM image
        TiffSrcFile inputImage;
        std::string  myFilename = "C:\\ws\\sc_arch\\speedtests\\out\\build\\x64-debug\\LowerLeftQuadrant.tiff";
        if (inputImage.OpenFile(myFilename.c_str() ) != 0) {
            fprintf(stderr, "Failed to open TIFF file: %s\n", input_filename);
            return;
        }
        printf("Opened TIFF file: %s\n", input_filename);

        std::unique_ptr<uint16_t[]> bufImg;
        if (inputImage.ReadMonochrome(bufImg) != 0) {
            fprintf(stderr, "Failed to read image data\n");
            inputImage.CloseFile();
            return;
        }
        
        printf("Read image data successfully\n");

        // Create an output buffer
#if 1

    // The last lesson was quite involved, and scheduling complex
    // multi-stage pipelines is ahead of us. As an interlude, let's
    // consider something easy: evaluating funcs over rectangular
    // domains that do not start at the origin.

    // We define our familiar gradient function.
        Func gradient("gradient");
        Var x("x"), y("y");
        gradient(x, y) = x + y;

        // And turn on tracing so we can see how it is being evaluated.
        gradient.trace_stores();

//        Buffer<uint16_t> halideBuffer( bufImg.get(), inputImage.getWidth(), inputImage.getHeight() );
        int nWidth = inputImage.getWidth();
        int nHeight = inputImage.getHeight();
        uint8_t         myStuff[100];

        nWidth = 512;
        nHeight = 256;
//        Halide::Buffer<uint8_t>  haldBuffer( nWidth, nHeight );
        Buffer<int> result(8, 8);

#else
        Buffer<uint16_t> halideBuffer = convertToHalideBuffer(bufImg, inputImage.getWidth(), inputImage.getHeight());
#endif // 1
        Buffer<uint16_t> outBuffer(inputImage.getWidth(), inputImage.getHeight());
        // Call the demosaicing function
        try {
            //demosaicHalide(halideBuffer, outBuffer);
        }
        catch (Halide::RuntimeError& e) {
            std::cerr << "Exception when running Halide code: " << e.what() << std::endl;
            return;
        }
        outBuffer.copy_to_host();
        printf("Realized Halide function\n");

        // Create and write the output TIFF image
        TIFF* out = TIFFOpen(output_filename, "w");
        if (!out) {
            fprintf(stderr, "Failed to open output TIFF file: %s\n", output_filename);
            return;
        }
        printf("Opened output TIFF file: %s\n", output_filename);

        TIFFSetField(out, TIFFTAG_IMAGEWIDTH, outBuffer.width());
        TIFFSetField(out, TIFFTAG_IMAGELENGTH, outBuffer.height());
        TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);
        TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

        for (uint32_t row = 0; row < outBuffer.height(); row++) {
            if (TIFFWriteScanline(out, outBuffer.data() + row * outBuffer.width(), row, 0) < 0) {
                fprintf(stderr, "Failed to write TIFF scanline\n");
                TIFFClose(out);
                return;
            }
        }

        TIFFClose(out);
        printf("Image written successfully to %s\n", output_filename);
    }
    catch (const std::exception& e) {
        fprintf(stderr, "Exception: %s\n", e.what());
    }
    catch (...) {
        fprintf(stderr, "Unknown error occurred\n");
    }
}

void brightHalide(const std::string& filename, int factor) {
    //Halide::Buffer<uint8_t> input = load_image("Madonna.jpg");
    Halide::Buffer<uint8_t> input;
    try {
        input = Halide::Tools::load_image(filename);
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }


    Halide::Func brighter;
    Halide::Var x, y, c;

    Halide::Expr value = input(x, y, c);

    value = value + factor;
    value = Halide::min(value, 255.0f);

    value = Halide::cast<uint8_t>(value);

    brighter(x, y, c) = value;
    Halide::Buffer<uint8_t> output =
        brighter.realize({ input.width(), input.height(), input.channels() });

    save_image(output, "brighterHalide.png");
};


float calculateKernelVariance(Buffer<uint8_t> input, int x, int y, int kernel_size) {
    int half_kernel = kernel_size / 2;
    RDom r(-half_kernel, kernel_size, -half_kernel, kernel_size);

    // Define a Func to compute the mean
    Func mean_func;
    mean_func() = sum(cast<float>(input(x + r.x, y + r.y))) / (kernel_size * kernel_size);

    // Realize the mean
    Buffer<float> mean_buf = mean_func.realize();
    float mean = mean_buf();

    // Define a Func to compute the variance
    Func variance_func;
    variance_func() = sum(pow(cast<float>(input(x + r.x, y + r.y)) - mean, 2)) / (kernel_size * kernel_size);

    // Realize the variance
    Buffer<float> variance_buf = variance_func.realize();
    return variance_buf();
}

void medianFilter(const std::string& filename, int kernel_size, float variance_threshold) {
    try {
        // Load the input image
        Buffer<uint8_t> input = Tools::load_image(filename);

        // Define the algorithm
        Var x, y, c;
        Func clamped = BoundaryConditions::repeat_edge(input);
        Func median;

        // Define a reduction domain for the specified kernel size
        int half_kernel = kernel_size / 2;
        RDom r(-half_kernel, kernel_size, -half_kernel, kernel_size);

        // Calculate the mean and variance using reduction domains
        Func mean_func, variance_func;
        mean_func(x, y, c) = sum(cast<float>(clamped(x + r.x, y + r.y, c))) / (kernel_size * kernel_size);
        Expr mean = mean_func(x, y, c);

        variance_func(x, y, c) = sum(pow(cast<float>(clamped(x + r.x, y + r.y, c)) - mean, 2)) / (kernel_size * kernel_size);
        Expr variance = variance_func(x, y, c);

        // Collect the values in the neighborhood
        std::vector<Expr> values;
        for (int i = 0; i < kernel_size * kernel_size; i++) {
            values.push_back(clamped(x + r.x, y + r.y, c));
        }

        // Apply the median filter if the variance is below the threshold
        median(x, y, c) = select(variance < variance_threshold, cast<uint8_t>(values[values.size() / 2]), clamped(x, y, c));

        // Schedule the algorithm
        median.vectorize(x, 16).parallel(y);

        // Create an output buffer
        Buffer<uint8_t> output(input.width(), input.height(), input.channels());

        // Realize the algorithm into the output buffer
        median.realize(output);

        // Save the output
        Tools::save_image(output, "bay_clean2.png");
    }
    catch (const Halide::CompileError& e) {
        std::cerr << "Halide Compile Error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard Exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown Exception occurred." << std::endl;
    }
}

//bayer Demosaic 
void BayerDemosaicHalide(const std::string& inputFilename, const std::string& outputFilename) {
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

        // Convert the loaded image data to a Halide Buffer
        Buffer<uint16_t> input(raw_data, { width, height });

        // Define the Halide variables
        Var x("x"), y("y"), c("c");

        // Defin  e the Halide function
        Func demosaic("demosaic");

        // Define the Bayer pattern
        Expr R = input(x, y);
        Expr G = input(x, y);
        Expr B = input(x, y);

        // Apply the Bayer pattern
        R = select((x % 2 == 0) && (y % 2 == 0), input(x, y),
            (x % 2 == 1) && (y % 2 == 0), (input(x - 1, y) + input(x + 1, y)) / 2,
            (x % 2 == 0) && (y % 2 == 1), (input(x, y - 1) + input(x, y + 1)) / 2,
            (input(x - 1, y - 1) + input(x + 1, y - 1) + input(x - 1, y + 1) + input(x + 1, y + 1)) / 4);

        G = select((x % 2 == 1) && (y % 2 == 1), input(x, y),
            (x % 2 == 0) && (y % 2 == 1), (input(x - 1, y) + input(x + 1, y)) / 2,
            (x % 2 == 1) && (y % 2 == 0), (input(x, y - 1) + input(x, y + 1)) / 2,
            (input(x - 1, y - 1) + input(x + 1, y - 1) + input(x - 1, y + 1) + input(x + 1, y + 1)) / 4);

        B = select((x % 2 == 1) && (y % 2 == 1), input(x, y),
            (x % 2 == 0) && (y % 2 == 1), (input(x - 1, y) + input(x + 1, y)) / 2,
            (x % 2 == 1) && (y % 2 == 0), (input(x, y - 1) + input(x, y + 1)) / 2,
            (input(x - 1, y - 1) + input(x + 1, y - 1) + input(x - 1, y + 1) + input(x + 1, y + 1)) / 4);

        // Combine the channels
        demosaic(x, y, c) = select(c == 0, R,
            c == 1, G,
            B);

        // Realize the function
        Buffer<uint16_t> output = demosaic.realize({ width, height, 3 });

        // Create an output PGM image
        PGMImage outputImage(width, height);

        // Copy the Halide buffer data to the PGMImage data
        std::memcpy(outputImage.data(), output.data(), width * height * sizeof(uint16_t));

        // Save the output image
        if (outputImage.Write(outputFilename)) {
            std::cout << "Image written successfully to " << outputFilename << std::endl;
        }
        else {
            std::cerr << "Failed to write image to " << outputFilename << std::endl;
        }
    }
    catch (const Halide::Error& e) {
        std::cerr << "Halide error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
    }
}


int main() 
{
    printf("Starting main\n");
    //TiffSrcFile inputImage;
    //inputImage.ReadMonochrome();
    //double medianFilterTime = timeFunction(medianFilter, "bay_dust.jpg", 3, 80); 
    //loadTiff("LowerLeftQuadrant.tiff");
    demosaicImage("UpperLeftQuadrant.tiff", "LLQ.tiff");
    //double halideDemosaicTime = timeFunction(BayerDemosaicHalide, "LowerLeftQuadrant.tiff", "test1.png"); //demosaic_image
    //double bayerMosaicTime = timeFunction(demosaicImage, "LowerLeftQuadrant.tiff", "test.tiff");
    //std::cout << "Halide execution time: " << halideDemosaicTime << " seconds" << std::endl;
    //std::cout << "Halide execution time: " << bayerMosaicTime << " seconds" << std::endl;

    //processImage("bay_dust.jpg", 3, .01);
    return 0;
}
