# Procesador de Imágenes (Secuencial, Pthreads, OpenMP y MPI)

Este repositorio contiene implementaciones en C++ para aplicar filtros a imágenes en formato **PGM** (escala de grises) y **PPM** (color).  
El proyecto compara el rendimiento de diferentes modelos de **programación paralela** frente a la versión **secuencial**.

Filtros soportados:
- **Blur**
- **Laplace**
- **Sharpen**

---

## 📌 Resumen de Comandos

| Programa         | Compilación                                                                                   | Ejecución                                                                 |
|------------------|-----------------------------------------------------------------------------------------------|---------------------------------------------------------------------------|
| **Secuencial**   | `g++ -o processor processor.cpp filter.cpp imagen.cpp PGMimage.cpp PPMimage.cpp timer.cpp`    | `./processor ./imagenes/lena.pgm ./imagenes/lena_blur.pgm --f blur`       |
| **Pthreads**     | `g++ -o processor_pthread processor_pthread.cpp filter.cpp imagen.cpp PGMimage.cpp PPMimage.cpp timer.cpp -lpthread` | `./processor_pthread ./imagenes/fruit.ppm ./imagenes/fruit_col_pthread_la.ppm --f laplace` |
| **OpenMP**       | `g++ -o image_processor processor_omp.cpp filter.cpp imagen.cpp PGMimage.cpp PPMimage.cpp timer.cpp -fopenmp` | `./image_processor ./imagenes/fruit.ppm ./imagenes/fruit_result.ppm` |
| **MPI**          | mpic++ -std=c++11 -Wall -Wextra -g processor_mpi.cpp imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp -o mpi_processor
p timer.cpp` | `mpirun -np 4 ./mpi_processor ./imagenes/lena.pgm ./imagenes/lena_simple_mpi.pgm --f blur` |

---

## 📌 Descripción de Programas

### 🔹 Secuencial
- Procesa la imagen en un único hilo.
- Base de comparación para medir la aceleración obtenida con paralelismo.

### 🔹 Pthreads
- Divide la imagen entre múltiples hilos POSIX.
- Cada hilo aplica el filtro a una sección de la imagen.
- Permite observar cómo el paralelismo manual mejora (o degrada) el tiempo de procesamiento.

### 🔹 OpenMP
- Utiliza directivas de compilador (`#pragma omp parallel for`) para paralelizar el recorrido de los píxeles.
- Se simplifica la gestión de hilos y balanceo de carga.
- Puede aplicar varios filtros en paralelo de manera eficiente.

### 🔹 MPI (en Docker con Compose)
- Divide el procesamiento entre **múltiples procesos distribuidos** en distintos contenedores.
- Cada proceso trabaja sobre un subconjunto de filas de la imagen.
- Los resultados se combinan para generar la imagen final.

---

## ⚡ Ejecución en MPI con Docker Compose

### 1️⃣ Levantar los nodos
Se incluye un archivo `docker-compose.yml` que crea 3 nodos (`node1`, `node2`, `node3`) en la misma red `mpi-net`.

```bash
docker-compose up -d

```crar cada nodo con su nombre
docker run -dit --name node2 --hostname node2 --network mpi-net \
  -v "%cd%":/home/japeto/app -w /home/japeto/app japeto/parallel-tools:v64 bash

```se copila en el host
mpirun -np 4 ./mpi_processor ./imagenes/lena.pgm ./imagenes/lena_simple_mpi.pgm --f blur

```Bajar contenedores
docker-compose down
docker network rm mpi-net   # si fue creada manualmente


