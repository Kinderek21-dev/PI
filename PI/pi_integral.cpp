/**
 * 
 * @brief Program obliczaj¹cy przybli¿enie liczby Pi metod¹ ca³kowania numerycznego.
 *
 * Program wykorzystuje metodê prostok¹tów (punktu œrodkowego) do obliczenia ca³ki oznaczonej
 * funkcji \f$ f(x) = \frac{4}{1 + x^2} \f$ na przedziale [0, 1].
 * Wartoœæ tej ca³ki wynosi dok³adnie \f$ \pi \f$.
 * Obliczenia s¹ zrównoleglone przy u¿yciu standardowych w¹tków C++ (std::thread).
 *
 * 
 */

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <string>
#include <sstream>
#include <algorithm>

 /**
  * @brief G³ówna funkcja wejœciowa programu.
  *
  * Pobiera parametry konfiguracyjne z linii komend, dzieli zakres ca³kowania pomiêdzy w¹tki,
  * agreguje wyniki cz¹stkowe i wypisuje finalne przybli¿enie liczby Pi, b³¹d bezwzglêdny oraz czas wykonania.
  *
  * Logika podzia³u pracy zapewnia równomierne obci¹¿enie w¹tków nawet w przypadku, gdy liczba podzia³ów 'n'
  * nie dzieli siê bez reszty przez liczbê w¹tków.
  *
  * @param argc Liczba argumentów przekazanych do programu.
  * @param argv Tablica ci¹gów znaków zawieraj¹ca argumenty:
  * - argv[0]: Nazwa programu.
  * - argv[1]: (Opcjonalny) Liczba podzia³ów (kroków ca³ki) 'n'. Domyœlnie 100 000 000.
  * - argv[2]: (Opcjonalny) Liczba w¹tków do utworzenia. Domyœlnie wykrywana liczba rdzeni logicznych.
  *

  */
int main(int argc, char* argv[]) {

    /// Domyœlna liczba podzia³ów (kroków ca³kowania).
    uint64_t n = 100'000'000ULL;

    /// Liczba w¹tków sprzêtowych (rdzeni logicznych).
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 1; // Zabezpieczenie na wypadek braku wykrycia

    // Parsowanie pierwszego argumentu (liczba n)
    if (argc >= 2) {
        std::istringstream ss(argv[1]);
        if (!(ss >> n) || n == 0) {
            std::cerr << "Niepoprawna wartosc n: " << argv[1] << "\n";
            return 1;
        }
    }

    // Parsowanie drugiego argumentu (liczba w¹tków)
    if (argc >= 3) {
        std::istringstream ss(argv[2]);
        if (!(ss >> num_threads) || num_threads == 0) {
            std::cerr << "Niepoprawna liczba watkow: " << argv[2] << "\n";
            return 1;
        }
    }

    // Ograniczenie liczby w¹tków, by nie przekracza³a liczby zadañ
    if (num_threads > n) num_threads = static_cast<unsigned int>(std::min<uint64_t>(num_threads, n));

    std::cout << "Liczba podzialow (n) = " << n << "\n";
    std::cout << "Liczba watkow = " << num_threads << "\n";

    /// Szerokoœæ pojedynczego prostok¹ta (krok ca³kowania).
    double dx = 1.0 / static_cast<double>(n);

    /// Wektor przechowuj¹cy sumy cz¹stkowe obliczone przez poszczególne w¹tki.
    std::vector<double> partial_sums(num_threads, 0.0);

    /// Kontener na obiekty w¹tków.
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Obliczanie podzia³u pracy
    uint64_t base_chunk = n / num_threads;
    uint64_t remainder = n % num_threads;

    // Tworzenie i uruchamianie w¹tków
    for (unsigned int t = 0; t < num_threads; ++t) {
        // Wyliczanie zakresu indeksów [start_idx, end_idx) dla danego w¹tku 't'
        uint64_t start_idx = t * base_chunk + std::min<uint64_t>(t, remainder);
        uint64_t chunk_size = base_chunk + (t < remainder ? 1 : 0);
        uint64_t end_idx = start_idx + chunk_size;

        /**
         * Lambda wykonuj¹ca obliczenia w w¹tku.
         * Iteruje po przydzielonym zakresie i sumuje pola prostok¹tów.
         */
        threads.emplace_back([t, start_idx, end_idx, dx, &partial_sums]() {
            double local_sum = 0.0;
            for (uint64_t i = start_idx; i < end_idx; ++i) {
                // Punkt œrodkowy prostok¹ta: (i + 0.5) * dx
                double x = (static_cast<double>(i) + 0.5) * dx;
                // Wartoœæ funkcji 4/(1+x^2)
                local_sum += 4.0 / (1.0 + x * x);
            }
            // Zapisanie wyniku do wspó³dzielonego wektora (bez koniecznoœci u¿ycia mutexa,
            // poniewa¿ ka¿dy w¹tek pisze do unikalnego indeksu 't')
            partial_sums[t] = local_sum;
            });
    }

    // Oczekiwanie na zakoñczenie wszystkich w¹tków
    for (auto& th : threads) if (th.joinable()) th.join();

    // Sumowanie wyników cz¹stkowych
    double total_sum = 0.0;
    for (auto v : partial_sums) total_sum += v;

    /// Ostateczne przybli¿enie wartoœci Pi.
    double pi_approx = total_sum * dx;

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    // Wyœwietlanie wyników
    std::cout << std::fixed << std::setprecision(12);
    std::cout << "Przyblizenie PI = " << pi_approx << "\n";
    std::cout << "Blad bezwzgledny = " << std::abs(pi_approx - 3.14159265358979323846) << "\n";
    std::cout << "Czas obliczen = " << elapsed.count() << " s\n";

    return 0;
}