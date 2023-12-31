//
// Created by bale2 on 26/09/2023.
//
#include "grid.hpp"

#include <math.h>
#include "progargs.hpp"
# define M_PI           3.14159265358979323846  /* pi */
#include <cmath>
#include <unordered_map>
#include <iostream>
#include <chrono>

void calculate_constants(double ppm, int np, Malla& malla){
    malla.ppm = ppm;
    malla.np = np;
    malla.h = smooth_length(ppm);
    malla.m = particle_mass(ppm);
    malla.n_blocks[0] = nx_calc(xmax, xmin, malla.h);
    malla.n_blocks[1] = ny_calc(ymax, ymin, malla.h);
    malla.n_blocks[2] = nz_calc(zmax, zmin, malla.h);
    malla.size_blocks[0] = sx_calc(xmax, xmin, malla.n_blocks[0]);
    malla.size_blocks[1] = sy_calc(ymax, ymin, malla.n_blocks[1]);
    malla.size_blocks[2] = sz_calc(zmax, zmin, malla.n_blocks[2]);
    double const acc_aux =M_PI*pow(malla.h,6);
    malla.acc_const[0] = 15/acc_aux * (3*malla.m*p_s)*0.5;
    malla.acc_const[1] = 15/acc_aux*3*mu*malla.m;



}



int nx_calc (double x_max, double x_min, double h_param){
    return floor( (x_max - x_min) / h_param);
}
int ny_calc (double y_max, double y_min, double h_param){
    return floor( (y_max - y_min) / h_param);
}
int nz_calc (double z_max, double z_min, double h_param){
    return floor( (z_max - z_min) / h_param);
}

double smooth_length (double ppm){
    return static_cast<double>(radio) / ppm;
}

double sx_calc (double x_max, double x_min, int n_x){
    return (x_max-x_min)/n_x;
}

double sy_calc (double y_max, double y_min, int n_y){
    return (y_max-y_min)/n_y;
}

double sz_calc (double z_max, double z_min, int n_z){
    return (z_max-z_min)/n_z;
}

double particle_mass (double ppm){
    return static_cast<double>(rho_f)/(pow (ppm,3));
}

void create_fill_grid(Malla& malla, int np,double ppm){
    //Malla malla(np, ppm, vector<Block>(),nx,ny,nz,h,m,sx,sy,sz);
    calculate_constants(ppm, np, malla);
    array<int,3> const n_blocks = malla.n_blocks;
        // Create the blocks and append them to Malla. Create all vectors in nx, ny, nz

    for (int k = 0; k < n_blocks[2]; ++k) {
        for (int j = 0; j < n_blocks[1]; ++j) {
            for (int i = 0; i < n_blocks[0]; ++i) {
                std::array<int, 3> const indexes = {i,j,k};
                Block block;
                block.coords = indexes;
                block.particles = {};
                block.neighbours = check_neighbours(indexes, n_blocks);
                malla.blocks.emplace_back(block);
            }
        }
    }
}


void increase_density(array<double, 3>& pivot_coords, array<double, 3>& particle2_coords, double h, double& pivot_rho, double& particle2_rho) {
    const static auto h_squared = h * h;
    double const dx_differ = pivot_coords[0] - particle2_coords[0];
    double const dy_differ = pivot_coords[1] - particle2_coords[1];
    double const dz_differ = pivot_coords[2] - particle2_coords[2];
    double const norm_squared = dx_differ * dx_differ + dy_differ * dy_differ + dz_differ * dz_differ;
    if (norm_squared < h_squared) {
        double const diff = (h_squared - norm_squared);
        double const increase =  diff *  diff * diff;
        pivot_rho += increase;
        particle2_rho += increase;
    }
}


double density_transformation(double rho, double h, double m){
    const static auto h_sixth = pow(h,6);
    const static auto h_ninth = pow(h,9);
    double const first_term = (rho + h_sixth);
    const static auto second_term = (315/(64*M_PI*h_ninth));
    return first_term*second_term*m;
}

