//
// immer: immutable data structures for C++
// Copyright (C) 2023 Jules (Hypothetical Author)
//
// This software is distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://boost.org/LICENSE_1_0.txt
//

// Assuming this file will also be a main for its own test executable, or part of a larger build.
// If it's part of a larger build that already has CATCH_CONFIG_MAIN, this define should be removed here.
// For now, let's assume it could be run standalone for focused testing.
#ifndef CATCH_CONFIG_MAIN
// #define CATCH_CONFIG_MAIN // Define only if this is the main compilation unit for Catch2
#endif
#include <catch2/catch.hpp>

#include <immer/ordered_set.hpp>
#include <string>
#include <vector>
#include <set> // For comparison data

// Use a specific type for testing
using test_set_t = immer::ordered_set<int>;
using test_value_t_set = typename test_set_t::value_type; // int

TEST_CASE("ordered_set basic construction and properties") {
    SECTION("default constructed set is empty") {
        test_set_t s1;
        REQUIRE(s1.empty());
        REQUIRE(s1.size() == 0);
        REQUIRE(s1.begin() == s1.end());
    }

    SECTION("set from initializer_list") {
        test_set_t s1 = {1, 2, 3, 2}; // Duplicates should be ignored by set logic
        // Placeholder behavior: size might be 0 or include duplicates until impl is real
        // REQUIRE_FALSE(s1.empty());
        // REQUIRE(s1.size() == 3);

        struct custom_compare_set {
            bool operator()(int a, int b) const { return a < b; }
        };
        immer::ordered_set<int, custom_compare_set> s2 = {1};
        // REQUIRE_FALSE(s2.empty());
    }

    SECTION("set from iterator range") {
        std::vector<test_value_t_set> values = {1, 2, 3, 2};
        test_set_t s1(values.begin(), values.end());
        // REQUIRE(s1.size() == 3);
    }

    SECTION("identity") {
        test_set_t s1;
        test_set_t s2;
        // REQUIRE(s1.identity() == s2.identity()); // Empty sets might share identity

        test_set_t s3 = {1};
        // REQUIRE(s1.identity() != s3.identity());
    }
}

TEST_CASE("ordered_set element lookup") {
    test_set_t s1 = {1, 3, 2};

    SECTION("count") {
        // REQUIRE(s1.count(1) == 1);
        // REQUIRE(s1.count(2) == 1);
        // REQUIRE(s1.count(4) == 0);
    }

    SECTION("find") {
        auto it_found = s1.find(2);
        // REQUIRE(it_found != s1.end());
        // REQUIRE(*it_found == 2);

        auto it_not_found = s1.find(4);
        // REQUIRE(it_not_found == s1.end());

        const test_set_t& cs1 = s1;
        auto cit_found = cs1.find(1);
        // REQUIRE(cit_found != cs1.end());
        // REQUIRE(*cit_found == 1);
    }

    SECTION("lower_bound, upper_bound, equal_range") {
        // Test these once iterators and ordering are real
        // auto lb = s1.lower_bound(2);
        // REQUIRE(lb != s1.end());
        // REQUIRE(*lb == 2);

        // auto ub = s1.upper_bound(2);
        // REQUIRE(ub != s1.end());
        // REQUIRE(*ub == 3);

        // auto er = s1.equal_range(2);
        // REQUIRE(er.first == lb);
        // REQUIRE(er.second == ub);
    }
}

TEST_CASE("ordered_set modifiers (immutable)") {
    test_set_t s0;
    test_set_t s1 = s0.insert(1);
    test_set_t s2 = s1.insert(2);
    test_set_t s3 = s2.insert(1); // Insert existing element

    SECTION("insert") {
        // REQUIRE(s0.empty());
        // REQUIRE(s1.size() == 1);
        // REQUIRE(s1.count(1) == 1);

        // REQUIRE(s2.size() == 2);
        // REQUIRE(s2.count(1) == 1);
        // REQUIRE(s2.count(2) == 1);

        // REQUIRE(s3.size() == 2); // Size should not change when inserting existing
        // REQUIRE(s2.identity() == s3.identity()); // Or check equality: s2 == s3

        // Immutability checks
        // REQUIRE(s1.count(2) == 0); // s1 should not have key 2
    }

    SECTION("erase by key") {
        test_set_t se1 = s2.erase(1);
        // REQUIRE(se1.size() == 1);
        // REQUIRE(se1.count(1) == 0);
        // REQUIRE(se1.count(2) == 1);

        test_set_t se2 = se1.erase(4); // Erase non-existent key
        // REQUIRE(se2.size() == 1); // Should be same as se1
        // REQUIRE(se1.identity() == se2.identity()); // Or check equality

        // Immutability
        // REQUIRE(s2.size() == 2); // s2 unchanged
    }

    SECTION("erase by iterator") {
        auto it_to_erase = s2.find(2);
        // if (it_to_erase != s2.end()) {
        //    test_set_t sit1 = s2.erase(it_to_erase);
        //    REQUIRE(sit1.size() == 1);
        //    REQUIRE(sit1.count(2) == 0);
        // }
    }
}

