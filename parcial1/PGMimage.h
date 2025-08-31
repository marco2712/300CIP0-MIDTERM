#ifndef PGMIMAGE_H
#define PGMIMAGE_H

#include "imagen.h"

class PGMImage : public Imagen {
public:
    PGMImage();
    ~PGMImage();
    
    // Implementación de métodos virtuales
    bool load(const char* filename) override;
    bool save(const char* filename) override;
    
    // Métodos específicos para PGM
    int getGrayValue(int x, int y) const;
    void setGrayValue(int x, int y, int value);
    
    // Método para clonar
    PGMImage* clone() const;
    
private:
    void skipComments(FILE* file);
};

#endif