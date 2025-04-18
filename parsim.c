#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define G 6.67408e-11 
#define EPSILON2 (0.005 * 0.005)
#define DELTAT 0.1 

typedef struct {
    double x, y; //x e y representam a posição actual da particula
    double vx, vy; //componentes da velocidade nos eixos x e y
    double m; //massa da particula
} particle_t;

unsigned int seed;//variavel "semente" para gerar numeros aleatorios

//função inicializa a variavel global seed, consoante no input do usuário
void init_r4uni(int input_seed) {
    seed = input_seed + 987654321;
}

//função geradora de numeros aleatorios de 0 a 1 
double rnd_uniform01() {
    int seed_in = seed; //variavel auxiliar para seed (guardando o valor original de seed)
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    //retornando o numero aleatorio
    return 0.5 + 0.2328306e-09 * (seed_in + (int) seed);
}

//função geradora de numeros aleatorios seguindo uma distribuição normal, a volta do intervalo 0 e 1.
double rnd_normal01() {
    double u1, u2, z, result;
    do {
        u1 = rnd_uniform01(); //u1 recebe um numero aleatorio uniforme (0 e 1)
        u2 = rnd_uniform01(); //u2 recebe um numero aleatorio uniforme (0 e 1)
        z = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2); //valor gerado pela transformação de Box-Muller
        result = 0.5 + 0.15 * z; //numero ajustado para dentro do intervalo (0 e 1)
    } while (result < 0 || result >= 1); //mantendo o intervalo entre 0 e 1
    return result;
}

//função inicializa um conjunto de particulas com posições, velocidades e massas aleatórias dentro de um esppaço bidimensional
void init_particles(long userseed, double side, long ncside, long long n_part, particle_t *par) {
    //ponteiro que aponta para a função que gera os numeros de 0 a 1 de forma aleatoria, distribuida e uniforme  
    double (*rnd01)() = rnd_uniform01; 
    long long i;
    //caso o numero apresentado em userseed for negativo, será mudado para positivo automaticamente
    if (userseed < 0) {
        rnd01 = rnd_normal01;
        userseed = -userseed;
    }
    
    //função garante que a geração de numeros aleatórios seja reprodutível
    init_r4uni(userseed);

    //for que percorre todas as particulas e define os seus valores logo a seguir
    for (i = 0; i < n_part; i++) {
        //posição da particula no intervalo de 0 ao tamanho 'side' definido, para que as particulas não estejam fora do espaço definido, e geradas de forma aleatoria 
        par[i].x = rnd01() * side; 
        par[i].y = rnd01() * side;
        //velocidades das particulas em ambas as direcções de -0.5 a 0.5, geradas de forma aleatoria
        par[i].vx = (rnd01() - 0.5) * side / ncside / 5.0;
        par[i].vy = (rnd01() - 0.5) * side / ncside / 5.0;
        //gera a massa de forma aleatória
        par[i].m = rnd01() * 0.01 * (ncside * ncside) / n_part / G * EPSILON2;
    }
}

//função que calcula as forças gravitacionais entre as particulas e atualiza suas velocidades com base nessas forças 
void compute_forces(particle_t *par, long long n_part) {
    for (long long i = 0; i < n_part; i++) {
        double fx = 0.0, fy = 0.0; //iniciando as forças de cada particula
        //for percorre todas as outras particulas para calcular a força gravitacional que cada uma exerce sobre i 
        for (long long j = 0; j < n_part; j++) {
            if (i != j) {
                //calculo das distancias entre as particulas 
                double dx = par[j].x - par[i].x;
                double dy = par[j].y - par[i].y;
                double dist2 = dx * dx + dy * dy + EPSILON2;

                double inv_dist3 = 1.0 / (dist2 * sqrt(dist2));
                //componentes da força em x e y 
                fx += G * par[i].m * par[j].m * dx * inv_dist3;
                fy += G * par[i].m * par[j].m * dy * inv_dist3;
            }
        }
        //atualização das velocidades 
        par[i].vx += (fx / par[i].m) * DELTAT;
        par[i].vy += (fy / par[i].m) * DELTAT;
    }
}

//função atualiza as posições das particulas com base nas suas velocidades, garantindo permanencia dentro do dominio do espaço 
void update_positions(particle_t *par, long long n_part, double side) {
    for (long long i = 0; i < n_part; i++) {
        par[i].x += par[i].vx * DELTAT;
        par[i].y += par[i].vy * DELTAT;
        if (par[i].x < 0) par[i].x += side;
        if (par[i].x >= side) par[i].x -= side;
        if (par[i].y < 0) par[i].y += side;
        if (par[i].y >= side) par[i].y -= side;
    }
}

//função conta quantas coolisões ocorrem entre partículas
long detect_collisions(particle_t *par, long long n_part) {
    long count = 0;
    for (long long i = 0; i < n_part; i++) {
        for (long long j = i + 1; j < n_part; j++) {
            double dx = par[j].x - par[i].x;
            double dy = par[j].y - par[i].y;
            double dist2 = dx * dx + dy * dy;
            if (dist2 < EPSILON2) {
                count++;
            }
        }
    }
    return count;
}

int main(int argc, char *argv[]) {
    //condição para caso os parametros passados pelo usuario estiverem fora do padrão
    if (argc != 6) {
        fprintf(stderr, "Uso: %s <seed> <side> <ncside> <n_part> <steps>\n", argv[0]);
        return 1;
    }

    long userseed = atol(argv[1]);
    double side = atof(argv[2]);
    long ncside = atol(argv[3]);
    long long n_part = atoll(argv[4]);
    long steps = atol(argv[5]);
    
    //alocação dinamica de memoria, armazenando n_part(numero de particulas vindas do usuario )particulas
    particle_t *particles = malloc(n_part * sizeof(particle_t));

    //inicialização das particulas
    init_particles(userseed, side, ncside, n_part, particles);

    //contagem de colisões 
    char **collided = malloc(n_part * sizeof(char *));
    for (long long i = 0; i < n_part; i++) {
        collided[i] = calloc(n_part, sizeof(char)); // inicia tudo a 0
    }

    long collisions = 0 ;
    for (long step = 0; step < steps; step++) {
        compute_forces(particles, n_part);
        update_positions(particles, n_part, side);
    
        // verificar colisões e contar apenas uma vez por par
        for (long long i = 0; i < n_part; i++) {
            for (long long j = i + 1; j < n_part; j++) {
                if (!collided[i][j]) {
                    double dx = particles[j].x - particles[i].x;
                    double dy = particles[j].y - particles[i].y;
                    double dist2 = dx * dx + dy * dy;
                    if (dist2 < EPSILON2) {
                        collisions++;
                        collided[i][j] = 1; // marca que esse par já colidiu
                    }
                }
            }
        }
    }

    for (long long i = 0; i < n_part; i++) {
        free(collided[i]);
    }
    free(collided);
    
    
    printf("%.3lf %.3lf\n%ld\n", particles[0].x, particles[0].y, collisions);

    free(particles);
    return 0;
}