void acceleration_transfer(Particle & pivot, Particle & particle2, double h,
                           const array<double, 2> & acc_const) {
    // Aquí calculamos los términos por separado de la ecuación grande.
    // Aquí calculamos los términos por separado de la ecuación grande.
    array<double,3>& position_pivot = pivot.p;
    array<double,3>& position_2 = particle2.p;


    double const differ_x = position_pivot[0] - position_2[0];
    double const differ_y = position_pivot[1] - position_2[1];
    double const differ_z = position_pivot[2] - position_2[2];

    double const norm = sqrt(
        differ_x * differ_x + differ_y * differ_y + differ_z * differ_z
    );

    if (norm <= h) {
        const static auto factor    = 1e-6;
        const static auto doble_rho = 2 * rho_f;
        double const distij         = max(norm, factor);
        double const difference_h_distij = h - distij;

        double const term2 = difference_h_distij*difference_h_distij / distij;
        // Sacamos todas las constantes fuera del bucle.
        double const density_pivot = pivot.rho;
        double const density_2     = particle2.rho;
        double const term3         = (density_pivot + density_2 - (doble_rho));
        double const denominator   = 1 / (density_2 * density_pivot);
        double const numerator1    = acc_const[0] * term2 * term3;

        for (int i = 0; i < 3; i++) {
            double const term1  = (pivot.p[i] - particle2.p[i]);
            double const term4  = (particle2.v[i] - pivot.v[i]);
            pivot.a[i]         += ((term1 * numerator1) + (term4 * acc_const[1])) * denominator;
            particle2.a[i]     -= ((term1 * numerator1) + (term4 * acc_const[1])) * denominator;
        }
    }
}


void wall_colissions(Particle& particle, Block& block, array<int,3>n_blocks){

  if (block.coords[0] == 0 || block.coords[0]==n_blocks[0]-1){
        edge_collisions(particle, block.coords[0], 0);
  }
  if (block.coords[1] == 0||block.coords[1] == n_blocks[1]-1) {
        edge_collisions(particle, block.coords[1], 1);
  }
  if (block.coords[2] == 0 || block.coords[2]==n_blocks[2]-1){
        edge_collisions(particle, block.coords[2], 2);
  }
}


void edge_collisions(Particle& particula, int extremo, int eje) {
  double min_limit = 0.0;
  double max_limit = 0.0;
  if (eje == 0){ min_limit= xmin; max_limit = xmax;}
  if (eje == 1){ min_limit= ymin; max_limit = ymax;}
  if (eje == 2){ min_limit= zmin; max_limit = zmax;}

  double const coord = particula.p[eje] + particula.hv[eje] * delta_t;
  if (extremo == 0) {
        double deltax = d_p - (coord - min_limit);
        if (deltax > pow(10, -10)) {
            particula.a[eje] = particula.a[eje] + (s_c * deltax - d_v * particula.v[eje]);
        }
  } else {
        double deltax = d_p - (max_limit - coord);
        if (deltax > pow(10, -10)) {
            particula.a[eje] = particula.a[eje] - (s_c * deltax + d_v * particula.v[eje]);
        }
  }
}


void particle_movement(Particle& particle){

    double delta_t_squared = delta_t * delta_t;
    double delta_t_half = delta_t*0.5;

    for(int i =0;i<3;i++){
        double hv_i = particle.hv[i];
        double acc_i = particle.a[i];

        particle.p[i] += (hv_i*delta_t) + (acc_i*delta_t_squared);
        particle.v[i] = hv_i + (acc_i*delta_t_half);
        particle.hv[i] += (acc_i*delta_t);
    }
}




void limits_interaction(Particle& particle, Block& block, array<int,3>n_blocks) {
    if (block.coords[0] == 0 || block.coords[0] == n_blocks[0] - 1) { edge_interaction(particle, block.coords[0], 0); }
    if (block.coords[1] == 0 || block.coords[1] == n_blocks[1] - 1) { edge_interaction(particle, block.coords[1], 1); }
    if (block.coords[2] == 0 || block.coords[2] == n_blocks[2] - 1) { edge_interaction(particle, block.coords[2], 2); }
}

void edge_interaction(Particle& particle,int extremo,int eje){
    double min_limit = 0;
    double max_limit = 0;
    if (eje == 0){ min_limit= xmin; max_limit = xmax;}
    if (eje == 1){ min_limit= ymin; max_limit = ymax;}
    if (eje == 2){ min_limit= zmin; max_limit = zmax;}
    double displacement_edge = 0.0;
    if (extremo == 0){
      displacement_edge = particle.p[eje] - min_limit;
      if(displacement_edge<0) {
            particle.p[eje] = min_limit - displacement_edge;
      }
    }
    else{
      displacement_edge = max_limit - particle.p[eje];
      if(displacement_edge<0) {
            particle.p[eje] = max_limit + displacement_edge;
      }
    }
    if(displacement_edge<0) {
        particle.v[eje]  *= -1;
        particle.hv[eje] *= -1;
    }
}


