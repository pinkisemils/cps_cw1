#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <random>
#include <chrono>
#include <array>
#include <cstring>
#include <thread>
#include <unistd.h>


using namespace std;
using namespace std::chrono;

constexpr char *secret = "A long time ago in a galaxy far, far away....";

constexpr unsigned int GENE_LENGTH = 8;
constexpr unsigned int NUM_COPIES_ELITE = 4;
constexpr unsigned int NUM_ELITE = 8;
constexpr double CROSSOVER_RATE = 0.9;
constexpr unsigned int POP_SIZE = 512;
constexpr unsigned int NUM_CHARS = strlen(secret);
constexpr unsigned int CHROMO_LENGTH = NUM_CHARS * GENE_LENGTH;
constexpr double MUTATION_RATE = 0.001;

unsigned int CORES;


struct genome
{
    vector<unsigned int> bits;
    unsigned int fitness = 0.0;
    unsigned int gene_length = GENE_LENGTH;
};

//struct worker_thread
//{
//    genome* genomes;
//};

genome best;

unsigned int calculate_total_fitness(const vector<genome> &genomes)
{
    unsigned int total_fitness = 0;

    for (auto &genome : genomes)
        total_fitness += genome.gene_length;
    return total_fitness;
}

inline bool comp_by_fitness(const genome &a, const genome &b)
{
    return a.fitness < b.fitness;
}

void grab_N_best(const unsigned int N, const unsigned int copies, vector<genome> &old_pop, vector<genome> &new_pop)
{
    sort(old_pop.begin(), old_pop.end(), comp_by_fitness);
    best = old_pop[0];
    for (unsigned int n = 0; n < N; ++n)
        for (unsigned int i = 0; i < copies; ++i)
            new_pop[n * copies + i] = old_pop[n];
}

