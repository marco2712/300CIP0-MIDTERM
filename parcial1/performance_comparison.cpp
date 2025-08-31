#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>
#include "timer.h"

struct TestResult {
    std::string implementation;
    std::string image;
    std::string filter;
    int threads_or_processes;
    double load_time;
    double process_time;
    double save_time;
    double total_time;
};

void runTest(const std::string& implementation, const std::string& image, 
             const std::string& filter, int num_threads, std::vector<TestResult>& results) {
    Timer timer;
    timer.start();
    
    std::string command;
    if (implementation == "sequential") {
        command = "./processor " + image + " output_" + implementation + ".ppm --f " + filter;
    }
    else if (implementation == "pthread" || implementation == "omp") {
        command = "./processor_" + implementation + " " + image + " output_" + implementation + ".ppm --f " + 
                 filter + " --t " + std::to_string(num_threads);
    }
    else if (implementation == "mpi") {
        command = "mpirun -np " + std::to_string(num_threads) + " ./processor_mpi " + 
                 image + " output_" + implementation + ".ppm --f " + filter;
    }
    
    std::cout << "\nExecuting: " << command << std::endl;
    system(command.c_str());
    
    timer.stop();
    
    // Los tiempos específicos son registrados por cada implementación en su salida
    TestResult result;
    result.implementation = implementation;
    result.image = image;
    result.filter = filter;
    result.threads_or_processes = num_threads;
    result.total_time = timer.getElapsedMilliseconds();
    
    results.push_back(result);
}

void saveResults(const std::vector<TestResult>& results, const std::string& filename) {
    std::ofstream file(filename);
    
    // Escribir encabezado CSV
    file << "Implementation,Image,Filter,Threads/Processes,Total Time (ms)\n";
    
    // Escribir resultados
    for (const auto& result : results) {
        file << result.implementation << ","
             << result.image << ","
             << result.filter << ","
             << result.threads_or_processes << ","
             << result.total_time << "\n";
    }
    
    file.close();
}

int main() {
    std::vector<TestResult> results;
    std::vector<std::string> images = {"imagenes/fruit.ppm", "imagenes/lena.ppm"};
    std::vector<std::string> filters = {"blur", "sharpen", "laplace"};
    std::vector<int> thread_counts = {1, 2, 4, 8};
    
    std::cout << "=== Performance Comparison ===" << std::endl;
    
    // Pruebas secuenciales
    for (const auto& image : images) {
        for (const auto& filter : filters) {
            runTest("sequential", image, filter, 1, results);
        }
    }
    
    // Pruebas con pthreads
    for (const auto& image : images) {
        for (const auto& filter : filters) {
            for (int threads : thread_counts) {
                runTest("pthread", image, filter, threads, results);
            }
        }
    }
    
    // Pruebas con OpenMP
    for (const auto& image : images) {
        for (const auto& filter : filters) {
            for (int threads : thread_counts) {
                runTest("omp", image, filter, threads, results);
            }
        }
    }
    
    // Pruebas con MPI
    for (const auto& image : images) {
        for (const auto& filter : filters) {
            for (int processes : thread_counts) {
                runTest("mpi", image, filter, processes, results);
            }
        }
    }
    
    // Guardar resultados en CSV
    saveResults(results, "performance_results.csv");
    
    std::cout << "\nResults have been saved to performance_results.csv" << std::endl;
    return 0;
}
