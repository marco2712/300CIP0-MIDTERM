#ifndef FILTER_H
#define FILTER_H

#include "imagen.h"
#include "PGMimage.h"
#include "PPMimage.h"

class Filter {
public:
    // Tipos de filtro disponibles
    enum FilterType {
        BLUR,
        LAPLACE,
        SHARPEN
    };

    // Constructor y destructor
    Filter();
    virtual ~Filter();
    
    // Método principal para aplicar filtro
    static bool applyFilter(Imagen* input, Imagen* output, FilterType filter_type);
    
    // Métodos específicos para cada filtro
    static bool applyBlur(Imagen* input, Imagen* output);
    static bool applyLaplace(Imagen* input, Imagen* output);
    static bool applySharpen(Imagen* input, Imagen* output);
    
    // Método para convertir string a FilterType
    static FilterType stringToFilterType(const char* filter_name);
    static const char* filterTypeToString(FilterType filter_type);
    static const float BLUR_KERNEL[3][3];
    static const float LAPLACE_KERNEL[3][3];
    static const float SHARPEN_KERNEL[3][3];

private:
    // Kernels para los filtros

    
    // Métodos auxiliares para aplicar convolución
    static float applyKernel(Imagen* image, int x, int y, const float kernel[3][3], int component = 0);
    static bool applyConvolution(Imagen* input, Imagen* output, const float kernel[3][3]);
    
    // Métodos para PGM y PPM específicamente
    static bool applyConvolutionPGM(PGMImage* input, PGMImage* output, const float kernel[3][3]);
    static bool applyConvolutionPPM(PPMImage* input, PPMImage* output, const float kernel[3][3]);
    
    // Método para clamping de valores
    static int clampValue(int value, int min_val, int max_val);
};

#endif