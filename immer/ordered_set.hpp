//
// immer: immutable data structures for C++
// Copyright (C) 2023 Jules (Hypothetical Author)
//
// This software is distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://boost.org/LICENSE_1_0.txt
//

#pragma once

#include <immer/config.hpp>
#include <immer/memory_policy.hpp>
#include <immer/detail/iterator_facade.hpp>
// Placeholder for the actual B+ tree implementation
// #include <immer/detail/bptree/bplus_tree_impl.hpp>

#include <functional>
#include <stdexcept> // For std::out_of_range (though set doesn't have 'at')

namespace immer {

// Forward declaration for transient type
template <typename T,
          typename Compare,
          typename MemoryPolicy,
          int M>
class ordered_set_transient;

// Dummy type for set values in B+ tree, if not already in immer::detail
namespace detail {
    struct empty_t {};
} // namespace detail


template <typename T,
          typename Compare        = std::less<T>,
          typename MemoryPolicy   = default_memory_policy,
          int M                   = 32> // Default order for B+ Tree
class ordered_set
{
public:
    using key_type        = T;
    using value_type      = T; // For set, key_type and value_type are the same
    using size_type       = std::size_t; // Or a type from B+ tree impl
    using difference_type = std::ptrdiff_t;
    using key_compare     = Compare;
    using value_compare   = Compare; // For set, value_compare is same as key_compare
    using memory_policy   = MemoryPolicy;

    // Placeholder for actual B+ tree implementation type
    // Using the same placeholder as ordered_map, but V will be empty_t
    // struct bplus_tree_impl_placeholder { ... };
    // For brevity, assume ordered_map's placeholder can be adapted or a similar one exists
    // that takes <T, detail::empty_t, Compare, MemoryPolicy, M>

    // Adapting ordered_map's placeholder for set
    struct bplus_tree_impl_placeholder_set {
        using node_ptr = void*; // Placeholder
        node_ptr root_identity_ = nullptr;
        size_type size_ = 0;

        bplus_tree_impl_placeholder_set() = default;
        bplus_tree_impl_placeholder_set(node_ptr r, size_type s) : root_identity_(r), size_(s) {}

        static bplus_tree_impl_placeholder_set empty() { return {}; }
        size_type size() const { return size_; }
        bool empty() const { return size_ == 0; }

        template<typename Key>
        const Key* find_key(const Key& /*k*/, const Compare& /*cmp*/) const { return nullptr; }

        template<typename Key>
        bool count_key(const Key& /*k*/, const Compare& /*cmp*/) const { return false; }

        template<typename Edit, typename Value>
        bplus_tree_impl_placeholder_set insert_key(Edit /*e*/, Value&& /*v*/, const Compare& /*cmp*/) const { return *this; }

        template<typename Edit, typename Key>
        bplus_tree_impl_placeholder_set erase_key(Edit /*e*/, const Key& /*k*/, const Compare& /*cmp*/) const { return *this; }

        node_ptr root_ptr() const { return root_identity_; }

        using iterator_impl = void*;
        iterator_impl begin_impl() const { return nullptr; }
        iterator_impl end_impl() const { return nullptr; }
        iterator_impl rbegin_impl() const { return nullptr; }
        iterator_impl rend_impl() const { return nullptr; }

        template<typename Key>
        iterator_impl lower_bound_impl(const Key& /*k*/, const Compare& /*cmp*/) const { return nullptr; }
        template<typename Key>
        iterator_impl upper_bound_impl(const Key& /*k*/, const Compare& /*cmp*/) const { return nullptr; }
    };
    using impl_t = bplus_tree_impl_placeholder_set; // Replace with actual: detail::bplus_tree_impl<T, detail::empty_t, Compare, MemoryPolicy, M>;


private:
    using move_t = std::integral_constant<bool, MemoryPolicy::use_transient_rvalues>;

    impl_t impl_ = impl_t::empty();
    Compare compare_ = Compare{};

    // Private constructor for internal use
    ordered_set(impl_t i, Compare c) : impl_(std::move(i)), compare_(std::move(c)) {}

public:
    using transient_type = ordered_set_transient<T, Compare, MemoryPolicy, M>;

    // Iterators for set are always const_iterators effectively
    template <bool IsReverse> // IsConst is always true for set iterators
    struct basic_iterator
        : immer::detail::iterator_facade<
              basic_iterator<IsReverse>,
              const value_type, // Always const value_type
              std::bidirectional_iterator_tag,
              const value_type&, // Always const reference
              difference_type> {

        typename impl_t::iterator_impl current_ = nullptr;
        const impl_t* tree_impl_ = nullptr;
        Compare iter_compare_ = Compare{};

        basic_iterator() = default;
        basic_iterator(typename impl_t::iterator_impl iter, const impl_t* tree, Compare cmp)
            : current_(iter), tree_impl_(tree), iter_compare_(cmp) {}

        const value_type& dereference() const {
            // Simplified: real version gets from current_ in tree_impl_
            static value_type dummy{};
            return dummy;
        }
        bool equals(const basic_iterator& other) const { return current_ == other.current_; }
        void increment() { /* tree_impl_->advance(current_); */ }
        void decrement() { /* tree_impl_->retreat(current_); */ }
    };

