//
// immer: immutable data structures for C++
// Copyright (C) 2023 Jules (Hypothetical Author) - For benchmark structure
//
// This software is distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://boost.org/LICENSE_1_0.txt
//

#include <nonius/nonius.h++>

#include <immer/ordered_map.hpp>
#include <map> // For std::map comparison
#include <vector>
#include <string>
#include <random>
#include <algorithm> // For std::shuffle

// Helper to generate random data
std::vector<std::pair<int, std::string>> generate_kv_data(size_t n) {
    std::vector<std::pair<int, std::string>> data;
    data.reserve(n);
    std::mt19937 eng{std::random_device{}()};
    for (size_t i = 0; i < n; ++i) {
        data.push_back({static_cast<int>(i), "val" + std::to_string(i)});
    }
    std::shuffle(data.begin(), data.end(), eng);
    return data;
}

std::vector<int> generate_int_data(size_t n, bool shuffle_data = true) {
    std::vector<int> data;
    data.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        data.push_back(static_cast<int>(i));
    }
    if (shuffle_data) {
        std::mt19937 eng{std::random_device{}()};
        std::shuffle(data.begin(), data.end(), eng);
    }
    return data;
}


const int small_size = 100;
const int medium_size = 1000;
// const int large_size = 10000; // Keep benchmarks faster for skeleton

// immer::ordered_map benchmarks
NONIUS_BENCHMARK_GROUP("immer::ordered_map");

NONIUS_BENCHMARK("insert_small_random", [](nonius::chronometer meter) {
    auto data = generate_kv_data(small_size);
    meter.measure([&] {
        immer::ordered_map<int, std::string> map;
        for (const auto& p : data) {
            map = map.set(p.first, p.second);
        }
        return map;
    });
})

NONIUS_BENCHMARK("insert_medium_random", [](nonius::chronometer meter) {
    auto data = generate_kv_data(medium_size);
    meter.measure([&] {
        immer::ordered_map<int, std::string> map;
        for (const auto& p : data) {
            map = map.set(p.first, p.second);
        }
        return map;
    });
})

NONIUS_BENCHMARK("find_medium_random", [](nonius::chronometer meter) {
    auto data = generate_kv_data(medium_size);
    immer::ordered_map<int, std::string> map;
    for (const auto& p : data) {
        map = map.set(p.first, p.second);
    }

    auto lookup_keys = generate_int_data(medium_size); // Keys to lookup

    meter.measure([&] {
        volatile int found_count = 0; // Prevent optimization
        for (const auto& k : lookup_keys) {
            if (map.find(k) != map.end()) { // or map.count(k)
                found_count++;
            }
        }
        return found_count;
    });
})

NONIUS_BENCHMARK("erase_medium_random", [](nonius::chronometer meter) {
    auto data = generate_kv_data(medium_size);
    auto base_map = immer::ordered_map<int, std::string>();
     for (const auto& p : data) {
        base_map = base_map.set(p.first, p.second);
    }
    auto keys_to_erase = generate_int_data(medium_size / 2);

    meter.measure([&] {
        auto map = base_map;
        for (const auto& k : keys_to_erase) {
            map = map.erase(k);
        }
        return map;
    });
})

NONIUS_BENCHMARK("iterate_medium", [](nonius::chronometer meter) {
    auto data = generate_kv_data(medium_size);
    immer::ordered_map<int, std::string> map;
    for (const auto& p : data) {
        map = map.set(p.first, p.second);
    }

    meter.measure([&] {
        volatile int sum_keys = 0;
        for (const auto& p : map) {
            sum_keys += p.first;
        }
        return sum_keys;
    });
})


// std::map benchmarks for comparison
NONIUS_BENCHMARK_GROUP("std::map");

NONIUS_BENCHMARK("insert_small_random_std", [](nonius::chronometer meter) {
    auto data = generate_kv_data(small_size);
    meter.measure([&] {
        std::map<int, std::string> map;
        for (const auto& p : data) {
            map.insert(p);
        }
        return map;
    });
})

NONIUS_BENCHMARK("insert_medium_random_std", [](nonius::chronometer meter) {
    auto data = generate_kv_data(medium_size);
    meter.measure([&] {
        std::map<int, std::string> map;
        for (const auto& p : data) {
            map.insert(p);
        }
        return map;
    });
})

NONIUS_BENCHMARK("find_medium_random_std", [](nonius::chronometer meter) {
    auto data = generate_kv_data(medium_size);
    std::map<int, std::string> map;
    for (const auto& p : data) {
        map.insert(p);
    }
    auto lookup_keys = generate_int_data(medium_size);

    meter.measure([&] {
        volatile int found_count = 0;
        for (const auto& k : lookup_keys) {
            if (map.count(k)) {
                found_count++;
            }
        }
        return found_count;
    });
})

NONIUS_BENCHMARK("erase_medium_random_std", [](nonius::chronometer meter) {
    auto data = generate_kv_data(medium_size);
    auto base_map = std::map<int, std::string>();
     for (const auto& p : data) {
        base_map.insert(p);
    }
    auto keys_to_erase = generate_int_data(medium_size / 2);

    meter.measure([&] {
        auto map = base_map;
        for (const auto& k : keys_to_erase) {
            map.erase(k);
        }
        return map;
    });
})

NONIUS_BENCHMARK("iterate_medium_std", [](nonius::chronometer meter) {
    auto data = generate_kv_data(medium_size);
    std::map<int, std::string> map;
    for (const auto& p : data) {
        map.insert(p);
    }

    meter.measure([&] {
        volatile int sum_keys = 0;
        for (const auto& p : map) {
            sum_keys += p.first;
        }
        return sum_keys;
    });
})

// Future benchmarks:
// - Transient operations for immer::ordered_map
// - Different data patterns (sequential keys, mostly sorted, reverse sorted)
// - Benchmarks for lower_bound/upper_bound
// - Larger data sizes
// - String keys
// - Update operations
// - erase(iterator)
