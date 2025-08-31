#include "imagen.h"
#include <cstdlib>
#include <cstring>

Imagen::Imagen() : magic(nullptr), width(0), height(0), max_color(0), pixels(nullptr), pixel_count(0) {
    magic = new char[3];
}

Imagen::~Imagen() {
    deallocatePixels();
    if (magic) {
        delete[] magic;
    }
}

void Imagen::allocatePixels() {
    if (pixels) {
        deallocatePixels();
    }
    if (pixel_count > 0) {
        pixels = new int[pixel_count];
    }
}

void Imagen::deallocatePixels() {
    if (pixels) {
        delete[] pixels;
        pixels = nullptr;
    }
}

bool Imagen::isValidCoordinate(int x, int y) const {
    return (x >= 0 && x < width && y >= 0 && y < height);
}

int Imagen::getPixelIndex(int x, int y) const {
    return y * width + x;
}