void densinc(Malla& malla, vector<Particle>& particles){
    double const  h_value = malla.h;
    double const  m_value = malla.m;
    
    for (Particle & particle_pivot: particles){
        int const particle_pivot_id = particle_pivot.id;
        Block & current_block =  malla.blocks[particle_pivot.current_block];
        vector<int>& neighbours_particles = current_block.neighbours_particles;
        for (auto particle2_id: neighbours_particles){
            if(particle_pivot_id < particle2_id){
                Particle & particle2 = particles[particle2_id];
                increase_density(particle_pivot.p, particle2.p, h_value, particle_pivot.rho, particle2.rho);
            }
        }
        particle_pivot.rho = density_transformation(particle_pivot.rho, h_value, m_value);
    }


}


void denstransf(Malla& malla, vector<Particle>& particles){
    double const h_value = malla.h;
    double const m_value = malla.m;
    for (Particle & particle: particles){
        particle.rho = density_transformation(particle.rho, h_value, m_value);
    }
}



void acctransf(Malla& malla, vector<Particle>& particles){
    double const h_value = malla.h;
    array<double,2> const acc_constants = malla.acc_const;
    for (Particle & particle_pivot: particles){
        int const particle_pivot_id = particle_pivot.id;
        Block const & current_block =  malla.blocks[particle_pivot.current_block];
        vector<int> const & neighbours_particles = current_block.neighbours_particles;
        for (auto particle2_id: neighbours_particles){
           if(particle_pivot_id < particle2_id){
               Particle & particle2 = particles[particle2_id];
               acceleration_transfer(particle_pivot, particle2, h_value, acc_constants);
           }
       }
    }
}




void triplete(Malla &malla, vector<Particle>& particles){
    array<int,3> const n_blocks = malla.n_blocks;
    for (Particle & particle_pivot : particles) {
        Block& block = malla.blocks[particle_pivot.current_block];
        bool extremo = false;

        if (block.coords[0] == 0||block.coords[1]==0||block.coords[2]==0||
            block.coords[0]==n_blocks[0]-1||block.coords[1]==n_blocks[1]-1||block.coords[2]==n_blocks[2]-1){
            extremo = true;
            wall_colissions(particle_pivot, block, n_blocks);
        }
        particle_movement(particle_pivot);
        if (extremo){
            limits_interaction(particle_pivot, block, n_blocks);
        }
    }
}



void repos(Malla& malla, vector<Particle>& particles){
    for (Block & block : malla.blocks) {
      block.particles.clear();
      block.neighbours_particles.clear();
    }

    for (Particle & particle : particles) {
      array<int,3> const new_indexes =
              calculate_block_indexes(particle.p, malla);
      int const block_index = calculate_block_index(new_indexes,malla.n_blocks[0],malla.n_blocks[1]);
      particle.current_block = block_index;

      // antiguo initacc
      particle.rho = 0;
      particle.a = {0, g, 0};
      // fin antiguo initacc

      auto new_block = static_cast<size_t>(block_index);
      malla.blocks[new_block].particles.emplace_back(particle.id);
    }

    int currentBlock = 0;
    for (Block & block: malla.blocks){
        for (auto index: block.neighbours){
            Block& neighbour_block = malla.blocks[index];
            for (auto particle_id: neighbour_block.particles){
                malla.blocks[currentBlock].neighbours_particles.emplace_back(particle_id);
            }
        }
        currentBlock++;
    }


}


void malla_interaction(Malla& malla, vector<Particle>& particles){
    auto startTimeTotal = std::chrono::high_resolution_clock::now();

    auto startTime = std::chrono::high_resolution_clock::now();
    repos(malla, particles);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << "TIEMPO repos: " << duration.count() << " microsegundos" << "\n";

    //initacc(malla);

    startTime = std::chrono::high_resolution_clock::now();
    densinc(malla, particles);
    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << "TIEMPO densinc: " << duration.count() << " microsegundos" << "\n";


    //denstransf(malla, particles);

    startTime = std::chrono::high_resolution_clock::now();
    acctransf(malla, particles);
    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << "TIEMPO acctransf: " << duration.count() << " microsegundos" << "\n";

    //partcol(malla);

    //motion(malla);

    //boundint(malla);

    triplete(malla, particles);

    auto endTimeTotal = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(endTimeTotal - startTimeTotal);
    std::cout << "TIEMPO ITERACION: " << duration.count() << " microsegundos" << "\n";
    cout << "***************************************" << "\n";
}