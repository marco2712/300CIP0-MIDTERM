#include "PGMimage.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>

PGMImage::PGMImage() : Imagen() {
    strcpy(magic, "P2");
}

PGMImage::~PGMImage() {
    // El destructor de la clase base se encarga de la limpieza
}

void PGMImage::skipComments(FILE* file) {
    int c;
    while ((c = fgetc(file)) == '#') {
        while ((c = fgetc(file)) != '\n' && c != EOF);
    }
    ungetc(c, file);
}

bool PGMImage::load(const char* filename) {
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
    
    // Verificar que sea P2
    if (strcmp(magic, "P2") != 0) {
        std::cerr << "Error: Not a valid PGM file (P2 expected)" << std::endl;
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
    
    // Calcular número de píxeles
    pixel_count = width * height;
    allocatePixels();
    
    // Leer píxeles
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

bool PGMImage::save(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        std::cerr << "Error: Cannot create file " << filename << std::endl;
        return false;
    }
    
    // Escribir header
    fprintf(file, "%s\n%d %d\n%d\n", magic, width, height, max_color);
    
    // Escribir píxeles
    for (int i = 0; i < pixel_count; i++) {
        fprintf(file, "%d\n", pixels[i]);
    }
    
    fclose(file);
    return true;
}

int PGMImage::getGrayValue(int x, int y) const {
    if (!isValidCoordinate(x, y)) {
        return 0;
    }
    return pixels[getPixelIndex(x, y)];
}

void PGMImage::setGrayValue(int x, int y, int value) {
    if (isValidCoordinate(x, y)) {
        value = std::max(0, std::min(max_color, value));
        pixels[getPixelIndex(x, y)] = value;
    }
}

PGMImage* PGMImage::clone() const {
    PGMImage* copy = new PGMImage();
    copy->width = this->width;
    copy->height = this->height;
    copy->max_color = this->max_color;
    copy->pixel_count = this->pixel_count;
    strcpy(copy->magic, this->magic);
    
    copy->allocatePixels();
    for (int i = 0; i < pixel_count; i++) {
        copy->pixels[i] = this->pixels[i];
    }
    
    return copy;
}