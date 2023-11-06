//
// Created by bale2 on 26/09/2023.
//
#include "grid.hpp"
#include "progargs.hpp"
# define M_PI           3.14159265358979323846  /* pi */
#include <math.h>
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
    malla.blocks = {};
        // Create the blocks and append them to Malla. Create all vectors in nx, ny, nz
    for (int k = 0; k < malla.n_blocks[2]; ++k) {
        for (int j = 0; j < malla.n_blocks[1]; ++j) {
            for (int i = 0; i < malla.n_blocks[0]; ++i) {
                std::array<int, 3> const indexes = {i,j,k};
                malla.blocks.push_back(Block(indexes, {}, check_neighbours({i,j,k}, malla.n_blocks)));
            }
        }
    }
}

/*

double increase_density(array<double, 3> pivot_coords, array<double, 3> particle2_coords, double h){
    double const norm_squared = pow((pivot_coords[0] - particle2_coords[0]),2) + pow((pivot_coords[1] - particle2_coords[1]),2)
            + pow((pivot_coords[2] - particle2_coords[2]),2);
    double const h_squared = pow(h,2);
    if (norm_squared < h_squared){
        return pow(h_squared-norm_squared,3);
    }
    return 0;
}
 */

double increase_density(array<double, 3> pivot_coords, array<double, 3> particle2_coords, double h) {
    double const h_squared = h * h;
    double const dx = pivot_coords[0] - particle2_coords[0];
    double const dy = pivot_coords[1] - particle2_coords[1];
    double const dz = pivot_coords[2] - particle2_coords[2];
    double const norm_squared = dx * dx + dy * dy + dz * dz;

    return (norm_squared < h_squared) ? pow(h_squared - norm_squared, 3) : 0;
}


double density_transformation(double rho,double h, double m){
    double first_term = (rho + pow(h,6));
    double second_term = (315/(64*M_PI*pow(h,9)));
    return first_term*second_term*m;
}


array<double,3> acceleration_transfer(Particle& pivot, Particle& particle2, double h, array<double,2> acc_const){
    //Aquí calculamos los términos por separado de la ecuación grande.
    double const norm_squared = pow((pivot.p[0] - particle2.p[0]),2) + pow((pivot.p[1] - particle2.p[1]),2)
                        + pow((pivot.p[2] - particle2.p[2]),2);

    if (norm_squared>=pow(h,2)){return {0,0,0};}
    array<double, 3> acc_increase = {0.0, 0.0, 0.0};
    double const distij = sqrt(max(norm_squared,pow(10,-12)));
    // aqui estaba aux

    double const term2 = pow(h-distij,2)/distij;

    double density_pivot = pivot.rho;
    double density_2 = particle2.rho;
    double const term3 = (density_pivot + density_2 - (2*rho_f));
    double const denominator = (density_2  * density_pivot);

    double const numerator1 = acc_const[0]*term2*term3;


    for (int i=0; i<3; i++){
        double const term1 = (pivot.p[i]-particle2.p[i]);
        double const term4 = (particle2.v[i] - pivot.v[i]);
        acc_increase[i] = ((term1*numerator1) + (term4*acc_const[1]))/denominator;

        }

    return acc_increase;
  }


/*TODO ampersand block? */
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
  double min_limit = NAN;
  double max_limit = NAN;
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

    double delta_t_squared = pow(delta_t,2);
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





void update_grid(Malla& malla, vector<Particle>& new_particles){
    for (Block & block : malla.blocks) {
      block.particles.clear();
    }
    for (Particle  const& particle: new_particles){
      malla.blocks[particle.current_block].particles.emplace_back(particle);
    }
}

