#include <iostream>
#include <cstring>
#include <mpi.h>
#include "imagen.h"
#include "PGMimage.h"
#include "PPMimage.h"
#include "filter.h"
#include "timer.h"

Imagen* createImageFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return nullptr;
    }
    
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1) {
        fclose(file);
        return nullptr;
    }
    fclose(file);
    
    Imagen* image = nullptr;
    if (strcmp(magic, "P2") == 0) {
        image = new PGMImage();
    } else if (strcmp(magic, "P3") == 0) {
        image = new PPMImage();
    }
    
    if (image && image->load(filename)) {
        return image;
    }
    
    delete image;
    return nullptr;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc < 5) {
        if (rank == 0) {
            std::cout << "Usage: mpirun -np N " << argv[0] << " input output --f filter" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = argv[2];
    const char* filter_name = argv[4]; // Asumiendo --f filter
    
    if (rank == 0) {
        std::cout << "=== Simple MPI Image Processor ===" << std::endl;
        std::cout << "Processes: " << size << std::endl;
        std::cout << "Input: " << input_file << std::endl;
        std::cout << "Output: " << output_file << std::endl;
        std::cout << "Filter: " << filter_name << std::endl;
    }
    
    // Cada proceso carga la imagen completa (approach simple)
    Imagen* input_image = createImageFromFile(input_file);
    if (!input_image) {
        if (rank == 0) {
            std::cerr << "Error: Cannot load image " << input_file << std::endl;
        }
        MPI_Finalize();
        return 1;
    }
    
    // Crear imagen de salida
    Imagen* output_image = nullptr;
    if (strcmp(input_image->getMagic(), "P2") == 0) {
        PGMImage* pgm_input = dynamic_cast<PGMImage*>(input_image);
        output_image = pgm_input->clone();
    } else if (strcmp(input_image->getMagic(), "P3") == 0) {
        PPMImage* ppm_input = dynamic_cast<PPMImage*>(input_image);
        output_image = ppm_input->clone();
    }
    
    if (!output_image) {
        std::cerr << "Process " << rank << ": Failed to create output image" << std::endl;
        delete input_image;
        MPI_Finalize();
        return 1;
    }
    
    if (rank == 0) {
        std::cout << "Image loaded: " << input_image->getWidth() << "x" 
                  << input_image->getHeight() << std::endl;
    }
    
    // Aplicar filtro usando las funciones existentes
    Filter::FilterType filter_type = Filter::stringToFilterType(filter_name);
    
    Timer timer;
    timer.start();
    
    bool success = Filter::applyFilter(input_image, output_image, filter_type);
    
    timer.stop();
    
    if (!success) {
        std::cerr << "Process " << rank << ": Filter failed" << std::endl;
        delete input_image;
        delete output_image;
        MPI_Finalize();
        return 1;
    }
    
    std::cout << "Process " << rank << ": Filter applied in " 
              << timer.getElapsedMilliseconds() << " ms" << std::endl;
    
    // Solo el proceso 0 guarda el resultado
    if (rank == 0) {
        bool saved = output_image->save(output_file);
        if (saved) {
            std::cout << "Output saved to: " << output_file << std::endl;
        } else {
            std::cerr << "Failed to save output" << std::endl;
        }
    }
    
    delete input_image;
    delete output_image;
    
    MPI_Finalize();
    return 0;
}