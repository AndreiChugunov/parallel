#include <iostream>
#include <array>
#include <random>
#include <algorithm>
#include <functional>
#include <cassert>
#include <iterator>
#include <chrono>

#include <pthread.h>


namespace mt_lab {
    constexpr int NUBMER_OF_STARTS = 1;
    constexpr int MAX_THREADS = 4;
    constexpr long AMOUNT_OF_DATA = 16;
    using int_t = int;
    constexpr int_t LOWER = 0;
    constexpr int_t UPPER = 100;
    static_assert(UPPER >= LOWER, "UPPER must be greater than LOWER");
    static_assert((MAX_THREADS & (MAX_THREADS - 1)) == 0, "MAX_THREADS should be power of 2");
    static_assert((AMOUNT_OF_DATA & (AMOUNT_OF_DATA - 1)) == 0, "AMOUNT_OF_DATA should be power of 2");
    static_assert((AMOUNT_OF_DATA > MAX_THREADS * 2), "AMOUNT_OF_DATA should be more than 2xMAX_THREADS");

}

namespace mt_lab {

    void print_data(int_t* first, int_t* last) {
        std::copy(first, last, std::ostream_iterator<mt_lab::int_t>(std::cout, ", "));
        std::cout << "\n";
    }

    void compute_comparators(int_t * first, int size, int is_even) {
        for (auto j = is_even; j < size - 1; j += 2) {
            if (first[j] > first[j + 1]) {
                std::swap(first[j], first[j + 1]);
            }
        }
    }

    void odd_even_seq_sort(int_t * first, int_t * last) {
        const auto size = std::distance(first, last);
        for (auto i = 0; i < size; ++i) {
            compute_comparators(first, size, i % 2);
        }
    }

    struct thread_data {
        int thread_id;
        int iteration;
        long int array_size;
        int_t * array;
    };
    
    void * thread_job(void * th_data) {
        const thread_data data = *static_cast<thread_data *>(th_data);
        
        const auto th_id = data.thread_id;
        const auto i = data.iteration;
        const auto size = (data.array_size / MAX_THREADS) - ((th_id == MAX_THREADS - 1) ? (i % 2) : 0);
        const auto first = data.array + data.array_size / MAX_THREADS * th_id + (i % 2);
        compute_comparators(first, size, 0);
        return 0;
    }

    void odd_even_par_sort(int_t * first, int_t * last) {
        std::array<pthread_t, MAX_THREADS> threads = {};
        std::array<thread_data, MAX_THREADS> threads_data = {};
        std::array<int, MAX_THREADS> codes = {1};

        const auto size = std::distance(first, last);
        for (auto i = 0; i < size; ++i) {
            auto th_id = 0;
            std::generate(std::begin(threads_data), std::end(threads_data), [&](){
                return thread_data {th_id++, i, size, first};
            });
            std::transform(std::begin(threads), std::end(threads), std::begin(threads_data), std::begin(codes),
                        [=](auto & th, auto & data) {
                            return pthread_create(&th, nullptr, thread_job, &data);
                        });
            auto is_ok = [](int rhs){ return !rhs; };
            assert(std::all_of(std::begin(codes), std::end(codes), is_ok));
            std::transform(std::begin(threads), std::end(threads), std::begin(codes),
                        [](auto & th) {
                            return pthread_join(th, nullptr);
                        });
            assert(std::all_of(std::begin(codes), std::end(codes), is_ok));
            //mt_lab::print_data(first, last);
        }
        
    }


    void sort_n_times(double * first, int n, void (* sort_fun) (int_t *, int_t *)) {
        for (int i = 0; i < n; i++) {

            std::array<mt_lab::int_t, mt_lab::AMOUNT_OF_DATA> data = {0};
            {
                auto dice = std::bind(std::uniform_int_distribution<mt_lab::int_t>(mt_lab::LOWER, mt_lab::UPPER), std::default_random_engine{});
                std::generate(std::begin(data), std::end(data), dice);
            }

            auto start = std::chrono::steady_clock::now();
            sort_fun(std::begin(data), std::end(data));
            auto end = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration<double, std::milli>(end - start).count();
            first[i] = diff;
            assert(std::is_sorted(std::begin(data), std::end(data)));
        }
    }

}


namespace mt_util {
    double mean_value (double * first, int size) {
        double sum = 0;
        for (int i = 0; i < size; ++i) {
            sum += first[i];
        }
        return sum / size;
    }

    double dispersion(double * first, int size, double mean_value) {
        double sum = 0;
        for (int i = 0; i < size; ++i) {
            sum += (first[i] * first[i] - mean_value * mean_value);
        }
        return sum / size;
    }

    void print_times(double* first, double* last) {
        std::copy(first, last, std::ostream_iterator<double>(std::cout, ", "));
        std::cout << "\n";
    }

}

int main() {

    std::array<double, mt_lab::NUBMER_OF_STARTS> times_par = {0};
    mt_lab::sort_n_times(std::begin(times_par), mt_lab::NUBMER_OF_STARTS, mt_lab::odd_even_par_sort);
    std::cout << "Parralel times: ";
    mt_util::print_times(std::begin(times_par), std::end(times_par));
    std::cout << "\n";
    double mean_par = mt_util::mean_value(std::begin(times_par), mt_lab::NUBMER_OF_STARTS);
    double dis_par = mt_util::dispersion(std::begin(times_par), mt_lab::NUBMER_OF_STARTS, mean_par);

    std::cout << "mean_par: " << mean_par << std::endl;
    std::cout << "dis_par: " << dis_par << "\n" << std::endl;


    // std::array<double, mt_lab::NUBMER_OF_STARTS> times_seq = {0};
    // mt_lab::sort_n_times(std::begin(times_seq), mt_lab::NUBMER_OF_STARTS, mt_lab::odd_even_seq_sort);
    // std::cout << "Seq times: ";
    // mt_util::print_times(std::begin(times_seq), std::end(times_seq));
    // std::cout << "\n";
    // double mean_seq = mt_util::mean_value(std::begin(times_seq), mt_lab::NUBMER_OF_STARTS);
    // double dis_seq = mt_util::dispersion(std::begin(times_seq), mt_lab::NUBMER_OF_STARTS, mean_seq);

    // std::cout << "mean_seq: " << mean_seq << std::endl;
    // std::cout << "dis_seq: " << dis_seq << "\n" << std::endl;



}