void densinc_old(Malla& malla){
    vector<Particle> all_iterated_particles;
    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            Particle particle_updated = particle_pivot;
            for (auto index: block.neighbours) {
                for (Particle & particle2 : malla.blocks[index].particles) {
                    if (particle_updated.id == particle2.id || particle_updated.id<particle2.id) { continue; }
                    double const increase_d_factor =
                        increase_density(particle_updated.p, particle2.p, malla.h);
                    particle_updated.rho = particle_updated.rho + increase_d_factor;
                }
            }
            all_iterated_particles.push_back(particle_updated);
      }
    }
    update_grid(malla, all_iterated_particles);
}

void densinc(Malla& malla){
    //vector<Particle> all_iterated_particles;
    //vector<double> new_densities(malla.np, 0);

    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            //Particle particle_updated = particle_pivot
            for (auto index: block.neighbours) {
                Block& neighbour_block = malla.blocks[index];
                for (Particle & particle2 : neighbour_block.particles) {
                    // TODO esto optimiza? // TODO extraer metodo de check colisions?
                    // int particle1_id = particle_updated.id; particle2_id = particle2.id;
                    if (particle_pivot.id <= particle2.id) { continue; }
                    double const increase_d_factor =
                        increase_density(particle_pivot.p, particle2.p, malla.h);
                    //new_densities[particle_pivot.id] += increase_d_factor;
                    //new_densities[particle2.id] += increase_d_factor;
                    particle_pivot.rho += increase_d_factor;
                    particle2.rho += increase_d_factor;
                }
            }
      }
    }

    //for (Block & block : malla.blocks) {
    //  for (Particle & particle : block.particles) { particle.rho = new_densities[particle.id]; }
    //}
}



void denstransf(Malla& malla){
    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            particle_pivot.rho = density_transformation(particle_pivot.rho, malla.h, malla.m);
      }
    }
}


void acctransf(Malla& malla){
    //vector<Acceleration> new_accelerations(malla.np, {0,g,0});

    double const h_value = malla.h;
    array<double,2> const acc_constants = malla.acc_const;

    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            int const pivot_id = particle_pivot.id;
            for (auto index: block.neighbours) {
                Block& neighbour_block = malla.blocks[index];
                for (Particle & particle2 : neighbour_block.particles) {
                    int const particle2_id = particle2.id;
                    if (pivot_id > particle2_id) {continue;}

                    array<double,3> acc_incr = acceleration_transfer(particle_pivot,particle2,h_value,acc_constants);
                    /*
                    new_accelerations[pivot_id].accx += acc_incr[0];
                    new_accelerations[pivot_id].accy += acc_incr[1];
                    new_accelerations[pivot_id].accz += acc_incr[2];
                    new_accelerations[particle2_id].accx -= acc_incr[0];
                    new_accelerations[particle2_id].accy -= acc_incr[1];
                    new_accelerations[particle2_id].accz -= acc_incr[2];
                    */
                    particle_pivot.a[0] += acc_incr[0];
                    particle_pivot.a[1] += acc_incr[1];
                    particle_pivot.a[2] += acc_incr[2];
                    particle2.a[0] -= acc_incr[0];
                    particle2.a[1] -= acc_incr[1];
                    particle2.a[2] -= acc_incr[2];
                }
            }
      }
    }

    /*
    for (Block & block : malla.blocks) {
      for (Particle & particle : block.particles) {
            int const particle_id=particle.id;
            // TODO creo que PARTICLE puede ser un struct Acceleration
            particle.a[0] = new_accelerations[particle_id].accx;
            particle.a[1] = new_accelerations[particle_id].accy;
            particle.a[2] = new_accelerations[particle_id].accz;
      }
    }
    */

}

void partcol(Malla& malla){
    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            if (block.coords[0] == 0||block.coords[1]==0||block.coords[2]==0||
                block.coords[0]==malla.n_blocks[0]-1||block.coords[1]==malla.n_blocks[1]-1||block.coords[2]==malla.n_blocks[2]-1){
                wall_colissions(particle_pivot, block, malla.n_blocks);
            }
      }
    }
}

void motion(Malla& malla){
    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            particle_movement(particle_pivot);
      }
    }
}

