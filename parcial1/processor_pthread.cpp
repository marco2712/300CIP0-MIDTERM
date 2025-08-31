#include <iostream>
#include <cstring>
#include <pthread.h>
#include <vector>
#include "imagen.h"
#include "PGMimage.h"
#include "PPMimage.h"
#include "filter.h"
#include "timer.h"

#define NUM_THREADS 4

struct ThreadData {
    Imagen* input;
    Imagen* output;
    Filter::FilterType filter_type;
    int thread_id;
    int start_row;
    int end_row;
    int width;
    int height;
    const float (*kernel)[3];
    bool success;
};

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " input_file output_file [--f filter_type]" << std::endl;
    std::cout << "  input_file:  Input image file (PPM or PGM)" << std::endl;
    std::cout << "  output_file: Output image file" << std::endl;
    std::cout << "  --f filter:  Filter to apply (blur, laplace, sharpen)" << std::endl;
    std::cout << "               If no filter specified, image will be copied" << std::endl;
    std::cout << std::endl;
    std::cout << "This version uses " << NUM_THREADS << " threads (pthreads)" << std::endl;
}

const float* getKernel(Filter::FilterType filter_type) {
    static const float blur_kernel[3][3] = {
        {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
        {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f},
        {1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f}
    };
    
    static const float laplace_kernel[3][3] = {
        {0.0f, -1.0f, 0.0f},
        {-1.0f, 4.0f, -1.0f},
        {0.0f, -1.0f, 0.0f}
    };
    
    static const float sharpen_kernel[3][3] = {
        {0.0f, -1.0f, 0.0f},
        {-1.0f, 5.0f, -1.0f},
        {0.0f, -1.0f, 0.0f}
    };
    
    switch (filter_type) {
        case Filter::BLUR:
            return (const float*)blur_kernel;
        case Filter::LAPLACE:
            return (const float*)laplace_kernel;
        case Filter::SHARPEN:
            return (const float*)sharpen_kernel;
        default:
            return (const float*)blur_kernel;
    }
}

void* processRegionPGM(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    PGMImage* input = dynamic_cast<PGMImage*>(data->input);
    PGMImage* output = dynamic_cast<PGMImage*>(data->output);
    
    if (!input || !output) {
        data->success = false;
        return nullptr;
    }
    
    const float (*kernel)[3] = data->kernel;
    
    for (int y = data->start_row; y < data->end_row; y++) {
        for (int x = 0; x < data->width; x++) {
            float sum = 0.0f;
            
            // Aplicar kernel 3x3
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int nx = x + kx;
                    int ny = y + ky;
                    
                    if (input->isValidCoordinate(nx, ny)) {
                        int pixel_value = input->getGrayValue(nx, ny);
                        sum += pixel_value * kernel[ky + 1][kx + 1];
                    }
                }
            }
            
            // Clamping del resultado
            int result = std::max(0, std::min(input->getMaxColor(), static_cast<int>(sum)));
            output->setGrayValue(x, y, result);
        }
    }
    
    data->success = true;
    return nullptr;
}

void* processRegionPPM(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    PPMImage* input = dynamic_cast<PPMImage*>(data->input);
    PPMImage* output = dynamic_cast<PPMImage*>(data->output);
    
    if (!input || !output) {
        data->success = false;
        return nullptr;
    }
    
    const float (*kernel)[3] = data->kernel;
    
    for (int y = data->start_row; y < data->end_row; y++) {
        for (int x = 0; x < data->width; x++) {
            float sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f;
            
            // Aplicar kernel 3x3
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int nx = x + kx;
                    int ny = y + ky;
                    
                    if (input->isValidCoordinate(nx, ny)) {
                        RGB pixel_value = input->getRGBValue(nx, ny);
                        float kernel_val = kernel[ky + 1][kx + 1];
                        sum_r += pixel_value.r * kernel_val;
                        sum_g += pixel_value.g * kernel_val;
                        sum_b += pixel_value.b * kernel_val;
                    }
                }
            }
            
            // Clamping del resultado
            int result_r = std::max(0, std::min(input->getMaxColor(), static_cast<int>(sum_r)));
            int result_g = std::max(0, std::min(input->getMaxColor(), static_cast<int>(sum_g)));
            int result_b = std::max(0, std::min(input->getMaxColor(), static_cast<int>(sum_b)));
            
            output->setRGBValue(x, y, result_r, result_g, result_b);
        }
    }
    
    data->success = true;
    return nullptr;
}

