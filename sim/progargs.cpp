#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "progargs.hpp"
#include "block.hpp"
#include "grid.hpp"

#include <filesystem>

using namespace std;
int validate_time_steps(int nts) {
  if (nts == 0) {
    cerr << "Error: time steps must be numeric." << "\n";
    return -1;
  }
  if (nts < 0) {
    cerr << "Error: Invalid number of time steps." << "\n";
    return -2;
  }
  return 0;
}

int validate_input_file(const char* inputFileName) {
  cout << "Input file: " << inputFileName << "\n";
  ifstream input_file(inputFileName);
  if (!input_file.is_open()) {
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::cout << "Current working directory: " << currentPath << std::endl;
    cerr << "Error: Cannot open " << inputFileName << " for reading." << "\n";
    return -1;
  }


  return 0;
}

int validate_output_file(const char* outputFileName) {
  ofstream output_file(outputFileName);
  if (!output_file.is_open()) {
    cerr << "Error: Cannot open " << outputFileName << " for writing." << "\n";
    return -1;
  }
  return 0;
}

array<int,2> validate_parameters(int argc, const char* argv[]) {

  array <int,2> error_type={0,0};
  if (argc != 4) {
    cerr << "Error: Invalid number of arguments: " << argc << "\n";
    error_type[0] = -1;
    return error_type;
  }

  error_type[1] = stoi(argv[1]);
  error_type[0] = validate_time_steps(error_type[1]);
  if (error_type[0] != 0) {
    return error_type;
  };
  if (validate_input_file(argv[2]) != 0)
  {
    error_type[0] = -3;
    return error_type;
  }
  if (validate_output_file(argv[3]) != 0){
    error_type[0] = -4;
    return error_type;
  }
  return error_type;
}

void read_input_file (Malla& malla, const char * in_file, vector<Particle>& particles) {

  ifstream input_file(in_file, ios::binary);     /* TODO ppm check errors? */

  // Crear la malla base
  float ppm=0;
  input_file.read(reinterpret_cast<char *>(&ppm), sizeof(ppm));
  int np=0;
  input_file.read(reinterpret_cast<char *>(&np), sizeof(np));
  //Comprobamos que np sea mayor que 1
  check_np(np);

  // Calculate constants TODO crear archivo constantes
  // Check if the number of particles read matches the header
  auto ppm_double = static_cast<double>(ppm);

  /* TODO pasar struct constantes a malla*/
  // Creamos la malla y la llenamos de bloques vacíos
  create_fill_grid(malla, np, ppm_double);
  refactor_gordo(in_file, malla, particles);


  // Read particle data in a single loop and cast from float to double

  input_file.close(); /* TODO esto hay que cerrarlo? */

  cout << "Number of particles: " << np << "\n";
  cout << "Particles per meter: " << ppm << "\n";
  cout << "Smoothing length: " << malla.h << "\n";
  cout << "Particle mass: " << particle_mass(ppm) << "\n";
  cout << "Grid size: " << malla.n_blocks[0] << " x " << malla.n_blocks[1] << " x " << malla.n_blocks[2] << "\n";
  cout << "Number of blocks: " <<  malla.n_blocks[0] *  malla.n_blocks[1] *  malla.n_blocks[2] << "\n";
  cout << "Block size: " << malla.size_blocks[0] << " x " << malla.size_blocks[1] << " x " << malla.size_blocks[2] << "\n";
}



int write_output_file (Malla& malla, const char * out_file, vector<Particle>& particles){
  int np = malla.np;
  float ppm = malla.ppm;
  //Escribir en el archivo np y ppm antes de entrar en el bucle para las partículas
  ofstream output_file(out_file, ios::binary);
  output_file.write(reinterpret_cast<char*>(&ppm), sizeof(ppm));
  output_file.write(reinterpret_cast<char*>(&np), sizeof(np));

  float px_float, py_float, pz_float, hvx_float, hvy_float, hvz_float, vx_float, vy_float, vz_float;
  vector<Particle> particles_out;
  // Loop through all the blocks
  for (Particle & particle: particles){
      particles_out.push_back(particle);
  }


  // Sort particles_out by id
  sort(particles_out.begin(), particles_out.end(), [](Particle & a, Particle & b) { return a.id < b.id; });

  for (Particle & particle : particles_out) {
    px_float = static_cast<float>(particle.p[0]);
    py_float = static_cast<float>(particle.p[1]);
    pz_float = static_cast<float>(particle.p[2]);
    hvx_float = static_cast<float>(particle.hv[0]);
    hvy_float = static_cast<float>(particle.hv[1]);
    hvz_float = static_cast<float>(particle.hv[2]);
    vx_float = static_cast<float>(particle.v[0]);
    vy_float = static_cast<float>(particle.v[1]);
    vz_float = static_cast<float>(particle.v[2]);
    output_file.write(reinterpret_cast<char*>(&px_float), sizeof(px_float));
    output_file.write(reinterpret_cast<char*>(&py_float), sizeof(py_float));
    output_file.write(reinterpret_cast<char*>(&pz_float), sizeof(pz_float));
    output_file.write(reinterpret_cast<char*>(&hvx_float), sizeof(hvx_float));
    output_file.write(reinterpret_cast<char*>(&hvy_float), sizeof(hvy_float));
    output_file.write(reinterpret_cast<char*>(&hvz_float), sizeof(hvz_float));
    output_file.write(reinterpret_cast<char*>(&vx_float), sizeof(vx_float));
    output_file.write(reinterpret_cast<char*>(&vy_float), sizeof(vy_float));
    output_file.write(reinterpret_cast<char*>(&vz_float), sizeof(vz_float));

    if (particle.id == 0){
    cout << "Particle " << particle.id << " Data:" << "\n";
    cout << "px: " << particle.p[0] << "\n";
    cout << "py: " << particle.p[1] << "\n";
    cout << "pz: " << particle.p[2] << "\n";
    cout << "hvx: " << particle.hv[0] << "\n";
    cout << "hvy: " << particle.hv[1] << "\n";
    cout << "hvz: " << particle.hv[2] << "\n";
    cout << "vx: " << particle.v[0] << "\n";
    cout << "vy: " << particle.v[1] << "\n";
    cout << "vz: " <<particle.v[2] << "\n";

    }


  }

  output_file.close();

  return 0;
}



