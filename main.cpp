#include <cstddef> //para tamaños size_t
#include <fstream> //manipulacion de archivos
#include <iostream> // E/S estandar
#include <omp.h> //OpenMP
#include <vector> //vectores

//activar implementación de libreria stb
#define STB_IMAGE_WRITE_IMPLEMENTATION  
#include "stb_image_write.h" //Libreria externa para escribir imagenes

using namespace std;

// Dimensiones de la imagen
#define HEIGHT 7121
#define WIDTH 10681

vector<size_t> parse_file(string path) { // toma la ruta de un archivo y devuelve vector size_t
#pragma omp critical // Marca una sección crítica para evitar condiciones de carrera en la impresión de mensajes en paralelo
  cout << "[INFO] Poblando el archivo " << path << std::endl;
  vector<size_t> aux(WIDTH * HEIGHT, 0); //inicializa vector "aux" de tamaño widthxheight en 0

  ifstream file(path); //abre archivo ubicado en path

  for (int i = 0; i < HEIGHT * WIDTH; ++i) { //Itera sobre todos los elementos
    string out;
    file >> out; //Lee un valor del archivo como una cadena

    aux[i] = !out.compare("*") ? 300 : std::stoi(out); //Asigna 300 si el valor es "*", sino convierte la cadena a entero y lo asigna
  }

  file.close(); //Cierra el archivo despues de leer todos los datos

  return aux; //Devuelve el vector aux con los datos leidos
}

//Definie funcion "pixeling" que toma un puntero a un arreglo de pixeles, ruta de archivo, vector "mean", valor "step" y un porcentaje
void pixeling(uint32_t *pixels[], string path, vector<size_t> &mean, uint8_t step, float percentage) {
  ifstream file(path); //Abre el archivo ubicado en "path" para lectura

  for (int i = 0; i < HEIGHT * WIDTH; ++i) { //Itera sobre todos los elementos
    string out;
    file >> out;  //Lee el archivo como una cadena

    uint32_t value = !out.compare("*") ? mean[i] * percentage : (uint32_t)std::stoi(out); //Calcula el valor del pix
    (*pixels)[i] = (value << step | (*pixels)[i]);
  }

  file.close(); // Cierra el archivo leido
}

int main(int argc, char *argv[]) {  //Verifica que se proporcionen al menos 6 argumentos
  if (argc < 6) {
    cout << "Error, falta la ruta de alguno de los .txt" << endl
         << "uso: main <alfa.txt> <rojo.txt> <verde.txt> <azul.txt> "
            "<promedio.txt>"
         << endl;

    return 1;
  }
  // Genero un arreglo en el heap
  uint32_t *pixels = new uint32_t[WIDTH * HEIGHT];
#pragma omp parallel // Inicia un bucle paralelo para inicializar todos los píxeles a cero en paralelo
  for(size_t i = 0; i < WIDTH * HEIGHT; i++){
    pixels[i] = 0;
  }

  vector<size_t> mean = parse_file(argv[5]);  //Lee el archivo "promedio.txt" y almacena los datos en el vector "mean"

  cout << "[INFO] Generando Imagen" << endl;
  
  // Esto hace que cada función se ejecute en su propio hilo.
#pragma omp parallel //Inicia una región paralela.
#pragma omp single // Garantiza que las tareas siguientes se ejecuten solo una vez
  {
#pragma omp task // Cada tarea siguiente se ejecutará en paralelo.
     pixeling(&pixels, argv[1], mean, 24, 0.0f); //: Llama a la función pixeling en paralelo para el archivo alfa.txt
#pragma omp task // Cada tarea siguiente se ejecutará en paralelo.
     pixeling(&pixels, argv[2], mean, 0, 0.3f); // Llama a la función pixeling en paralelo para el archivo rojo.txt.
#pragma omp task // Cada tarea siguiente se ejecutará en paralelo.
     pixeling(&pixels, argv[3], mean, 8, 0.59f); // Llama a la función pixeling en paralelo para el archivo verde.txt.
#pragma omp task // Cada tarea siguiente se ejecutará en paralelo.
     pixeling(&pixels, argv[4], mean, 16, 0.11f); // Llama a la función pixeling en paralelo para el archivo azul.txt.
  }

  // Despejo de memoria del vector mean
  mean = vector<size_t>();

  cout << "[INFO] Escribiendo imagen" << endl;

  const char *file_path = "output.png";  //Define la ruta del archivo de salida
  if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels,
                      sizeof(uint32_t) * WIDTH)) {  //Escribe la imagen en formato "PNG" usando libreria "STB"
    fprintf(stderr, "ERROR: could not write %s\n", file_path);
    return 1;
  }

  return 0;  //se ejecutó correctamente
}