bool applyFilterPthread(Imagen* input, Imagen* output, Filter::FilterType filter_type) {
    if (!input || !output) {
        return false;
    }
    
    int width = input->getWidth();
    int height = input->getHeight();
    
    // Configurar imagen de salida
    output->setWidth(width);
    output->setHeight(height);
    output->setMaxColor(input->getMaxColor());
    output->allocatePixels();
    
    // Obtener kernel
    const float* kernel_ptr = getKernel(filter_type);
    const float (*kernel)[3] = reinterpret_cast<const float (*)[3]>(kernel_ptr);
    
    // Crear threads y datos
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    int rows_per_thread = height / NUM_THREADS;
    int remaining_rows = height % NUM_THREADS;
    
    // Función de procesamiento según el tipo de imagen
    void* (*process_function)(void*) = nullptr;
    
    if (strcmp(input->getMagic(), "P2") == 0) {
        process_function = processRegionPGM;
    } else if (strcmp(input->getMagic(), "P3") == 0) {
        process_function = processRegionPPM;
    } else {
        return false;
    }
    
    // Crear y lanzar threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].input = input;
        thread_data[i].output = output;
        thread_data[i].filter_type = filter_type;
        thread_data[i].thread_id = i;
        thread_data[i].width = width;
        thread_data[i].height = height;
        thread_data[i].kernel = kernel;
        thread_data[i].success = false;
        
        // Calcular filas para este hilo
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i + 1) * rows_per_thread;
        
        // Asignar filas restantes al último hilo
        if (i == NUM_THREADS - 1) {
            thread_data[i].end_row += remaining_rows;
        }
        
        int result = pthread_create(&threads[i], nullptr, process_function, &thread_data[i]);
        if (result != 0) {
            std::cerr << "Error creating thread " << i << std::endl;
            return false;
        }
    }
    
    // Esperar a que todos los hilos terminen
    bool overall_success = true;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
        if (!thread_data[i].success) {
            overall_success = false;
        }
    }
    
    return overall_success;
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

int main(int argc, char* argv[]) {
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
    
    Timer total_timer, load_timer, process_timer, save_timer;
    
    std::cout << "=== Pthread Image Processor (" << NUM_THREADS << " threads) ===" << std::endl;
    std::cout << "Input file: " << input_filename << std::endl;
    std::cout << "Output file: " << output_filename << std::endl;
    
    if (filter_name) {
        std::cout << "Filter: " << filter_name << std::endl;
    } else {
        std::cout << "Operation: Copy image (no filter)" << std::endl;
    }
    std::cout << std::endl;
    
    total_timer.start();
    
    // Cargar imagen
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
    
    // Aplicar filtro con pthreads
    if (filter_name) {
        std::cout << "Applying filter with " << NUM_THREADS << " threads: " << filter_name << "..." << std::endl;
        process_timer.start();
        
        Filter::FilterType filter_type = Filter::stringToFilterType(filter_name);
        bool success = applyFilterPthread(input_image, output_image, filter_type);
        
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
    }
    
    // Guardar imagen
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
    std::cout << "=== Performance Summary (Pthreads) ===" << std::endl;
    std::cout << "Threads used:    " << NUM_THREADS << std::endl;
    std::cout << "Load time:       " << load_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Processing time: " << process_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Save time:       " << save_timer.getElapsedMilliseconds() << " ms" << std::endl;
    std::cout << "Total time:      " << total_timer.getElapsedMilliseconds() << " ms" << std::endl;
    
    delete input_image;
    delete output_image;
    
    return 0;
}