#include <iostream>
#include <cstring>
#include <omp.h>
#include <string>
#include "imagen.h"
#include "PGMimage.h"
#include "PPMimage.h"
#include "filter.h"
#include "timer.h"

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " input_file output_prefix" << std::endl;
    std::cout << "  input_file:   Input image file (PPM or PGM)" << std::endl;
    std::cout << "  output_prefix: Prefix for output files" << std::endl;
    std::cout << "  The program will generate 3 output files:" << std::endl;
    std::cout << "    - output_prefix_blur.ext" << std::endl;
    std::cout << "    - output_prefix_laplace.ext" << std::endl;
    std::cout << "    - output_prefix_sharpen.ext" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " lena.ppm lena_result" << std::endl;
    std::cout << "  " << program_name << " fruit.pgm fruit_result" << std::endl;
}

Imagen* createImageFromFile(const char* filename) {
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
        image = new PGMImage();
    } else if (strcmp(magic, "P3") == 0) {
        image = new PPMImage();
    } else {
        std::cerr << "Error: Unsupported image format. Only P2 (PGM) and P3 (PPM) are supported." << std::endl;
        return nullptr;
    }
    
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
        PGMImage* pgm_input = dynamic_cast<PGMImage*>(input_image);
        output = pgm_input->clone();
    } else if (strcmp(input_image->getMagic(), "P3") == 0) {
        PPMImage* ppm_input = dynamic_cast<PPMImage*>(input_image);
        output = ppm_input->clone();
    }
    
    return output;
}

std::string getFileExtension(const char* filename) {
    std::string file_str(filename);
    size_t dot_pos = file_str.find_last_of('.');
    if (dot_pos != std::string::npos) {
        return file_str.substr(dot_pos);
    }
    return "";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    const char* input_filename = argv[1];
    const char* output_prefix = argv[2];
    
    Timer total_timer;
    Timer load_timer;
    Timer process_timer;
    
    std::cout << "=== OpenMP Image Processor ===" << std::endl;
    std::cout << "Input file: " << input_filename << std::endl;
    std::cout << "Output prefix: " << output_prefix << std::endl;
    std::cout << "Number of OpenMP threads available: " << omp_get_max_threads() << std::endl;
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
    
    // Obtener extensión del archivo
    std::string extension = getFileExtension(input_filename);
    
    // Crear nombres de archivos de salida
    std::string blur_filename = std::string(output_prefix) + "_blur" + extension;
    std::string laplace_filename = std::string(output_prefix) + "_laplace" + extension;
    std::string sharpen_filename = std::string(output_prefix) + "_sharpen" + extension;
    
    // Crear imágenes de salida (una por cada filtro)
    Imagen* blur_output = createOutputImage(input_image);
    Imagen* laplace_output = createOutputImage(input_image);
    Imagen* sharpen_output = createOutputImage(input_image);
    
    if (!blur_output || !laplace_output || !sharpen_output) {
        std::cerr << "Failed to create output images." << std::endl;
        delete input_image;
        delete blur_output;
        delete laplace_output;
        delete sharpen_output;
        return 1;
    }
    
    // Aplicar filtros en paralelo usando OpenMP
    std::cout << "Applying filters in parallel..." << std::endl;
    process_timer.start();
    
    bool success[3] = {false, false, false};
    
    // Usar parallel sections para aplicar cada filtro en un hilo diferente
    #pragma omp parallel sections num_threads(3)
    {
        #pragma omp section
        {
            int thread_id = omp_get_thread_num();
            std::cout << "Thread " << thread_id << " applying BLUR filter..." << std::endl;
            success[0] = Filter::applyFilter(input_image, blur_output, Filter::BLUR);
            if (success[0]) {
                std::cout << "Thread " << thread_id << " completed BLUR filter" << std::endl;
            }
        }
        
        #pragma omp section
        {
            int thread_id = omp_get_thread_num();
            std::cout << "Thread " << thread_id << " applying LAPLACE filter..." << std::endl;
            success[1] = Filter::applyFilter(input_image, laplace_output, Filter::LAPLACE);
            if (success[1]) {
                std::cout << "Thread " << thread_id << " completed LAPLACE filter" << std::endl;
            }
        }
        
        #pragma omp section
        {
            int thread_id = omp_get_thread_num();
            std::cout << "Thread " << thread_id << " applying SHARPEN filter..." << std::endl;
            success[2] = Filter::applyFilter(input_image, sharpen_output, Filter::SHARPEN);
            if (success[2]) {
                std::cout << "Thread " << thread_id << " completed SHARPEN filter" << std::endl;
            }
        }
    }
    
    process_timer.stop();
    
    // Verificar que todos los filtros se aplicaron correctamente
    if (!success[0] || !success[1] || !success[2]) {
        std::cerr << "Error: One or more filters failed to apply." << std::endl;
        if (!success[0]) std::cerr << "  - BLUR filter failed" << std::endl;
        if (!success[1]) std::cerr << "  - LAPLACE filter failed" << std::endl;
        if (!success[2]) std::cerr << "  - SHARPEN filter failed" << std::endl;
        
        delete input_image;
        delete blur_output;
        delete laplace_output;
        delete sharpen_output;
        return 1;
    }
    
    std::cout << "All filters applied successfully!" << std::endl;
    std::cout << "  Processing time: " << process_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << std::endl;
    
    // Guardar imágenes de salida en paralelo
    std::cout << "Saving output images..." << std::endl;
    Timer save_timer;
    save_timer.start();
    
    bool save_success[3] = {false, false, false};
    
    #pragma omp parallel sections num_threads(3)
    {
        #pragma omp section
        {
            int thread_id = omp_get_thread_num();
            save_success[0] = blur_output->save(blur_filename.c_str());
            if (save_success[0]) {
                std::cout << "Thread " << thread_id << " saved: " << blur_filename << std::endl;
            }
        }
        
        #pragma omp section
        {
            int thread_id = omp_get_thread_num();
            save_success[1] = laplace_output->save(laplace_filename.c_str());
            if (save_success[1]) {
                std::cout << "Thread " << thread_id << " saved: " << laplace_filename << std::endl;
            }
        }
        
        #pragma omp section
        {
            int thread_id = omp_get_thread_num();
            save_success[2] = sharpen_output->save(sharpen_filename.c_str());
            if (save_success[2]) {
                std::cout << "Thread " << thread_id << " saved: " << sharpen_filename << std::endl;
            }
        }
    }
    
    save_timer.stop();
    
    if (!save_success[0] || !save_success[1] || !save_success[2]) {
        std::cerr << "Error: Failed to save one or more output images." << std::endl;
        if (!save_success[0]) std::cerr << "  - Failed to save: " << blur_filename << std::endl;
        if (!save_success[1]) std::cerr << "  - Failed to save: " << laplace_filename << std::endl;
        if (!save_success[2]) std::cerr << "  - Failed to save: " << sharpen_filename << std::endl;
        
        delete input_image;
        delete blur_output;
        delete laplace_output;
        delete sharpen_output;
        return 1;
    }
    
    total_timer.stop();
    
    std::cout << std::endl;
    std::cout << "=== Performance Summary ===" << std::endl;
    std::cout << "Load time:       " << load_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Processing time: " << process_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Save time:       " << save_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Total time:      " << total_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Output files generated:" << std::endl;
    std::cout << "  - " << blur_filename << std::endl;
    std::cout << "  - " << laplace_filename << std::endl;
    std::cout << "  - " << sharpen_filename << std::endl;
    
    // Limpiar memoria
    delete input_image;
    delete blur_output;
    delete laplace_output;
    delete sharpen_output;
    
    return 0;
}