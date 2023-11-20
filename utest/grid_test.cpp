#include "grid_test.hpp"

#include "sim/grid.hpp"

#include <gtest/gtest.h>

Malla const & getGrid() {
  static Malla const grid(nnp, ppm);
  return grid;
}

TEST(GridTest, smoothlenght) {
  Malla const & grid  = getGrid();
  double const result = grid.smooth_length_calc();
  EXPECT_DOUBLE_EQ(result, 0.0058047945205479453);
}

TEST(GridTest, nxcalc) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.nx_calc(84, 25, 20), 2);
}

TEST(GridTest, nycalc) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.ny_calc(50, 25, 20), 1);
}

TEST(GridTest, nzcalc) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.nx_calc(401, 100, 20), 15);
}

TEST(GridTest, sx_calc) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.sx_calc(100, 50, 20), 2.5);
}

TEST(GridTest, sy_calc) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.sy_calc(200, 50, 20), 7.5);
}

TEST(GridTest, sz_calc) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.sz_calc(1000, 350, 10), 65);
}

TEST(GridTest, particle_mass) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.particle_mass_calc(), 4.0165339818054222e-05);
}

TEST(GridTest, createFillGridsize) {
  Malla grid                      = getGrid();
  std::vector<Block> const blocks = grid.createFillGrid();
  EXPECT_EQ(blocks.size(), 30008);
}

TEST(GridTest, density_transformation) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.density_transformation(1000), 8.4091188953149706e+18);
}

TEST(GridTest, get_block_index_from_position) {
  Malla grid                           = getGrid();
  std::array<double, 3> const position = {2, 3, 4};
  EXPECT_DOUBLE_EQ(grid.get_block_index_from_position(position), 15003);
}

TEST(GridTest, increase_density_noincrease) {
  Malla const & grid                     = getGrid();
  double const pivot_x                   = 2.0;
  double const pivot_y                   = 3.0;
  double const pivot_z                   = 4.0;
  double const particle_x                = 3.0;
  double const particle_y                = 1.0;
  double const particle_z                = 3.0;
  std::array<double, 3> pivot_coords     = {pivot_x, pivot_y, pivot_z};
  std::array<double, 3> particle2_coords = {particle_x, particle_y, particle_z};
  double const pivot_rho_ori             = 1.0;
  double const particle2_rho_ori         = 2.0;
  double pivot_rho                       = 1.0;
  double particle2_rho                   = 2;
  grid.increase_density(pivot_coords, particle2_coords, pivot_rho, particle2_rho);
  EXPECT_DOUBLE_EQ(pivot_rho, pivot_rho_ori);  // Los valores son iguales porque norm_squared > h_squared
  EXPECT_DOUBLE_EQ(particle2_rho, particle2_rho_ori);
}

TEST(GridTest, increase_density_increase) {
  Malla const & grid                     = getGrid();
  std::array<double, 3> pivot_coords     = {1, 2, 3};
  std::array<double, 3> particle2_coords = {1, 2, 3};
  double const pivot_rho_ori             = 1.0;
  double const particle2_rho_ori         = 2.0;
  double pivot_rho                       = 1.0;
  double particle2_rho                   = 2;
  grid.increase_density(pivot_coords, particle2_coords, pivot_rho, particle2_rho);
  EXPECT_GT(pivot_rho, pivot_rho_ori);  // Los valores NO son iguales porque norm_squared <
                                        // h_squared y por tanto hay incremento
  EXPECT_GT(particle2_rho, particle2_rho_ori);
}

TEST(GridTest, calculate_block_index) {
  Malla grid                            = getGrid();
  std::array<double, 3> const positions = {1, 2, 3};
  std::array<int, 3> const result       = {21, 30, 21};
  EXPECT_EQ(grid.calculate_block_indexes(positions), result);
}

TEST(GridTest, limit_of_edge) {
  Malla const & grid = getGrid();
  EXPECT_DOUBLE_EQ(grid.limit_of_edge(0, 0), -0.065);
  EXPECT_DOUBLE_EQ(grid.limit_of_edge(0, 1), 0.065);
  EXPECT_DOUBLE_EQ(grid.limit_of_edge(1, 0), -0.08);
  EXPECT_DOUBLE_EQ(grid.limit_of_edge(1, 1), 0.1);
  EXPECT_DOUBLE_EQ(grid.limit_of_edge(2, 0), -0.065);
  EXPECT_DOUBLE_EQ(grid.limit_of_edge(2, 1), 0.065);
}

