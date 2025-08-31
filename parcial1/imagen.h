#ifndef IMAGEN_H
#define IMAGEN_H

#include <string>

class Imagen {
protected:
    char* magic;
    int width;
    int height;
    int max_color;
    int* pixels;
    int pixel_count;

public:
    Imagen();
    virtual ~Imagen();
    
    // Métodos virtuales puros
    virtual bool load(const char* filename) = 0;
    virtual bool save(const char* filename) = 0;
    
    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getMaxColor() const { return max_color; }
    int* getPixels() const { return pixels; }
    int getPixelCount() const { return pixel_count; }
    char* getMagic() const { return magic; }
    
    // Setters
    void setWidth(int w) { width = w; }
    void setHeight(int h) { height = h; }
    void setMaxColor(int mc) { max_color = mc; }
    
    // Métodos utilitarios
    virtual void allocatePixels();
    virtual void deallocatePixels();
    bool isValidCoordinate(int x, int y) const;
    int getPixelIndex(int x, int y) const;
};

#endif