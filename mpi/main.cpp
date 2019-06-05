#define OMPI_SKIP_MPICXX
#include <iostream>
#include <array>
#include <random>
#include <algorithm>
#include <functional>
#include <iterator>
#include <cassert>
#include <mpi.h>
#include <chrono>
#include <fstream>


namespace mpi_lab {
    using int_t = int;
    constexpr int_t LOWER = 0;
    constexpr int_t UPPER = 100;
}

namespace mpi_lab {
     void print_data(int_t* first, int_t* last) {
        std::copy(first, last, std::ostream_iterator<mpi_lab::int_t>(std::cout, ", "));
        std::cout << "\n";
    }


    void compute_comparators(int_t * first, int size, int is_even) {
        for (auto j = is_even; j < size - 1; j += 2) {
            if (first[j] > first[j + 1]) {
                std::swap(first[j], first[j + 1]);
            }
        }
    }


}

int main (int argc, char* argv[]) {

    MPI_Init(&argc, &argv);
    auto initialized = 0;
    auto rank = 0, size = 0;
    
    auto amount = atoi(argv[1]);

    MPI_Initialized(std::addressof(initialized));
    if(initialized) {
        MPI_Comm_rank(MPI_COMM_WORLD, std::addressof(rank));
        MPI_Comm_size(MPI_COMM_WORLD, std::addressof(size));
    }

    assert((size & (size - 1)) == 0);
    assert((amount & (amount - 1)) == 0);
    assert((amount > size * 2));

    
    auto amount_per_proc = amount / size;

    std::vector<mpi_lab::int_t> data (amount_per_proc);
    {
        auto dice = std::bind(std::uniform_int_distribution<mpi_lab::int_t>(mpi_lab::LOWER, mpi_lab::UPPER), std::default_random_engine{});
        std::generate(std::begin(data), std::begin(data) + amount_per_proc, dice);
    }
    std::vector<mpi_lab::int_t> main_data(rank ? 0 : amount);
    MPI_Gather(data.data(), amount_per_proc, MPI_INT, main_data.data(), amount_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        //mpi_lab::print_data(main_data.data(), main_data.data() + main_data.size());
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < amount; ++i) {
            for (int r = 1; r < size; ++r) {
                const auto size_per_proc = (main_data.size() / size) - ((r == size - 1) ? (i % 2) : 0);
                const auto first = main_data.data() + main_data.size() / size * r + (i % 2);
                MPI_Send(first, size_per_proc, MPI_INT, r, 0, MPI_COMM_WORLD);
            }
            const auto my_size = main_data.size() / size;
            const auto my_first = main_data.data() + (i % 2);
            mpi_lab::compute_comparators(my_first, my_size, 0);
            for (int r = 1; r < size; ++r) {
                MPI_Status status;
                auto current_size = 0;
                MPI_Probe(r, 0, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &current_size);
                const auto first = main_data.data() + main_data.size() / size * r + (i % 2);
                MPI_Recv(first, current_size, MPI_INT, r, 0, MPI_COMM_WORLD, &status);
            }
        }
        auto end = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << "Time taken: " << diff << std::endl;
        std::ofstream myfile;
        std::string s_size = std::to_string(size);
        myfile.open ("size_" + s_size + ".txt", std::ios::app);
        myfile << diff << "\n";
        myfile.close();
    } else {
        for (int i = 0; i < amount; ++i) {
            MPI_Status status;
            auto current_size = 0;
            MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &current_size);

            std::vector<mpi_lab::int_t> current_data(current_size);
        
            MPI_Recv(current_data.data(), current_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            mpi_lab::compute_comparators(current_data.data(), current_size, 0);
            MPI_Send(current_data.data(), current_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) {
        //mpi_lab::print_data(main_data.data(), main_data.data() + main_data.size());
        assert(std::is_sorted(main_data.data(), main_data.data() + main_data.size()));
    }

    MPI_Finalize();
    return 0;
}