void boundint(Malla& malla){
    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            if (block.coords[0] == 0||block.coords[1]==0||block.coords[2]==0||
                block.coords[0]==malla.n_blocks[0]-1||block.coords[1]==malla.n_blocks[1]-1||block.coords[2]==malla.n_blocks[2]-1){
                limits_interaction(particle_pivot, block, malla.n_blocks);
            }
      }
    }
}


void triplete(Malla &malla){
    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            if (block.coords[0] == 0||block.coords[1]==0||block.coords[2]==0||
                block.coords[0]==malla.n_blocks[0]-1||block.coords[1]==malla.n_blocks[1]-1||block.coords[2]==malla.n_blocks[2]-1){
                wall_colissions(particle_pivot, block, malla.n_blocks);
            }
            particle_movement(particle_pivot);
            if (block.coords[0] == 0||block.coords[1]==0||block.coords[2]==0||
                block.coords[0]==malla.n_blocks[0]-1||block.coords[1]==malla.n_blocks[1]-1||block.coords[2]==malla.n_blocks[2]-1){
                limits_interaction(particle_pivot, block, malla.n_blocks);
            }
      }
    }
}


void repos_old(Malla& malla){
    // Iterar por todos los bloques de la malla
    for (size_t current_block= 0; current_block < malla.blocks.size(); current_block++) {
      // Iterar por todas las particulas del bloque
      for (size_t current_part= 0; current_part < malla.blocks[current_block].particles.size(); current_part++){
            // Calcular el nuevo posible bloque de la particula
            array<int,3> const new_indexes =
                calculate_block_indexes(malla.blocks[current_block].particles[current_part].p, malla);
            int const block_index = calculate_block_index(new_indexes,malla.n_blocks[0],malla.n_blocks[1]);
            auto new_block = static_cast<size_t>(block_index);
            int const part_old = static_cast<int>(current_part);

            // Si el bloque es distinto al actual, mover la particula
            if (new_block != current_block){
                if (malla.blocks[current_block].particles[part_old].id == 0){
                    cout << "NEW: " << new_block << "OLD: " << current_block<< "\n";
                }
                malla.blocks[new_block].particles.push_back(malla.blocks[current_block].particles[part_old]);
                malla.blocks[current_block].particles.erase(malla.blocks[current_block].particles.begin() + part_old);
            }
      }
    }
}

void initacc(Malla& malla){
    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            particle_pivot.a = {0, g, 0};
            particle_pivot.rho = 0;
      }
    }
}


void repos(Malla& malla){
    vector<Particle> particles;
    for (Block & block : malla.blocks) {
      for (Particle & particle_pivot : block.particles) {
            particles.push_back(particle_pivot);
      }
      block.particles.clear();
    }

    for (Particle & particle : particles) {
      array<int,3> const new_indexes =
              calculate_block_indexes(particle.p, malla);
      int const block_index = calculate_block_index(new_indexes,malla.n_blocks[0],malla.n_blocks[1]);
      particle.current_block = block_index;
      auto new_block = static_cast<size_t>(block_index);
      malla.blocks[new_block].particles.push_back(particle);
    }


}







void malla_interaction(Malla& malla){
    auto startTimeTotal = std::chrono::high_resolution_clock::now();

    repos(malla);

    initacc(malla);

    auto startTime = std::chrono::high_resolution_clock::now();
    densinc(malla);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << "TIEMPO densinc: " << duration.count() << " microsegundos" << "\n";

    denstransf(malla);

    startTime = std::chrono::high_resolution_clock::now();
    acctransf(malla);
    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << "TIEMPO acctransf: " << duration.count() << " microsegundos" << "\n";

    //partcol(malla);

    //motion(malla);

    //boundint(malla);

    triplete(malla);

    auto endTimeTotal = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(endTimeTotal - startTimeTotal);
    std::cout << "TIEMPO ITERACION: " << duration.count() << " microsegundos" << "\n";
    cout << "**************************************" << "\n";
}