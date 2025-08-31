#include "filter.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>

// Definición de kernels
const float Filter::BLUR_KERNEL[3][3] = {
    {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
    {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
    {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f}
};

const float Filter::LAPLACE_KERNEL[3][3] = {
    {0.0f, -1.0f, 0.0f},
    {-1.0f, 4.0f, -1.0f},
    {0.0f, -1.0f, 0.0f}
};

const float Filter::SHARPEN_KERNEL[3][3] = {
    {0.0f, -1.0f, 0.0f},
    {-1.0f, 5.0f, -1.0f},
    {0.0f, -1.0f, 0.0f}
};

Filter::Filter() {
    // Constructor vacío
}

Filter::~Filter() {
    // Destructor vacío
}

bool Filter::applyFilter(Imagen* input, Imagen* output, FilterType filter_type) {
    if (!input || !output) {
        std::cerr << "Error: Input or output image is null" << std::endl;
        return false;
    }
    
    switch (filter_type) {
        case BLUR:
            return applyBlur(input, output);
        case LAPLACE:
            return applyLaplace(input, output);
        case SHARPEN:
            return applySharpen(input, output);
        default:
            std::cerr << "Error: Unknown filter type" << std::endl;
            return false;
    }
}

bool Filter::applyBlur(Imagen* input, Imagen* output) {
    return applyConvolution(input, output, BLUR_KERNEL);
}

bool Filter::applyLaplace(Imagen* input, Imagen* output) {
    return applyConvolution(input, output, LAPLACE_KERNEL);
}

bool Filter::applySharpen(Imagen* input, Imagen* output) {
    return applyConvolution(input, output, SHARPEN_KERNEL);
}

bool Filter::applyConvolution(Imagen* input, Imagen* output, const float kernel[3][3]) {
    if (!input || !output) {
        return false;
    }
    
    // Verificar si es PGM o PPM
    if (strcmp(input->getMagic(), "P2") == 0) {
        // Es PGM
        PGMImage* pgm_input = dynamic_cast<PGMImage*>(input);
        PGMImage* pgm_output = dynamic_cast<PGMImage*>(output);
        
        if (!pgm_input || !pgm_output) {
            std::cerr << "Error: Failed to cast to PGMImage" << std::endl;
            return false;
        }
        
        return applyConvolutionPGM(pgm_input, pgm_output, kernel);
    }
    else if (strcmp(input->getMagic(), "P3") == 0) {
        // Es PPM
        PPMImage* ppm_input = dynamic_cast<PPMImage*>(input);
        PPMImage* ppm_output = dynamic_cast<PPMImage*>(output);
        
        if (!ppm_input || !ppm_output) {
            std::cerr << "Error: Failed to cast to PPMImage" << std::endl;
            return false;
        }
        
        return applyConvolutionPPM(ppm_input, ppm_output, kernel);
    }
    
    return false;
}

bool Filter::applyConvolutionPGM(PGMImage* input, PGMImage* output, const float kernel[3][3]) {
    int width = input->getWidth();
    int height = input->getHeight();
    
    // Configurar la imagen de salida
    output->setWidth(width);
    output->setHeight(height);
    output->setMaxColor(input->getMaxColor());
    output->allocatePixels();
    
    // Aplicar convolución
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum = 0.0f;
            
            // Aplicar kernel 3x3
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int nx = x + kx;
                    int ny = y + ky;
                    
                    if (input->isValidCoordinate(nx, ny)) {
                        int pixel_value = input->getGrayValue(nx, ny);
                        sum += pixel_value * kernel[ky + 1][kx + 1];
                    }
                }
            }
            
            // Clamping del resultado
            int result = clampValue(static_cast<int>(sum), 0, input->getMaxColor());
            output->setGrayValue(x, y, result);
        }
    }
    
    return true;
}

bool Filter::applyConvolutionPPM(PPMImage* input, PPMImage* output, const float kernel[3][3]) {
    int width = input->getWidth();
    int height = input->getHeight();
    
    // Configurar la imagen de salida
    output->setWidth(width);
    output->setHeight(height);
    output->setMaxColor(input->getMaxColor());
    output->allocatePixels();
    
    // Aplicar convolución para cada canal RGB
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f;
            
            // Aplicar kernel 3x3
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int nx = x + kx;
                    int ny = y + ky;
                    
                    if (input->isValidCoordinate(nx, ny)) {
                        RGB pixel_value = input->getRGBValue(nx, ny);
                        float kernel_val = kernel[ky + 1][kx + 1];
                        sum_r += pixel_value.r * kernel_val;
                        sum_g += pixel_value.g * kernel_val;
                        sum_b += pixel_value.b * kernel_val;
                    }
                }
            }
            
            // Clamping del resultado
            int result_r = clampValue(static_cast<int>(sum_r), 0, input->getMaxColor());
            int result_g = clampValue(static_cast<int>(sum_g), 0, input->getMaxColor());
            int result_b = clampValue(static_cast<int>(sum_b), 0, input->getMaxColor());
            
            output->setRGBValue(x, y, result_r, result_g, result_b);
        }
    }
    
    return true;
}

Filter::FilterType Filter::stringToFilterType(const char* filter_name) {
    if (strcmp(filter_name, "blur") == 0) {
        return BLUR;
    } else if (strcmp(filter_name, "laplace") == 0) {
        return LAPLACE;
    } else if (strcmp(filter_name, "sharpen") == 0) {
        return SHARPEN;
    }
    
    // Por defecto retornar BLUR
    return BLUR;
}

const char* Filter::filterTypeToString(FilterType filter_type) {
    switch (filter_type) {
        case BLUR:
            return "blur";
        case LAPLACE:
            return "laplace";
        case SHARPEN:
            return "sharpen";
        default:
            return "unknown";
    }
}

int Filter::clampValue(int value, int min_val, int max_val) {
    return std::max(min_val, std::min(max_val, value));
}