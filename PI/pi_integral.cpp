#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <string>
#include <sstream>
#include <algorithm>

int main(int argc, char* argv[]) {

    uint64_t n = 100'000'000ULL;
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 1;

    if (argc >= 2) {
        std::istringstream ss(argv[1]);
        if (!(ss >> n) || n == 0) {
            std::cerr << "Niepoprawna wartosc n: " << argv[1] << "\n";
            return 1;
        }
    }
    if (argc >= 3) {
        std::istringstream ss(argv[2]);
        if (!(ss >> num_threads) || num_threads == 0) {
            std::cerr << "Niepoprawna liczba watkow: " << argv[2] << "\n";
            return 1;
        }
    }

    if (num_threads > n) num_threads = static_cast<unsigned int>(std::min<uint64_t>(num_threads, n));

    std::cout << "Liczba podzialow (n) = " << n << "\n";
    std::cout << "Liczba watkow = " << num_threads << "\n";

    double dx = 1.0 / static_cast<double>(n);
    std::vector<double> partial_sums(num_threads, 0.0);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    auto start_time = std::chrono::high_resolution_clock::now();

    uint64_t base_chunk = n / num_threads;
    uint64_t remainder = n % num_threads;

    for (unsigned int t = 0; t < num_threads; ++t) {
        uint64_t start_idx = t * base_chunk + std::min<uint64_t>(t, remainder);
        uint64_t chunk_size = base_chunk + (t < remainder ? 1 : 0);
        uint64_t end_idx = start_idx + chunk_size;

        threads.emplace_back([t, start_idx, end_idx, dx, &partial_sums]() {
            double local_sum = 0.0;
            for (uint64_t i = start_idx; i < end_idx; ++i) {
                double x = (static_cast<double>(i) + 0.5) * dx;
                local_sum += 4.0 / (1.0 + x * x);
            }
            partial_sums[t] = local_sum;
            });
    }

    for (auto& th : threads) if (th.joinable()) th.join();

    double total_sum = 0.0;
    for (auto v : partial_sums) total_sum += v;
    double pi_approx = total_sum * dx;

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << std::fixed << std::setprecision(12);
    std::cout << "Przyblizenie PI = " << pi_approx << "\n";
    std::cout << "Blad bezwzgledny = " << std::abs(pi_approx - 3.14159265358979323846) << "\n";
    std::cout << "Czas obliczen = " << elapsed.count() << " s\n";

    return 0;
}