const genome& roulette_wheel_selection(unsigned int pop_size, const unsigned int fitness, const genome* genomes)
{
    static default_random_engine e(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    static uniform_real_distribution<double> dist;
    double slice = dist(e) * fitness;
    unsigned int total = 0;

    for (unsigned int i = 0; i < pop_size; ++i)
    {
        total += genomes[i].fitness;
        if (total > slice)
            return genomes[i];
    }
    return genomes[0];
}

void cross_over(double crossover_rate, unsigned int chromo_length, const genome &mum, const genome &dad, genome *baby1, genome *baby2)
{
    static default_random_engine e(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    static uniform_real_distribution<double> float_dist;
    static uniform_int_distribution<unsigned int> int_dist(0, chromo_length);

    if (float_dist(e) > crossover_rate || mum.bits == dad.bits)
    {
        baby1->bits = mum.bits;
        baby2->bits = mum.bits;
    }
    else
    {
        const unsigned int cp = int_dist(e);

        baby1->bits.insert(baby1->bits.end(), mum.bits.begin(), mum.bits.begin() + cp);
        baby1->bits.insert(baby1->bits.end(), dad.bits.begin() + cp, dad.bits.end());
        baby2->bits.insert(baby2->bits.end(), dad.bits.begin(), dad.bits.begin() + cp);
        baby2->bits.insert(baby2->bits.end(), mum.bits.begin() + cp, mum.bits.end());
    }
}

void mutate(double mutation_rate, genome *gen)
{
    static default_random_engine e(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    static uniform_real_distribution<double> dist;
    double rnd;

    for (auto &bit : gen->bits)
    {
        rnd = dist(e);
        if (rnd < mutation_rate)
            bit = !bit;
    }
}

void epoch_st(unsigned int pop_size,
             int fitness,
             genome* genomes,
             genome* babies,
             int step_size)
{
    for (int itr = 0; itr < step_size; itr+=2) {
        auto mum = roulette_wheel_selection(pop_size, fitness, genomes);
        auto dad = roulette_wheel_selection(pop_size, fitness, genomes);
        genome baby1;
        genome baby2;

        cross_over(CROSSOVER_RATE, CHROMO_LENGTH, mum, dad, &babies[itr], &babies[itr+1]);
        mutate(MUTATION_RATE,  &babies[itr]);
        mutate(MUTATION_RATE,  &babies[itr+1]);
    }
    return;
}

vector<genome> epoch(unsigned int pop_size, vector<genome> &genomes)
{
    auto fitness = calculate_total_fitness(genomes);
    vector<genome> babies(pop_size);
    babies.reserve(pop_size);
    vector<thread> threads(CORES-1);

    if (((NUM_COPIES_ELITE * NUM_ELITE) % 2) == 0)
        grab_N_best(NUM_ELITE, NUM_COPIES_ELITE, genomes, babies);
    int offset = NUM_ELITE * NUM_COPIES_ELITE;
    int step = (pop_size - offset) / CORES;
    for (int i=0; i< CORES-1; ++i)
    {
        threads[i] = std::thread(epoch_st, pop_size, fitness, &genomes[0], &babies[offset + i*step], step);
    }
    epoch_st(pop_size, fitness, &genomes[0], &babies[offset + ((CORES-1)*step)], step + step%CORES);

    for (auto &t : threads)
    {
        t.join();
    }
    return babies;
}

vector<unsigned int> decode(genome &gen)
{
    static vector<unsigned int> this_gene(gen.gene_length);
    vector<unsigned int> decoded(NUM_CHARS);

    for (unsigned int gene = 0, count = 0; gene < gen.bits.size(); gene += gen.gene_length, ++count)
    {
        for (unsigned int bit = 0; bit < gen.gene_length; ++bit)
            this_gene[bit] = gen.bits[gene + bit];

        unsigned int val = 0;
        unsigned int multiplier = 1;
        for (unsigned int c_bit = this_gene.size(); c_bit > 0; --c_bit)
        {
            //val += this_gene[c_bit - 1] * multiplier;
            val += gen.bits[gene + c_bit - 1] * multiplier;
            multiplier *= 2;
        }
        decoded[count] = val;
    }
    return decoded;
}

void decode_st(genome *genes, vector<unsigned int> *output, int chunk_size )
{


    for (int out_i = 0; out_i < chunk_size; ++out_i)
    {
        genome gen = genes[out_i];
        for (unsigned int gene = 0, count = 0; gene < gen.bits.size(); gene += gen.gene_length, ++count)
        {
            unsigned int val = 0;
            unsigned int multiplier = 1<< GENE_LENGTH;

            for (unsigned int c_bit = 0; c_bit < GENE_LENGTH; ++c_bit)
            {
                multiplier /= 2;
                val += gen.bits[gene + c_bit] * multiplier;
            }
            output[out_i][count] = val;
        }
    }
}

vector<vector<unsigned int>> update_epoch(unsigned int pop_size, vector<genome> &genomes)
{
    vector<vector<unsigned int>> guesses;
    vector<thread> threads(CORES-1);
    guesses.reserve(genomes.size());

    genomes = epoch(pop_size, genomes);

    // Allocate memory for the guesses vector
    // Imagine a world where a vector can be constructed out of a 2d array
    //  WITHOUT A MEMCPY()

    for (int i = 0; i < genomes.size(); ++i)
    {
        guesses.push_back(std::vector<unsigned int>(NUM_CHARS));
        guesses[i].reserve(NUM_CHARS);
    }
    int chunk_size = POP_SIZE/ CORES;
    for (int i = 0; i< CORES-1; ++i )
    {
        threads[i] = std::thread(decode_st,&genomes[chunk_size * i], &guesses[chunk_size * i], chunk_size);
    }
    // have to account for weird population sizes/core counts
    decode_st(&genomes[chunk_size * (CORES-1)], &guesses[chunk_size * (CORES-1)], chunk_size + POP_SIZE%CORES);

    for (auto &t : threads)
    {
        t.join();
    }

    return guesses;
}

unsigned int check_guess(const vector<unsigned int> &guess)
{
    vector<unsigned char> v(guess.size());
    for (unsigned int i = 0; i < guess.size(); ++i)
        v[i] = static_cast<unsigned char>(guess[i]);
    string s(v.begin(), v.end());
    unsigned int diff = 0;

    for (unsigned int i = 0; i < s.length(); ++i)
    {
        diff += abs(s[i] - secret[i]);
    }
    return diff;
}

string get_guess(const vector<unsigned int> &guess)
{
    vector<unsigned char> v(guess.size());
    for (unsigned int i = 0; i < guess.size(); ++i)
        v[i] = static_cast<unsigned char>(guess[i]);
    string s(v.begin(), v.end());
    return s;
}

int main()
{
    CORES = std::thread::hardware_concurrency();
    default_random_engine e(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    uniform_int_distribution<unsigned int> int_dist(0, 1);
    vector<genome> genomes(POP_SIZE);

    for (unsigned int i = 0; i < POP_SIZE; ++i)
    {
        for (unsigned int j = 0; j < CHROMO_LENGTH; ++j)
        {
            genomes[i].bits.push_back(int_dist(e));
        }
    }
    auto population = update_epoch(POP_SIZE, genomes);
    for (unsigned int generation = 0; generation < 2048; ++generation)
    {
        for (unsigned int i = 0; i < POP_SIZE; ++i)
        {
            genomes[i].fitness = check_guess(population[i]);
        }
        population = update_epoch(POP_SIZE, genomes);
    }
    cout << "Best generation: " << get_guess(decode(best)) << endl;
    cout << "Diff: " << check_guess(decode(best)) << endl;
    cout<< "Number of threads:" << CORES << endl;
    return 0;
}