void check_np (int np){
  if (np <= 0) {
    string errorMsg = "Error: Invalid number of particles: " + to_string(np) + ".\n";
    throw runtime_error(errorMsg);
  }
}



void refactor_gordo (const char * in_file, Malla& malla, vector<Particle>& particles) {
  ifstream input_file(in_file, ios::binary);
  double trash;
  input_file.read(reinterpret_cast<char *>(&trash), sizeof(double));
  array<array<float, 3>, 3> info_particle;
  array<array<double, 3>, 3> info_particle_double;
  int counter = 0;

  while (input_file.read(reinterpret_cast<char *>(&info_particle[0][0]), sizeof(info_particle[0][0]))) {
    // if i < np then read the next 8 floats, else continue
    input_file.read(reinterpret_cast<char *>(&info_particle[0][1]), sizeof(info_particle[0][1]));
    input_file.read(reinterpret_cast<char *>(&info_particle[0][2]), sizeof(info_particle[0][2]));
    input_file.read(reinterpret_cast<char *>(&info_particle[1][0]), sizeof(info_particle[1][0]));
    input_file.read(reinterpret_cast<char *>(&info_particle[1][1]), sizeof(info_particle[1][1]));
    input_file.read(reinterpret_cast<char *>(&info_particle[1][2]), sizeof(info_particle[1][2]));
    input_file.read(reinterpret_cast<char *>(&info_particle[2][0]), sizeof(info_particle[2][0]));
    input_file.read(reinterpret_cast<char *>(&info_particle[2][1]), sizeof(info_particle[2][1]));
    input_file.read(reinterpret_cast<char *>(&info_particle[2][2]), sizeof(info_particle[2][2]));

    for (size_t i = 0; i < info_particle.size(); ++i) {
      for (size_t j = 0; j < info_particle[i].size(); ++j) {
        info_particle_double[i][j] = static_cast<double>(info_particle[i][j]);
      }
    }


    array<int, 3> index_array = calculate_block_indexes(info_particle_double[0], malla);
    // Linear mapping para encontrar el bloque correcto
    int index = index_array[0] + index_array[1] * malla.n_blocks[0] + index_array[2] * malla.n_blocks[0] * malla.n_blocks[1];
    insert_particle_info(info_particle_double,malla.blocks[index],counter, index, particles);

    counter++;

  }

  check_missmatch_particles(counter, malla.np);
}


array<int, 3> calculate_block_indexes(array <double,3> positions, Malla& malla){
  int i_index=0;
  int j_index=0;
  int k_index=0;
  i_index = initial_block_index(positions[0], xmin,  malla.size_blocks[0]);
  j_index = initial_block_index(positions[1], ymin,  malla.size_blocks[1]);
  k_index = initial_block_index(positions[2], zmin,  malla.size_blocks[2]);
  /* TODO problema coma flotante floor error */
  if (i_index<0){i_index = 0;}
  if (j_index<0){j_index = 0;}
  if (k_index<0){k_index = 0;}
  if (i_index >= malla.n_blocks[0]) {i_index = malla.n_blocks[0] - 1;}
  if (j_index >= malla.n_blocks[1]){j_index = malla.n_blocks[1] - 1;}
  if (k_index >= malla.n_blocks[2]){k_index = malla.n_blocks[2] - 1;}
  return array<int, 3>{i_index,j_index,k_index};

}

void insert_particle_info(array<array<double, 3>, 3> info, Block& bloque, int id, int block_index, vector<Particle>& particles){
  Particle particle{};
  particle.p = info[0];
  particle.hv = info[1];
  particle.v = info[2];
  particle.a = {0,0,0};
  particle.rho = 0;
  particle.id = id;
  particle.current_block = block_index;
  bloque.particles.emplace_back(id);
  particles.emplace_back(particle);
}

void check_missmatch_particles(int counter, int malla_np) {
  if (counter != malla_np) {
    string errorMsg =
        "Error: Number of particles mismatch. Header: " + to_string(malla_np) + ", Found: " + to_string(counter) +
        ".\n";
    throw runtime_error(errorMsg);
  }
}


