//
// Created by jaroslavp on 5/13/2022.
//

#include "base.h"

[[nodiscard]] constexpr std::vector<long long> make_powers(int n, long long base, long long modulo) {
    std::vector<long long> powers;
    powers.reserve(n + 1);
    powers.emplace_back(1);
    for (int i = 0; i < n; i++)
        powers.emplace_back(base * powers.back() % modulo);
    return powers;
}

using namespace std;

int main() {
    TIMER(main_timer);
    for (int i = 0; i < 10; i++)
        cout << rng::get<double>() << endl;
}