TEST(GridTest, edge_collisions) {
  Particle particle{
    {1, 2, 3}, // pos
    {1, 2, 3}, // h_vel
    {1, 2, 3}, // vel
    {1, 2, 3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {1, 2, 3}, // pos
    {1, 2, 3}, // h_vel
    {1, 2, 3}, // vel
    {1, 2, 3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Malla::edge_collisions(particle, 1, 2);
  EXPECT_LT(particle.acc.at(2), particle2.acc.at(2));
}

TEST(GridTest, edge_collisions2) {
  Particle particle{
    {e12m,   e12m, e12m}, // pos
    {   1, minust,    3}, // h_vel
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {e12m,   e12m, e12m}, // pos
    {   1, minust,    3}, // h_vel
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Malla::edge_collisions(particle, 0, 1);
  EXPECT_GT(particle.acc.at(1), particle2.acc.at(1));
}

TEST(GridTest, particle_movement) {
  Particle particle{
    {1, 2, 3}, // pos
    {1, 2, 3}, // h_vel
    {1, 2, 3}, // vel
    {1, 2, 3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {1, 2, 3}, // pos
    {1, 2, 3}, // h_vel
    {1, 2, 3}, // vel
    {1, 2, 3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Malla::particle_movement(particle);
  EXPECT_NE(particle.pos.at(2), particle2.pos.at(2));
  EXPECT_NE(particle.h_vel.at(2), particle2.h_vel.at(2));
  EXPECT_NE(particle.vel.at(2), particle2.vel.at(2));
}

TEST(GridTest, limits_interactions1) {
  Malla grid                            = getGrid();
  std::array<int, 3> const block_coords = {0, 1, 2};
  std::array<int, 3> const n_blocks     = {1, 2, 3};
  Block block(block_coords, n_blocks);
  Particle particle{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  grid.limits_interaction(particle, block);
  EXPECT_DOUBLE_EQ(particle.acc.at(0), particle2.acc.at(0));
}

TEST(GridTest, limits_interactions2) {
  Malla grid                            = getGrid();
  std::array<int, 3> const block_coords = {1, 0, 2};
  std::array<int, 3> const n_blocks     = {1, 2, 3};
  Block block(block_coords, n_blocks);
  Particle particle{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  grid.limits_interaction(particle, block);
  EXPECT_DOUBLE_EQ(particle.acc.at(1), particle2.acc.at(1));
}

TEST(GridTest, limits_interactions3) {
  Malla grid                            = getGrid();
  std::array<int, 3> const block_coords = {1, 1, 0};
  std::array<int, 3> const n_blocks     = {1, 2, 3};
  Block block(block_coords, n_blocks);
  Particle particle{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  grid.limits_interaction(particle, block);
  EXPECT_DOUBLE_EQ(particle.acc.at(2), particle2.acc.at(2));
}

TEST(GridTest, wall_interactions1) {
  Malla grid                            = getGrid();
  std::array<int, 3> const block_coords = {0, 1, 2};
  std::array<int, 3> const n_blocks     = {1, 2, 3};
  Block block(block_coords, n_blocks);
  Particle particle{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  grid.wall_colissions(particle, block);
  EXPECT_DOUBLE_EQ(particle.acc.at(0), particle2.acc.at(0));
}

TEST(GridTest, wall_interactions2) {
  Malla grid                            = getGrid();
  std::array<int, 3> const block_coords = {1, 0, 2};
  std::array<int, 3> const n_blocks     = {1, 2, 3};
  Block block(block_coords, n_blocks);
  Particle particle{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  grid.wall_colissions(particle, block);
  EXPECT_GT(particle.acc.at(1), particle2.acc.at(1));
}

TEST(GridTest, wall_interactions3) {
  Malla grid                            = getGrid();
  std::array<int, 3> const block_coords = {1, 1, 0};
  std::array<int, 3> const n_blocks     = {1, 2, 3};
  Block block(block_coords, n_blocks);
  Particle particle{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  Particle particle2{
    {   1, minust,    3}, // h_vel
    {e12m,   e12m, e12m}, // pos
    {   1,      2,    3}, // vel
    {   1,      2,    3}, // acc
    0.0, // density
    0, // id
    0  // current_block
  };
  grid.wall_colissions(particle, block);
  EXPECT_DOUBLE_EQ(particle.acc.at(2), particle2.acc.at(2));
}
