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

int main(int argc, char *argv[]) {
  if (argc < 6) {
    cout << "Error, falta la ruta de alguno de los .txt" << endl
         << "uso: main <alfa.txt> <rojo.txt> <verde.txt> <azul.txt> "
            "<promedio.txt>"
         << endl;

    return 1;
  }
  vector<size_t> alfa;
  vector<size_t> azul;
  vector<size_t> rojo;
  vector<size_t> verde;
  vector<size_t> mean;

  // Esto hace que cada función se ejecute en su propio hilo.
#pragma omp parallel
#pragma omp single
  {
#pragma omp task
    alfa = parse_file(argv[1]);
#pragma omp task
    rojo = parse_file(argv[2]);
#pragma omp task
    verde = parse_file(argv[3]);
#pragma omp task
    azul = parse_file(argv[4]);
#pragma omp task
    mean = parse_file(argv[5]);
  }

  cout << "[INFO] Generando Imagen" << endl;

  // Genero un arreglo en el heap
  uint32_t *pixels = new uint32_t[WIDTH * HEIGHT];

#pragma omp parallel for
  for (int i = 0; i < HEIGHT * WIDTH; ++i) {
    if (rojo[i] > 255)
      rojo[i] = mean[i] * 0.3;
    if (verde[i] > 255)
      verde[i] = mean[i] * 0.59;
    if (azul[i] > 255)
      azul[i] = mean[i] * 0.11;

    // Pixel[i] = 0xFFAABBCC
    //  FF -> Alfa
    //  AA -> Azul
    //  BB -> Verde
    //  CC -> Rojo
    pixels[i] = (alfa[i] << 24 | azul[i] << 16 | verde[i] << 8 | rojo[i]);
  }

  // Esto es para vaciar memoria nomas
  rojo = vector<size_t>();
  azul = vector<size_t>();
  verde = vector<size_t>();
  alfa = vector<size_t>();
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
