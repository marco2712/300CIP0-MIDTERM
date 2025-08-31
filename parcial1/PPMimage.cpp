#include "PPMimage.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>

PPMImage::PPMImage() : Imagen() {
    strcpy(magic, "P3");
}

PPMImage::~PPMImage() {
    // El destructor de la clase base se encarga de la limpieza
}

void PPMImage::allocatePixels() {
    if (pixels) {
        deallocatePixels();
    }
    if (width > 0 && height > 0) {
        pixel_count = width * height * 3; // 3 componentes RGB
        pixels = new int[pixel_count];
    }
}

void PPMImage::skipComments(FILE* file) {
    int c;
    while ((c = fgetc(file)) == '#') {
        while ((c = fgetc(file)) != '\n' && c != EOF);
    }
    ungetc(c, file);
}

bool PPMImage::load(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    // Leer magic number
    if (fscanf(file, "%2s", magic) != 1) {
        std::cerr << "Error: Cannot read magic number" << std::endl;
        fclose(file);
        return false;
    }
    
    // Verificar que sea P3
    if (strcmp(magic, "P3") != 0) {
        std::cerr << "Error: Not a valid PPM file (P3 expected)" << std::endl;
        fclose(file);
        return false;
    }
    
    // Saltar comentarios
    skipComments(file);
    
    // Leer dimensiones
    if (fscanf(file, "%d %d", &width, &height) != 2) {
        std::cerr << "Error: Cannot read image dimensions" << std::endl;
        fclose(file);
        return false;
    }
    
    // Saltar comentarios
    skipComments(file);
    
    // Leer valor máximo
    if (fscanf(file, "%d", &max_color) != 1) {
        std::cerr << "Error: Cannot read max color value" << std::endl;
        fclose(file);
        return false;
    }
    
    // Asignar memoria para píxeles RGB
    allocatePixels();
    
    // Leer píxeles RGB
    for (int i = 0; i < pixel_count; i++) {
        int value;
        if (fscanf(file, "%d", &value) != 1) {
            std::cerr << "Error: Cannot read pixel value at position " << i << std::endl;
            fclose(file);
            return false;
        }
        pixels[i] = value;
    }
    
    fclose(file);
    return true;
}

bool PPMImage::save(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        std::cerr << "Error: Cannot create file " << filename << std::endl;
        return false;
    }
    
    // Escribir header
    fprintf(file, "%s\n%d %d\n%d\n", magic, width, height, max_color);
    
    // Escribir píxeles RGB
    for (int i = 0; i < pixel_count; i++) {
        fprintf(file, "%d\n", pixels[i]);
    }
    
    fclose(file);
    return true;
}

int PPMImage::getRGBIndex(int x, int y, int component) const {
    return (y * width + x) * 3 + component;
}

RGB PPMImage::getRGBValue(int x, int y) const {
    if (!isValidCoordinate(x, y)) {
        return RGB(0, 0, 0);
    }
    
    int base_idx = getRGBIndex(x, y, 0);
    return RGB(pixels[base_idx], pixels[base_idx + 1], pixels[base_idx + 2]);
}

void PPMImage::setRGBValue(int x, int y, const RGB& color) {
    setRGBValue(x, y, color.r, color.g, color.b);
}

void PPMImage::setRGBValue(int x, int y, int r, int g, int b) {
    if (isValidCoordinate(x, y)) {
        r = std::max(0, std::min(max_color, r));
        g = std::max(0, std::min(max_color, g));
        b = std::max(0, std::min(max_color, b));
        
        int base_idx = getRGBIndex(x, y, 0);
        pixels[base_idx] = r;
        pixels[base_idx + 1] = g;
        pixels[base_idx + 2] = b;
    }
}

PPMImage* PPMImage::clone() const {
    PPMImage* copy = new PPMImage();
    copy->width = this->width;
    copy->height = this->height;
    copy->max_color = this->max_color;
    strcpy(copy->magic, this->magic);
    
    copy->allocatePixels();
    for (int i = 0; i < pixel_count; i++) {
        copy->pixels[i] = this->pixels[i];
    }
    
    return copy;
}