    using iterator               = basic_iterator<false>; // Effectively a const_iterator
    using const_iterator         = basic_iterator<false>; // Standard alias
    using reverse_iterator       = basic_iterator<true>;  // Effectively a const_reverse_iterator
    using const_reverse_iterator = basic_iterator<true>;  // Standard alias

    ordered_set() = default;
    ordered_set(Compare c) : compare_(std::move(c)) {}

    ordered_set(std::initializer_list<value_type> values, Compare c = Compare{})
        : compare_(std::move(c)) {
        auto temp_transient = transient();
        for (const auto& v : values) {
            temp_transient.insert(v);
        }
        *this = temp_transient.persistent();
    }

    template <typename InputIt>
    ordered_set(InputIt first, InputIt last, Compare c = Compare{})
        : compare_(std::move(c)) {
        auto temp_transient = transient();
        for (; first != last; ++first) {
            temp_transient.insert(*first);
        }
        *this = temp_transient.persistent();
    }

    IMMER_NODISCARD const_iterator begin() const { return const_iterator{impl_.begin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator cbegin() const { return begin(); }

    IMMER_NODISCARD const_iterator end() const { return const_iterator{impl_.end_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator cend() const { return end(); }

    IMMER_NODISCARD const_reverse_iterator rbegin() const { return const_reverse_iterator{impl_.rbegin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_reverse_iterator crbegin() const { return rbegin(); }

    IMMER_NODISCARD const_reverse_iterator rend() const { return const_reverse_iterator{impl_.rend_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_reverse_iterator crend() const { return rend(); }

    IMMER_NODISCARD bool empty() const { return impl_.empty(); }
    IMMER_NODISCARD size_type size() const { return impl_.size(); }
    IMMER_NODISCARD key_compare key_comp() const { return compare_; }
    IMMER_NODISCARD value_compare value_comp() const { return compare_; }


    IMMER_NODISCARD size_type count(const T& key) const {
        return impl_.count_key(key, compare_) ? 1 : 0;
    }

    IMMER_NODISCARD const_iterator find(const T& key) const {
        auto iter_impl = impl_.lower_bound_impl(key, compare_);
        const_iterator it(iter_impl, &impl_, compare_);
        if (it == cend() || compare_(key, *it) || compare_(*it, key)) { // Check for exact match
            return cend();
        }
        return it;
    }

    IMMER_NODISCARD const_iterator lower_bound(const T& key) const {
        return const_iterator{impl_.lower_bound_impl(key, compare_), &impl_, compare_};
    }

    IMMER_NODISCARD const_iterator upper_bound(const T& key) const {
        return const_iterator{impl_.upper_bound_impl(key, compare_), &impl_, compare_};
    }

    IMMER_NODISCARD std::pair<const_iterator, const_iterator> equal_range(const T& key) const {
        return {lower_bound(key), upper_bound(key)};
    }

    // Modifiers
    IMMER_NODISCARD ordered_set insert(T value) const& {
         return ordered_set{impl_.insert_key(typename MemoryPolicy::transience_t::edit_t{}, std::move(value), compare_), compare_};
    }
    IMMER_NODISCARD decltype(auto) insert(T value) && {
        if constexpr (move_t::value) {
            impl_ = impl_.insert_key(typename MemoryPolicy::transience_t::edit_t::form_mut(), std::move(value), compare_);
            return std::move(*this);
        } else {
            return ordered_set{impl_.insert_key(typename MemoryPolicy::transience_t::edit_t{}, std::move(value), compare_), compare_};
        }
    }

    IMMER_NODISCARD ordered_set erase(const T& key) const& {
        return ordered_set{impl_.erase_key(typename MemoryPolicy::transience_t::edit_t{}, key, compare_), compare_};
    }
    IMMER_NODISCARD decltype(auto) erase(const T& key) && {
         if constexpr (move_t::value) {
            impl_ = impl_.erase_key(typename MemoryPolicy::transience_t::edit_t::form_mut(), key, compare_);
            return std::move(*this);
        } else {
            return ordered_set{impl_.erase_key(typename MemoryPolicy::transience_t::edit_t{}, key, compare_), compare_};
        }
    }

    IMMER_NODISCARD ordered_set erase(const_iterator pos) const& {
        if (pos == cend()) return *this;
        return erase(*pos);
    }
    IMMER_NODISCARD decltype(auto) erase(const_iterator pos) && {
         if (pos == cend()) {
             if constexpr (move_t::value) return std::move(*this); else return *this;
        }
        return erase(*pos);
    }

    IMMER_NODISCARD transient_type transient() const& {
        return transient_type{impl_, compare_};
    }
    IMMER_NODISCARD transient_type transient() && {
        return transient_type{std::move(impl_), std::move(compare_)};
    }

    void* identity() const { return impl_.root_ptr(); }

    bool operator==(const ordered_set& other) const {
        if (size() != other.size()) return false;
         if (compare_ != other.compare_ && size() > 0) {
            return false;
        }
        // Element-wise comparison
        auto it1 = cbegin();
        auto it2 = other.cbegin();
        while(it1 != cend() && it2 != other.cend()){
            if (compare_(*it1, *it2) || compare_(*it2, *it1)) { // If elements are not equivalent
                return false;
            }
            ++it1;
            ++it2;
        }
        return it1 == cend() && it2 == other.cend();
    }
    bool operator!=(const ordered_set& other) const { return !(*this == other); }

    friend class ordered_set_transient<T, Compare, MemoryPolicy, M>;
};


template <typename T,
          typename Compare,
          typename MemoryPolicy,
          int M>
class ordered_set_transient {
public:
    using immutable_set_type = ordered_set<T, Compare, MemoryPolicy, M>;
    using underlying_impl_t = typename immutable_set_type::impl_t;
    using value_type = typename immutable_set_type::value_type;
    using key_type = typename immutable_set_type::key_type;
    using size_type = typename immutable_set_type::size_type;
    using key_compare = typename immutable_set_type::key_compare;

    using iterator = typename immutable_set_type::iterator; // Effectively const_iterator
    using const_iterator = typename immutable_set_type::const_iterator;

private:
    underlying_impl_t impl_;
    Compare compare_;
    typename MemoryPolicy::transience_t::edit_t edit_token_;

    ordered_set_transient(underlying_impl_t impl, Compare c)
        : impl_(std::move(impl)),
          compare_(std::move(c)),
          edit_token_(MemoryPolicy::transience_t::edit_t::form_mut()) {}

    friend class ordered_set<T, Compare, MemoryPolicy, M>;

public:
    ordered_set_transient(ordered_set_transient&&) = default;
    ordered_set_transient& operator=(ordered_set_transient&&) = default;

    ordered_set_transient(const ordered_set_transient&) = delete;
    ordered_set_transient& operator=(const ordered_set_transient&) = delete;

    // Capacity
    IMMER_NODISCARD bool empty() const { return impl_.empty(); }
    IMMER_NODISCARD size_type size() const { return impl_.size(); }
    IMMER_NODISCARD key_compare key_comp() const { return compare_; }

    // Iterators
    IMMER_NODISCARD iterator begin() { return iterator{impl_.begin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator begin() const { return const_iterator{impl_.begin_impl(), &impl_, compare_}; } // same as above
    IMMER_NODISCARD const_iterator cbegin() const { return begin(); }

    IMMER_NODISCARD iterator end() { return iterator{impl_.end_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator end() const { return const_iterator{impl_.end_impl(), &impl_, compare_}; } // same as above
    IMMER_NODISCARD const_iterator cend() const { return end(); }

    // Lookups
    IMMER_NODISCARD iterator find(const T& key) {
        auto iter_impl = impl_.lower_bound_impl(key, compare_);
        iterator it(iter_impl, &impl_, compare_);
        if (it == end() || compare_(key, *it) || compare_(*it, key)) {
            return end();
        }
        return it;
    }
    IMMER_NODISCARD const_iterator find(const T& key) const {
        auto iter_impl = impl_.lower_bound_impl(key, compare_);
        const_iterator it(iter_impl, &impl_, compare_);
        if (it == cend() || compare_(key, *it) || compare_(*it, key)) {
            return cend();
        }
        return it;
    }

    IMMER_NODISCARD size_type count(const T& key) const {
        return impl_.count_key(key, compare_) ? 1 : 0;
    }

    // Modifiers
    std::pair<iterator, bool> insert(value_type value) {
        auto old_size = impl_.size();
        impl_ = impl_.insert_key(edit_token_, std::move(value), compare_);
        bool inserted = impl_.size() > old_size;
        // To return a valid iterator, find would be needed if not returned by insert_key
        return {find(value), inserted};
    }

    iterator insert(const_iterator hint, value_type value) {
        (void)hint; // Hint ignored in this skeleton
        return insert(std::move(value)).first;
    }

    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    size_type erase(const T& key) {
        auto old_size = impl_.size();
        impl_ = impl_.erase_key(edit_token_, key, compare_);
        return old_size - impl_.size(); // Number of elements erased
    }

    iterator erase(iterator pos) {
        if (pos == end()) return pos;
        T key_to_erase = *pos; // copy key before invalidating iterator
        erase(key_to_erase);
        // Return iterator to the element that followed the erased element
        return find(key_to_erase) == end() ? upper_bound(key_to_erase) : find(key_to_erase); // Simplified
    }
     iterator erase(const_iterator pos) { // Allow erasing with const_iterator
        if (pos == cend()) return end();
        T key_to_erase = *pos;
        erase(key_to_erase);
        return find(key_to_erase) == end() ? upper_bound(key_to_erase) : find(key_to_erase); // Simplified
    }


    IMMER_NODISCARD immutable_set_type persistent() {
        return immutable_set_type{std::move(impl_), std::move(compare_)};
    }
};

} // namespace immer
