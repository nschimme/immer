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
#include <stdexcept>
#include <utility> // For std::pair

namespace immer {

// Forward declaration for transient type
template <typename K,
          typename T,
          typename Compare,
          typename MemoryPolicy,
          int M>
class ordered_map_transient;

// Forward declaration for iterators
namespace detail {
namespace bptree {
// Placeholder for iterator implementation
template <typename BPlusTreeImpl, bool IsConst, bool IsReverse>
class bplus_tree_iterator;
} // namespace bptree
} // namespace detail


template <typename K,
          typename T,
          typename Compare        = std::less<K>,
          typename MemoryPolicy   = default_memory_policy,
          int M                   = 32> // Default order for B+ Tree
class ordered_map
{
public:
    using key_type        = K;
    using mapped_type     = T;
    // std::map uses std::pair<const K, T> for value_type
    // This is important for iterators that provide references to elements.
    using value_type      = std::pair<const K, T>;
    using size_type       = std::size_t; // Or a type from B+ tree impl
    using difference_type = std::ptrdiff_t;
    using key_compare     = Compare;
    using memory_policy   = MemoryPolicy;

    // Placeholder for actual B+ tree implementation type
    // For now, use a dummy struct to make the code compile.
    struct bplus_tree_impl_placeholder {
        using node_ptr = void*; // Placeholder
        node_ptr root_identity_ = nullptr;
        size_type size_ = 0;

        bplus_tree_impl_placeholder() = default;
        bplus_tree_impl_placeholder(node_ptr r, size_type s) : root_identity_(r), size_(s) {}

        static bplus_tree_impl_placeholder empty() { return {}; }
        size_type size() const { return size_; }
        bool empty() const { return size_ == 0; }

        // Dummy methods to allow ordered_map to compile
        // These would interact with the actual B+ tree.
        template<typename Key>
        const value_type* find_val(const Key& /*k*/, const Compare& /*cmp*/) const { return nullptr; }

        template<typename Key>
        const T* find_mapped(const Key& /*k*/, const Compare& /*cmp*/) const { return nullptr; }

        template<typename Key>
        bool count_key(const Key& /*k*/, const Compare& /*cmp*/) const { return false; }

        template<typename Edit, typename Key, typename Value>
        bplus_tree_impl_placeholder set_val(Edit /*e*/, Key&& /*k*/, Value&& /*v*/, const Compare& /*cmp*/) const { return *this; }

        template<typename Edit, typename ValuePair>
        bplus_tree_impl_placeholder insert_val(Edit /*e*/, ValuePair&& /*vp*/, const Compare& /*cmp*/) const { return *this; }

        template<typename Edit, typename Key>
        bplus_tree_impl_placeholder erase_key(Edit /*e*/, const Key& /*k*/, const Compare& /*cmp*/) const { return *this; }

        template<typename Edit, typename IterImpl, typename Key, typename Fn>
        bplus_tree_impl_placeholder update_val(Edit /*e*/, IterImpl /*iter_impl*/, Key&& /*k*/, Fn&& /*fn*/, const Compare& /*cmp*/, bool /*is_update_if_exists*/) const { return *this; }

        node_ptr root_ptr() const { return root_identity_; }

        // Placeholder for iterator types within the impl
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
    using impl_t = bplus_tree_impl_placeholder; // Replace with actual: detail::bplus_tree_impl<K, T, Compare, MemoryPolicy, M>;

private:
    using move_t = std::integral_constant<bool, MemoryPolicy::use_transient_rvalues>;

    impl_t impl_ = impl_t::empty();
    Compare compare_ = Compare{}; // Store comparator if it can have state

    // Private constructor for internal use (e.g., from transient)
    ordered_map(impl_t i, Compare c) : impl_(std::move(i)), compare_(std::move(c)) {}

public:
    using transient_type = ordered_map_transient<K, T, Compare, MemoryPolicy, M>;

