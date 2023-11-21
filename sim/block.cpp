//
// Created by bale2 on 26/09/2023.
//

#include "block.hpp"

#include <cmath>
#include <iostream>

/**
 * Calculates the index of the block that contains a given position, based on the minimum limit and
 * block size.
 * @param position The position to calculate the block index for.
 * @param min_limit The minimum limit of the range of positions.
 * @param block_size The size of each block.
 * @return The index of the block that contains the given position.
 */
int initial_block_index(double position, double min_limit, double block_size) {
  // sumamos min_limit para que el bloque 0 sea el bloque que contiene a min_limit
  return floor((position - min_limit) / block_size);
}

/**
 * Calculates the index of a block in a 3D array given its coordinates and the size of the first two
 * dimensions.
 * @param coords The coordinates of the block in the format {x, y, z}.
 * @param nx The size of the first dimension.
 * @param ny The size of the second dimension.
 * @return The index of the block in the array.
 */
int calculate_block_index(std::array<int, 3> coords, int nx, int ny) {
  return coords[0] + coords[1] * nx + coords[2] * nx * ny;
}

/**
 * Constructor for Block class.
 * @param block_coords An array with the coordinates of the block.
 * @param n_blocks An array with the number of blocks in each dimension.
 */
Block::Block(std::array<int, 3> const & block_coords, std::array<int, 3> const & n_blocks)
  : coords(block_coords) {
  neighbours = check_neighbours(n_blocks);
}

/**
 * Calculates the neighbours of a block in a 3D grid.
 * @param n_blocks An array with the number of blocks in each dimension of the grid.
 * @return A vector with the indices of the neighbouring blocks.
 */
std::vector<int> Block::check_neighbours(std::array<int, 3> const & n_blocks) {

  // El bloque en el bloque en el bloque en el mismo de nuevo.
  for (int k_aux = -1; k_aux < 2; k_aux++) {
    for (int j_aux = -1; j_aux < 2; j_aux++) {
      for (int i_aux = -1; i_aux < 2; i_aux++) {
        if (coords[0] + i_aux > -1 && coords[0] + i_aux < n_blocks[0] && coords[1] + j_aux > -1 &&
            coords[1] + j_aux < n_blocks[1] && coords[2] + k_aux > -1 &&
            coords[2] + k_aux < n_blocks[2]) {
          neighbours.push_back(
              calculate_block_index({coords[0] + i_aux, coords[1] + j_aux, coords[2] + k_aux},
                                    n_blocks[0], n_blocks[1]));
        }
        }
      }
    }
  return neighbours;
}
