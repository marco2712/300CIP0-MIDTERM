#ifndef PPMIMAGE_H
#define PPMIMAGE_H

#include "imagen.h"

struct RGB {
    int r, g, b;
    RGB(int red = 0, int green = 0, int blue = 0) : r(red), g(green), b(blue) {}
};

class PPMImage : public Imagen {
public:
    PPMImage();
    ~PPMImage();
    
    // Implementación de métodos virtuales
    bool load(const char* filename) override;
    bool save(const char* filename) override;
    
    // Métodos específicos para PPM
    RGB getRGBValue(int x, int y) const;
    void setRGBValue(int x, int y, const RGB& color);
    void setRGBValue(int x, int y, int r, int g, int b);
    
    // Método para clonar
    PPMImage* clone() const;
    
    // Override de allocatePixels para PPM (3 valores por pixel)
    void allocatePixels() override;
    
private:
    void skipComments(FILE* file);
    int getRGBIndex(int x, int y, int component) const; // 0=R, 1=G, 2=B
};

#endif