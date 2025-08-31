#include <iostream>
#include <cstring>
#include "imagen.h"
#include "PGMimage.h"
#include "PPMimage.h"
#include "filter.h"
#include "timer.h"

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " input_file output_file [--f filter_type]" << std::endl;
    std::cout << "  input_file:  Input image file (PPM or PGM)" << std::endl;
    std::cout << "  output_file: Output image file" << std::endl;
    std::cout << "  --f filter:  Filter to apply (blur, laplace, sharpen)" << std::endl;
    std::cout << "               If no filter specified, image will be copied" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " lena.ppm lena_copy.ppm" << std::endl;
    std::cout << "  " << program_name << " fruit.ppm fruit_blur.ppm --f blur" << std::endl;
    std::cout << "  " << program_name << " image.pgm image_sharp.pgm --f sharpen" << std::endl;
}

Imagen* createImageFromFile(const char* filename) {
    // Detectar tipo de archivo leyendo el magic number
    FILE* file = fopen(filename, "r");
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return nullptr;
    }
    
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1) {
        std::cerr << "Error: Cannot read magic number from " << filename << std::endl;
        fclose(file);
        return nullptr;
    }
    fclose(file);
    
    Imagen* image = nullptr;
    
    if (strcmp(magic, "P2") == 0) {
        // Es PGM
        image = new PGMImage();
    } else if (strcmp(magic, "P3") == 0) {
        // Es PPM
        image = new PPMImage();
    } else {
        std::cerr << "Error: Unsupported image format. Only P2 (PGM) and P3 (PPM) are supported." << std::endl;
        return nullptr;
    }
    
    // Cargar la imagen
    if (!image->load(filename)) {
        delete image;
        return nullptr;
    }
    
    return image;
}

Imagen* createOutputImage(Imagen* input_image) {
    if (!input_image) {
        return nullptr;
    }
    
    Imagen* output = nullptr;
    
    if (strcmp(input_image->getMagic(), "P2") == 0) {
        // Crear PGM
        PGMImage* pgm_input = dynamic_cast<PGMImage*>(input_image);
        output = pgm_input->clone();
    } else if (strcmp(input_image->getMagic(), "P3") == 0) {
        // Crear PPM
        PPMImage* ppm_input = dynamic_cast<PPMImage*>(input_image);
        output = ppm_input->clone();
    }
    
    return output;
}

int main(int argc, char* argv[]) {
    // Verificar argumentos mínimos
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    const char* filter_name = nullptr;
    
    // Parsear argumentos para filtro
    for (int i = 3; i < argc - 1; i++) {
        if (strcmp(argv[i], "--f") == 0 && i + 1 < argc) {
            filter_name = argv[i + 1];
            break;
        }
    }
    
    Timer total_timer;
    Timer load_timer;
    Timer process_timer;
    Timer save_timer;
    
    std::cout << "=== Image Processor ===" << std::endl;
    std::cout << "Input file: " << input_filename << std::endl;
    std::cout << "Output file: " << output_filename << std::endl;
    
    if (filter_name) {
        std::cout << "Filter: " << filter_name << std::endl;
    } else {
        std::cout << "Operation: Copy image (no filter)" << std::endl;
    }
    std::cout << std::endl;
    
    total_timer.start();
    
    // Cargar imagen de entrada
    std::cout << "Loading input image..." << std::endl;
    load_timer.start();
    Imagen* input_image = createImageFromFile(input_filename);
    load_timer.stop();
    
    if (!input_image) {
        std::cerr << "Failed to load input image." << std::endl;
        return 1;
    }
    
    std::cout << "Image loaded successfully!" << std::endl;
    std::cout << "  Format: " << input_image->getMagic() << std::endl;
    std::cout << "  Dimensions: " << input_image->getWidth() << "x" << input_image->getHeight() << std::endl;
    std::cout << "  Max color value: " << input_image->getMaxColor() << std::endl;
    std::cout << "  Load time: " << load_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << std::endl;
    
    // Crear imagen de salida
    Imagen* output_image = createOutputImage(input_image);
    if (!output_image) {
        std::cerr << "Failed to create output image." << std::endl;
        delete input_image;
        return 1;
    }
    
    // Aplicar filtro si se especifica
    if (filter_name) {
        std::cout << "Applying filter: " << filter_name << "..." << std::endl;
        process_timer.start();
        
        Filter::FilterType filter_type = Filter::stringToFilterType(filter_name);
        bool success = Filter::applyFilter(input_image, output_image, filter_type);
        
        process_timer.stop();
        
        if (!success) {
            std::cerr << "Failed to apply filter." << std::endl;
            delete input_image;
            delete output_image;
            return 1;
        }
        
        std::cout << "Filter applied successfully!" << std::endl;
        std::cout << "  Processing time: " << process_timer.getElapsedMilliseconds() << " ms" << std::endl;
        std::cout << std::endl;
    } else {
        std::cout << "No filter specified, copying image..." << std::endl;
        process_timer.start();
        process_timer.stop();
        std::cout << "  Processing time: " << process_timer.getElapsedMilliseconds() << " ms" << std::endl;
        std::cout << std::endl;
    }
    
    // Guardar imagen de salida
    std::cout << "Saving output image..." << std::endl;
    save_timer.start();
    bool save_success = output_image->save(output_filename);
    save_timer.stop();
    
    if (!save_success) {
        std::cerr << "Failed to save output image." << std::endl;
        delete input_image;
        delete output_image;
        return 1;
    }
    
    total_timer.stop();
    
    std::cout << "Image saved successfully!" << std::endl;
    std::cout << "  Save time: " << save_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << std::endl;
    
    // Mostrar resumen de tiempos
    std::cout << "=== Performance Summary ===" << std::endl;
    std::cout << "Load time:       " << load_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Processing time: " << process_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Save time:       " << save_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Total time:      " << total_timer.getElapsedMilliseconds() << " ms" << std::endl;
    
    // Limpiar memoria
    delete input_image;
    delete output_image;
    
    return 0;
}