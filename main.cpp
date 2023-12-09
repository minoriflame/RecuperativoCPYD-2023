#include <cstddef>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

#define HEIGHT 7121
#define WIDTH 10681

vector<size_t> parse_file(string path) {
#pragma omp critical
  cout << "[INFO] Poblando el archivo " << path << std::endl;
  vector<size_t> aux(WIDTH * HEIGHT, 0);

  ifstream file(path);

  for (int i = 0; i < HEIGHT * WIDTH; ++i) {
    string out;
    file >> out;

    aux[i] = !out.compare("*") ? 300 : std::stoi(out);
  }

  file.close();

  return aux;
}

void pixeling(uint32_t *pixels[], string path, vector<size_t> &mean, uint8_t step, float percentage) {
  ifstream file(path);

  for (int i = 0; i < HEIGHT * WIDTH; ++i) {
    string out;
    file >> out;

    uint32_t value = !out.compare("*") ? mean[i] * percentage : (uint32_t)std::stoi(out);
    (*pixels)[i] = (value << step | (*pixels)[i]);
  }

  file.close();
}

int main(int argc, char *argv[]) {
  if (argc < 6) {
    cout << "Error, falta la ruta de alguno de los .txt" << endl
         << "uso: main <alfa.txt> <rojo.txt> <verde.txt> <azul.txt> "
            "<promedio.txt>"
         << endl;

    return 1;
  }
  // Genero un arreglo en el heap
  uint32_t *pixels = new uint32_t[WIDTH * HEIGHT];
#pragma omp parallel
  for(size_t i = 0; i < WIDTH * HEIGHT; i++){
    pixels[i] = 0;
  }

  vector<size_t> mean = parse_file(argv[5]);

  cout << "[INFO] Generando Imagen" << endl;
  // Esto hace que cada función se ejecute en su propio hilo.
#pragma omp parallel
#pragma omp single
  {
#pragma omp task
     pixeling(&pixels, argv[1], mean, 24, 0.0f);
#pragma omp task
     pixeling(&pixels, argv[2], mean, 0, 0.3f);
#pragma omp task
     pixeling(&pixels, argv[3], mean, 8, 0.59f);
#pragma omp task
     pixeling(&pixels, argv[4], mean, 16, 0.11f);
  }

  // Despejo memoria
  mean = vector<size_t>();

  cout << "[INFO] Escribiendo imagen" << endl;

  const char *file_path = "output.png";
  if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels,
                      sizeof(uint32_t) * WIDTH)) {
    fprintf(stderr, "ERROR: could not write %s\n", file_path);
    return 1;
  }

  return 0;
}