    // Placeholder Iterators
    template <bool IsConst, bool IsReverse>
    struct basic_iterator
        : immer::detail::iterator_facade<
              basic_iterator<IsConst, IsReverse>,
              std::conditional_t<IsConst, const value_type, value_type>,
              std::bidirectional_iterator_tag,
              std::conditional_t<IsConst, const value_type&, value_type&>,
              difference_type> {
        // This would hold state from impl_t::iterator_impl
        typename impl_t::iterator_impl current_ = nullptr;
        const impl_t* tree_impl_ = nullptr; // Pointer to the tree implementation for navigation
        Compare iter_compare_ = Compare{};

        basic_iterator() = default;
        basic_iterator(typename impl_t::iterator_impl iter, const impl_t* tree, Compare cmp)
            : current_(iter), tree_impl_(tree), iter_compare_(cmp) {}

        // iterator_facade requirements
        decltype(auto) dereference() const {
            // This is highly simplified. Real version would get from current_
            static value_type dummy{K{}, T{}};
            return dummy;
        }
        bool equals(const basic_iterator& other) const { return current_ == other.current_; }
        void increment() { /* tree_impl_->advance(current_); */ }
        void decrement() { /* tree_impl_->retreat(current_); */ }
    };

    using iterator               = basic_iterator<false, false>;
    using const_iterator         = basic_iterator<true, false>;
    using reverse_iterator       = basic_iterator<false, true>;
    using const_reverse_iterator = basic_iterator<true, true>;

    ordered_map() = default;
    ordered_map(Compare c) : compare_(std::move(c)) {}

    ordered_map(std::initializer_list<value_type> values, Compare c = Compare{})
        : compare_(std::move(c)) {
        auto temp_transient = transient();
        for (const auto& v : values) {
            temp_transient.insert(v);
        }
        *this = temp_transient.persistent();
    }

    template <typename InputIt>
    ordered_map(InputIt first, InputIt last, Compare c = Compare{})
        : compare_(std::move(c)) {
        auto temp_transient = transient();
        for (; first != last; ++first) {
            temp_transient.insert(*first);
        }
        *this = temp_transient.persistent();
    }

