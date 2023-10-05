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





int validate_parameters (int argc, const char * argv[]);
int validate_time_steps (int nts);
int validate_input_file(const char* inputFileName);
int validate_output_file(const char* outputFileName);
Malla read_input_file(const char * in_file);
void refactor_gordo (const char * in_file, Constants cons, Malla malla);
void check_np(int np);
array<double, 3> check_inside_grid(array<double, 3> position);
int write_output_file (Malla malla, const char * out_file);
array<int, 3> calculate_block_indexes(array <double,3> positions, Constants cons);
Block insert_particle_info(array<array<double, 3>, 3> info, Block bloque, int id);
void missmatch_particles(int counter, int malla_np);



#endif //ARQUITECTURA_DE_COMPUTADORES_PROGARGS_HPP


