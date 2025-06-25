//
// immer: immutable data structures for C++
// Copyright (C) 2023 Jules (Hypothetical Author) - For benchmark structure
//
// This software is distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://boost.org/LICENSE_1_0.txt
//

#include <nonius/nonius.h++>

#include <immer/ordered_set.hpp>
#include <set> // For std::set comparison
#include <vector>
#include <string>
#include <random>
#include <algorithm> // For std::shuffle

// Helper to generate random int data (can be shared or defined in a common header)
std::vector<int> generate_int_data_set(size_t n, bool shuffle_data = true) {
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

const int small_size_set = 100;
const int medium_size_set = 1000;

// immer::ordered_set benchmarks
NONIUS_BENCHMARK_GROUP("immer::ordered_set");

NONIUS_BENCHMARK("insert_small_random", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(small_size_set);
    meter.measure([&] {
        immer::ordered_set<int> set;
        for (const auto& val : data) {
            set = set.insert(val);
        }
        return set;
    });
})

NONIUS_BENCHMARK("insert_medium_random", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(medium_size_set);
    meter.measure([&] {
        immer::ordered_set<int> set;
        for (const auto& val : data) {
            set = set.insert(val);
        }
        return set;
    });
})

NONIUS_BENCHMARK("find_medium_random", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(medium_size_set);
    immer::ordered_set<int> set;
    for (const auto& val : data) {
        set = set.insert(val);
    }

    auto lookup_keys = generate_int_data_set(medium_size_set); // Keys to lookup

    meter.measure([&] {
        volatile int found_count = 0; // Prevent optimization
        for (const auto& k : lookup_keys) {
            if (set.find(k) != set.end()) { // or set.count(k)
                found_count++;
            }
        }
        return found_count;
    });
})

NONIUS_BENCHMARK("erase_medium_random", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(medium_size_set);
    auto base_set = immer::ordered_set<int>();
     for (const auto& val : data) {
        base_set = base_set.insert(val);
    }
    auto keys_to_erase = generate_int_data_set(medium_size_set / 2);

    meter.measure([&] {
        auto set = base_set;
        for (const auto& k : keys_to_erase) {
            set = set.erase(k);
        }
        return set;
    });
})

NONIUS_BENCHMARK("iterate_medium", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(medium_size_set);
    immer::ordered_set<int> set;
    for (const auto& val : data) {
        set = set.insert(val);
    }

    meter.measure([&] {
        volatile int sum_keys = 0;
        for (const auto& val : set) {
            sum_keys += val;
        }
        return sum_keys;
    });
})


// std::set benchmarks for comparison
NONIUS_BENCHMARK_GROUP("std::set");

NONIUS_BENCHMARK("insert_small_random_std", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(small_size_set);
    meter.measure([&] {
        std::set<int> set;
        for (const auto& val : data) {
            set.insert(val);
        }
        return set;
    });
})

NONIUS_BENCHMARK("insert_medium_random_std", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(medium_size_set);
    meter.measure([&] {
        std::set<int> set;
        for (const auto& val : data) {
            set.insert(val);
        }
        return set;
    });
})

NONIUS_BENCHMARK("find_medium_random_std", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(medium_size_set);
    std::set<int> set;
    for (const auto& val : data) {
        set.insert(val);
    }
    auto lookup_keys = generate_int_data_set(medium_size_set);

    meter.measure([&] {
        volatile int found_count = 0;
        for (const auto& k : lookup_keys) {
            if (set.count(k)) {
                found_count++;
            }
        }
        return found_count;
    });
})

NONIUS_BENCHMARK("erase_medium_random_std", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(medium_size_set);
    auto base_set = std::set<int>();
     for (const auto& val : data) {
        base_set.insert(val);
    }
    auto keys_to_erase = generate_int_data_set(medium_size_set / 2);

    meter.measure([&] {
        auto set = base_set;
        for (const auto& k : keys_to_erase) {
            set.erase(k);
        }
        return set;
    });
})

NONIUS_BENCHMARK("iterate_medium_std", [](nonius::chronometer meter) {
    auto data = generate_int_data_set(medium_size_set);
    std::set<int> set;
    for (const auto& val : data) {
        set.insert(val);
    }

    meter.measure([&] {
        volatile int sum_keys = 0;
        for (const auto& val : set) {
            sum_keys += val;
        }
        return sum_keys;
    });
})

// Future benchmarks:
// - Transient operations for immer::ordered_set
// - Different data patterns
// - String values
// - erase(iterator)
// - lower_bound/upper_bound
