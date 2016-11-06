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
    decode_st(&genomes[chunk_size * (CORES-1)], &guesses[chunk_size * (CORES-1)], chunk_size + POP_SIZE%CORES);

    for (auto &t : threads)
    {
        t.join();
    }

    return guesses;
}


