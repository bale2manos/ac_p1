//
// Created by bale2 on 26/09/2023.
//

#ifndef ARQUITECTURA_DE_COMPUTADORES_PROGARGS_HPP
#define ARQUITECTURA_DE_COMPUTADORES_PROGARGS_HPP

#include <fstream>
#include <vector>
#include <array>
#include "grid.hpp"
using namespace std;





std::array<int,2> validate_parameters (int argc, const char * argv[]);
int validate_time_steps (int nts);
int validate_input_file(const char* inputFileName);
int validate_output_file(const char* outputFileName);
void read_input_file (Malla& malla, const char * in_file);
void refactor_gordo (const char * in_file, Malla& malla);
void check_np(int np);
int write_output_file (Malla& malla, const char * out_file);
std::array<int, 3> calculate_block_indexes(std::array <double,3> positions, Malla& malla);
void insert_particle_info(std::array<std::array<double, 3>, 3> info, Block& bloque, int id, int block_index);
void check_missmatch_particles(int counter, int malla_np);



#endif //ARQUITECTURA_DE_COMPUTADORES_PROGARGS_HPP