    IMMER_NODISCARD iterator begin() { return iterator{impl_.begin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator begin() const { return const_iterator{impl_.begin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator cbegin() const { return const_iterator{impl_.begin_impl(), &impl_, compare_}; }

    IMMER_NODISCARD iterator end() { return iterator{impl_.end_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator end() const { return const_iterator{impl_.end_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator cend() const { return const_iterator{impl_.end_impl(), &impl_, compare_}; }

    IMMER_NODISCARD reverse_iterator rbegin() { return reverse_iterator{impl_.rbegin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_reverse_iterator rbegin() const { return const_reverse_iterator{impl_.rbegin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_reverse_iterator crbegin() const { return const_reverse_iterator{impl_.rbegin_impl(), &impl_, compare_}; }

    IMMER_NODISCARD reverse_iterator rend() { return reverse_iterator{impl_.rend_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_reverse_iterator rend() const { return const_reverse_iterator{impl_.rend_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_reverse_iterator crend() const { return const_reverse_iterator{impl_.rend_impl(), &impl_, compare_}; }

    IMMER_NODISCARD bool empty() const { return impl_.empty(); }
    IMMER_NODISCARD size_type size() const { return impl_.size(); }
    IMMER_NODISCARD key_compare key_comp() const { return compare_; }

    IMMER_NODISCARD size_type count(const K& k) const {
        return impl_.count_key(k, compare_) ? 1 : 0;
    }

    IMMER_NODISCARD const_iterator find(const K& k) const {
        // This would involve a specific find operation in bplus_tree_impl
        // that returns its internal iterator representation.
        auto iter_impl = impl_.lower_bound_impl(k, compare_);
        const_iterator it(iter_impl, &impl_, compare_);
        // If not found or key mismatch, return end()
        if (it == cend() || compare_(k, it->first) || compare_(it->first, k)) {
            return cend();
        }
        return it;
    }
    IMMER_NODISCARD iterator find(const K& k) {
        // Non-const version, similar logic
        auto iter_impl = impl_.lower_bound_impl(k, compare_);
        iterator it(iter_impl, &impl_, compare_);
        if (it == end() || compare_(k, it->first) || compare_(it->first, k)) {
            return end();
        }
        return it;
    }

    IMMER_NODISCARD const T& at(const K& k) const {
        const T* val = impl_.find_mapped(k, compare_);
        if (!val) {
            IMMER_THROW(std::out_of_range{"ordered_map::at: key not found"});
        }
        return *val;
    }

    IMMER_NODISCARD const T& operator[](const K& k) const {
        const T* val = impl_.find_mapped(k, compare_);
        if (!val) {
            // This behavior (returning ref to default-constructed T) is from immer::map.
            // It's a bit tricky for immutable map. A real impl might need to insert
            // and return, but that's not const. Or, return a static default T.
            // For now, let's assume it finds or throws like at(), or returns default.
            // Let's follow immer::map's lead and return a static default.
            static T default_val{};
            return default_val;
        }
        return *val;
    }

    IMMER_NODISCARD const_iterator lower_bound(const K& k) const {
        return const_iterator{impl_.lower_bound_impl(k, compare_), &impl_, compare_};
    }
    IMMER_NODISCARD iterator lower_bound(const K& k) {
        return iterator{impl_.lower_bound_impl(k, compare_), &impl_, compare_};
    }

    IMMER_NODISCARD const_iterator upper_bound(const K& k) const {
        return const_iterator{impl_.upper_bound_impl(k, compare_), &impl_, compare_};
    }
    IMMER_NODISCARD iterator upper_bound(const K& k) {
        return iterator{impl_.upper_bound_impl(k, compare_), &impl_, compare_};
    }

    IMMER_NODISCARD std::pair<const_iterator, const_iterator> equal_range(const K& k) const {
        return {lower_bound(k), upper_bound(k)};
    }
    IMMER_NODISCARD std::pair<iterator, iterator> equal_range(const K& k) {
        return {lower_bound(k), upper_bound(k)};
    }

    // Modifiers
    IMMER_NODISCARD ordered_map set(K k, T v) const& {
        return ordered_map{impl_.set_val(typename MemoryPolicy::transience_t::edit_t{}, std::move(k), std::move(v), compare_), compare_};
    }
    IMMER_NODISCARD decltype(auto) set(K k, T v) && {
        if constexpr (move_t::value) {
            impl_ = impl_.set_val(typename MemoryPolicy::transience_t::edit_t::form_mut(), std::move(k), std::move(v), compare_);
            return std::move(*this);
        } else {
            return ordered_map{impl_.set_val(typename MemoryPolicy::transience_t::edit_t{}, std::move(k), std::move(v), compare_), compare_};
        }
    }

    IMMER_NODISCARD ordered_map insert(value_type value) const& {
         return ordered_map{impl_.insert_val(typename MemoryPolicy::transience_t::edit_t{}, std::move(value), compare_), compare_};
    }
    IMMER_NODISCARD decltype(auto) insert(value_type value) && {
        if constexpr (move_t::value) {
            impl_ = impl_.insert_val(typename MemoryPolicy::transience_t::edit_t::form_mut(), std::move(value), compare_);
            return std::move(*this);
        } else {
            return ordered_map{impl_.insert_val(typename MemoryPolicy::transience_t::edit_t{}, std::move(value), compare_), compare_};
        }
    }

    IMMER_NODISCARD ordered_map erase(const K& k) const& {
        return ordered_map{impl_.erase_key(typename MemoryPolicy::transience_t::edit_t{}, k, compare_), compare_};
    }
    IMMER_NODISCARD decltype(auto) erase(const K& k) && {
         if constexpr (move_t::value) {
            impl_ = impl_.erase_key(typename MemoryPolicy::transience_t::edit_t::form_mut(), k, compare_);
            return std::move(*this);
        } else {
            return ordered_map{impl_.erase_key(typename MemoryPolicy::transience_t::edit_t{}, k, compare_), compare_};
        }
    }

    // Erase by iterator would require the iterator to provide enough info to bplus_tree_impl
    // For now, let's assume it converts to key erase.
    IMMER_NODISCARD ordered_map erase(const_iterator pos) const& {
        if (pos == cend()) return *this;
        return erase(pos->first);
    }
    IMMER_NODISCARD decltype(auto) erase(const_iterator pos) && {
        if (pos == cend()) { // or this->cend() if it's a member
             if constexpr (move_t::value) return std::move(*this); else return *this;
        }
        // This might be inefficient if key is copied, direct iter erase in impl is better
        return erase(pos->first);
    }

    template <typename Fn>
    IMMER_NODISCARD ordered_map update(K k, Fn&& fn) const& {
        // fn should take std::optional<const T&> and return T
        // The bplus_tree_impl::update_val would handle finding or not.
        // This is a simplified call, real one would pass iter_impl from find.
        auto iter_impl = impl_.find_val(k, compare_); // Or some marker for "key k"
        return ordered_map{impl_.update_val(typename MemoryPolicy::transience_t::edit_t{}, iter_impl, std::move(k), std::forward<Fn>(fn), compare_, false), compare_};
    }
    // && overload for update

    template <typename Fn>
    IMMER_NODISCARD ordered_map update_if_exists(K k, Fn&& fn) const& {
        // fn should take const T& and return T
        auto iter_impl = impl_.find_val(k, compare_); // Or some marker for "key k"
        return ordered_map{impl_.update_val(typename MemoryPolicy::transience_t::edit_t{}, iter_impl, std::move(k), std::forward<Fn>(fn), compare_, true), compare_};
    }
    // && overload for update_if_exists

    IMMER_NODISCARD transient_type transient() const& {
        return transient_type{impl_, compare_};
    }
    IMMER_NODISCARD transient_type transient() && {
        return transient_type{std::move(impl_), std::move(compare_)};
    }

    void* identity() const { return impl_.root_ptr(); }

    bool operator==(const ordered_map& other) const {
        if (size() != other.size()) return false;
        if (compare_ != other.compare_ && size() > 0) {
            // Comparators are different. This is tricky.
            // std::map requires equivalent comparators for equality.
            // For simplicity, if comparators differ and map is not empty, assume not equal.
            // A more robust check would be element-wise comparison.
            return false;
        }
        // Element-wise comparison
        auto it1 = cbegin();
        auto it2 = other.cbegin();
        while(it1 != cend() && it2 != other.cend()){
            if (compare_(it1->first, it2->first) || compare_(it2->first, it1->first) || it1->second != it2->second) {
                return false;
            }
            ++it1;
            ++it2;
        }
        return it1 == cend() && it2 == other.cend();
    }
    bool operator!=(const ordered_map& other) const { return !(*this == other); }

    // Friend declaration for transient to access private constructor and impl_
    friend class ordered_map_transient<K, T, Compare, MemoryPolicy, M>;
};


template <typename K,
          typename T,
          typename Compare,
          typename MemoryPolicy,
          int M>
class ordered_map_transient {
public:
    using immutable_map_type = ordered_map<K, T, Compare, MemoryPolicy, M>;
    using underlying_impl_t = typename immutable_map_type::impl_t;
    using value_type = typename immutable_map_type::value_type;
    using key_type = typename immutable_map_type::key_type;
    using mapped_type = typename immutable_map_type::mapped_type;
    using size_type = typename immutable_map_type::size_type;
    using key_compare = typename immutable_map_type::key_compare;

    // Transient iterators could be simpler if they don't support mutation through dereference
    // For now, let's assume they are similar to immutable iterators but operate on the transient's impl.
    using iterator = typename immutable_map_type::iterator; // Or a mutable variant if needed
    using const_iterator = typename immutable_map_type::const_iterator;


private:
    underlying_impl_t impl_;
    Compare compare_;
    typename MemoryPolicy::transience_t::edit_t edit_token_;

    // Private constructor for internal use by ordered_map's transient()
    ordered_map_transient(underlying_impl_t impl, Compare c)
        : impl_(std::move(impl)),
          compare_(std::move(c)),
          edit_token_(MemoryPolicy::transience_t::edit_t::form_mut()) {}

    friend class ordered_map<K, T, Compare, MemoryPolicy, M>;

public:
    ordered_map_transient(ordered_map_transient&&) = default;
    ordered_map_transient& operator=(ordered_map_transient&&) = default;

    // No copy constructor/assignment for transients
    ordered_map_transient(const ordered_map_transient&) = delete;
    ordered_map_transient& operator=(const ordered_map_transient&) = delete;

    // Capacity
    IMMER_NODISCARD bool empty() const { return impl_.empty(); }
    IMMER_NODISCARD size_type size() const { return impl_.size(); }
    IMMER_NODISCARD key_compare key_comp() const { return compare_; }

    // Iterators (simplified, assumes they work on the current impl_)
    IMMER_NODISCARD iterator begin() { return iterator{impl_.begin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator begin() const { return const_iterator{impl_.begin_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator cbegin() const { return begin(); }

    IMMER_NODISCARD iterator end() { return iterator{impl_.end_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator end() const { return const_iterator{impl_.end_impl(), &impl_, compare_}; }
    IMMER_NODISCARD const_iterator cend() const { return end(); }

    // Lookups
    IMMER_NODISCARD iterator find(const K& k) {
        auto iter_impl = impl_.lower_bound_impl(k, compare_);
        iterator it(iter_impl, &impl_, compare_);
        if (it == end() || compare_(k, it->first) || compare_(it->first, k)) {
            return end();
        }
        return it;
    }
    IMMER_NODISCARD const_iterator find(const K& k) const {
        auto iter_impl = impl_.lower_bound_impl(k, compare_);
        const_iterator it(iter_impl, &impl_, compare_);
        if (it == cend() || compare_(k, it->first) || compare_(it->first, k)) {
            return cend();
        }
        return it;
    }

    IMMER_NODISCARD size_type count(const K& k) const {
        return impl_.count_key(k, compare_) ? 1 : 0;
    }

    // mapped_type& at(const K& k) - Non-const at for transients is possible
    // T& operator[](const K& k) - Non-const operator[] for transients is possible

    // Modifiers
    // Return type for modifiers in transient is often void or iterator/pair<iterator, bool>
    std::pair<iterator, bool> insert(value_type value) {
        // Placeholder: actual insert might return info about success/location
        // For now, assume impl_ is updated. The real impl_.insert_val might need to return size change.
        auto old_size = impl_.size();
        impl_ = impl_.insert_val(edit_token_, std::move(value), compare_);
        bool inserted = impl_.size() > old_size;
        // To return a valid iterator, find would be needed if not returned by insert_val
        return {find(value.first), inserted};
    }

    iterator insert(const_iterator hint, value_type value) {
        // Hint is mostly for performance in underlying tree, ignored in this skeleton
        (void)hint;
        return insert(std::move(value)).first;
    }

    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    void set(K k, T v) { // Or key_type, mapped_type
        impl_ = impl_.set_val(edit_token_, std::move(k), std::move(v), compare_);
    }

    size_type erase(const K& k) {
        auto old_size = impl_.size();
        impl_ = impl_.erase_key(edit_token_, k, compare_);
        return old_size - impl_.size(); // Number of elements erased
    }

    iterator erase(iterator pos) {
        if (pos == end()) return pos; // or some other end iterator
        K key_to_erase = pos->first; // copy key before invalidating iterator by erase
        erase(key_to_erase);
        // Return iterator to the element that followed the erased element.
        // This would be upper_bound(key_to_erase) in the modified tree.
        return find(key_to_erase) == end() ? upper_bound(key_to_erase) : find(key_to_erase); // Simplified
    }
     iterator erase(const_iterator pos) { // Allow erasing with const_iterator
        if (pos == cend()) return end();
        K key_to_erase = pos->first;
        erase(key_to_erase);
        return find(key_to_erase) == end() ? upper_bound(key_to_erase) : find(key_to_erase); // Simplified
    }


    template <typename Fn>
    void update(K k, Fn&& fn) { // Fn takes std::optional<T&> returns T, or T& returns void
        // This is complex. The bplus_tree_impl needs to support mutable updates.
        // For now, let's assume a simplified path.
        // A real update might involve finding the element, then applying fn.
        // If fn creates a new value, it replaces the old one.
        // If element not found, and fn can provide a default, it inserts.
        // This placeholder just calls the impl's update.
        impl_ = impl_.update_val(edit_token_, /*iter_impl or key*/ impl_.find_val(k, compare_), std::move(k), std::forward<Fn>(fn), compare_, false);
    }

    IMMER_NODISCARD immutable_map_type persistent() {
        // The edit_token_ might do some finalization work here if it's ref-counted itself
        // or if the memory policy requires it.
        // For default transience policy, form_mut() gives a token that doesn't need explicit release usually.
        return immutable_map_type{std::move(impl_), std::move(compare_)};
    }
};

} // namespace immer