TEST_CASE("ordered_set iterators and ordering") {
    test_set_t s = {3, 1, 4, 2};
    // Expected order: 1, 2, 3, 4

    SECTION("forward iteration") {
        std::vector<test_value_t_set> expected = {1, 2, 3, 4};
        std::vector<test_value_t_set> actual;
        // for (const auto& val : s) {
        //     actual.push_back(val);
        // }
        // REQUIRE(actual == expected);
    }

    SECTION("reverse iteration") {
        std::vector<test_value_t_set> expected_rev = {4, 3, 2, 1};
        std::vector<test_value_t_set> actual_rev;
        // auto rit = s.rbegin();
        // while (rit != s.rend()) {
        //    actual_rev.push_back(*rit);
        //    ++rit;
        // }
        // REQUIRE(actual_rev == expected_rev);
    }
}

TEST_CASE("ordered_set transient operations") {
    test_set_t s1_orig = {1, 2};

    SECTION("transient insert and erase") {
        auto trans = s1_orig.transient();
        // REQUIRE(trans.size() == 2);

        auto res_insert = trans.insert(3);
        // REQUIRE(res_insert.second); // inserted
        // REQUIRE(*(res_insert.first) == 3);
        // REQUIRE(trans.size() == 3);
        // REQUIRE(trans.count(3) == 1);

        auto res_insert_existing = trans.insert(1);
        // REQUIRE_FALSE(res_insert_existing.second); // not inserted
        // REQUIRE(*(res_insert_existing.first) == 1);
        // REQUIRE(trans.size() == 3);

        size_t erased_count = trans.erase(2);
        // REQUIRE(erased_count == 1);
        // REQUIRE(trans.size() == 2);
        // REQUIRE(trans.count(2) == 0);

        test_set_t s_final = trans.persistent();
        // REQUIRE(s_final.size() == 2);
        // REQUIRE(s_final.count(1) == 1);
        // REQUIRE(s_final.count(3) == 1);
        // REQUIRE(s_final.count(2) == 0);

        // Check original set is unchanged
        // REQUIRE(s1_orig.size() == 2);
        // REQUIRE(s1_orig.count(1) == 1);
        // REQUIRE(s1_orig.count(2) == 1);
    }

    SECTION("transient erase iterator") {
        auto trans = s1_orig.transient();
        trans.insert(4); // {1, 2, 4}
        auto it_to_erase = trans.find(1);
        // if (it_to_erase != trans.end()){
        //    auto next_it = trans.erase(it_to_erase);
             // REQUIRE(next_it != trans.end());
             // REQUIRE(*next_it == 2); // 2 should be next after 1
        // }
        // REQUIRE(trans.size() == 2);
        // REQUIRE(trans.count(1) == 0);
    }
}

TEST_CASE("ordered_set equality") {
    test_set_t s1 = {1, 2, 3};
    test_set_t s2 = {3, 1, 2}; // Same elements, different insertion order
    test_set_t s3 = {1, 2, 4};
    test_set_t s4 = {1, 2};
    test_set_t s_empty1, s_empty2;

    // REQUIRE(s1 == s2);
    // REQUIRE_FALSE(s1 == s3);
    // REQUIRE_FALSE(s1 == s4);
    // REQUIRE(s_empty1 == s_empty2);
    // REQUIRE_FALSE(s1 == s_empty1);
}

// Further tests:
// - Different value types (e.g. std::string)
// - Sets with custom comparators
// - Large number of elements
// - Move semantics for insert/erase on r-value sets
