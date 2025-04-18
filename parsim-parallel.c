#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define G 6.67408e-11 
#define EPSILON2 (0.005 * 0.005)
#define DELTAT 0.1 

typedef struct {
    double x, y;
    double vx, vy;
    double m;
} particle_t;

unsigned int seed;

void init_r4uni(int input_seed) {
    seed = input_seed + 987654321;
}

double rnd_uniform01() {
    int seed_in = seed;
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    return 0.5 + 0.2328306e-09 * (seed_in + (int) seed);
}

double rnd_normal01() {
    double u1, u2, z, result;
    do {
        u1 = rnd_uniform01();
        u2 = rnd_uniform01();
        z = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
        result = 0.5 + 0.15 * z;
    } while (result < 0 || result >= 1);
    return result;
}

void init_particles(long userseed, double side, long ncside, long long n_part, particle_t *par) {
    double (*rnd01)() = rnd_uniform01;
    if (userseed < 0) {
        rnd01 = rnd_normal01;
        userseed = -userseed;
    }
    init_r4uni(userseed);

    for (long long i = 0; i < n_part; i++) {
        par[i].x = rnd01() * side; 
        par[i].y = rnd01() * side;
        par[i].vx = (rnd01() - 0.5) * side / ncside / 5.0;
        par[i].vy = (rnd01() - 0.5) * side / ncside / 5.0;
        par[i].m = rnd01() * 0.01 * (ncside * ncside) / n_part / G * EPSILON2;
    }
}

void compute_forces(particle_t *par, long long n_part) {
    #pragma omp parallel for
    for (long long i = 0; i < n_part; i++) {
        double fx = 0.0, fy = 0.0;
        for (long long j = 0; j < n_part; j++) {
            if (i != j) {
                double dx = par[j].x - par[i].x;
                double dy = par[j].y - par[i].y;
                double dist2 = dx * dx + dy * dy + EPSILON2;
                double inv_dist3 = 1.0 / (dist2 * sqrt(dist2));
                fx += G * par[i].m * par[j].m * dx * inv_dist3;
                fy += G * par[i].m * par[j].m * dy * inv_dist3;
            }
        }
        par[i].vx += (fx / par[i].m) * DELTAT;
        par[i].vy += (fy / par[i].m) * DELTAT;
    }
}

void update_positions(particle_t *par, long long n_part, double side) {
    #pragma omp parallel for
    for (long long i = 0; i < n_part; i++) {
        par[i].x += par[i].vx * DELTAT;
        par[i].y += par[i].vy * DELTAT;
        if (par[i].x < 0) par[i].x += side;
        if (par[i].x >= side) par[i].x -= side;
        if (par[i].y < 0) par[i].y += side;
        if (par[i].y >= side) par[i].y -= side;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Uso: %s <seed> <side> <ncside> <n_part> <steps>\n", argv[0]);
        return 1;
    }

    long userseed = atol(argv[1]);
    double side = atof(argv[2]);
    long ncside = atol(argv[3]);
    long long n_part = atoll(argv[4]);
    long steps = atol(argv[5]);

    particle_t *particles = malloc(n_part * sizeof(particle_t));
    if (!particles) {
        fprintf(stderr, "Erro ao alocar memoria\n");
        return 1;
    }

    init_particles(userseed, side, ncside, n_part, particles);

    char **collided = malloc(n_part * sizeof(char *));
    for (long long i = 0; i < n_part; i++) {
        collided[i] = calloc(n_part, sizeof(char));
    }

    long collisions = 0;
    for (long step = 0; step < steps; step++) {
        compute_forces(particles, n_part);
        update_positions(particles, n_part, side);

        #pragma omp parallel for reduction(+:collisions)
        for (long long i = 0; i < n_part; i++) {
            for (long long j = i + 1; j < n_part; j++) {
                if (!collided[i][j]) {
                    double dx = particles[j].x - particles[i].x;
                    double dy = particles[j].y - particles[i].y;
                    double dist2 = dx * dx + dy * dy;
                    if (dist2 < EPSILON2) {
                        collisions++;
                        collided[i][j] = 1;
                    }
                }
            }
        }
    }

    for (long long i = 0; i < n_part; i++) {
        free(collided[i]);
    }
    free(collided);

    printf("%.3lf %.3lf\n", particles[0].x, particles[0].y);
    printf("%ld\n", collisions);

    free(particles);
    return 0;
}