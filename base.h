//
// Created by jaroslavp on 5/16/2022.
//

#pragma once

#define _USE_MATH_DEFINES

#include <array>
#include <chrono>
#include <concepts>
#include <filesystem>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <span>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

std::ostream& log_prefix(std::ostream& os, const char* file, int line) {
    thread_local std::filesystem::path current_path = std::filesystem::current_path();
    return os << "[" << std::filesystem::relative(std::filesystem::path(file), current_path).string() << ":" << line
              << "]: ";
}

using modern_clock = std::chrono::high_resolution_clock;
using time_point = modern_clock::time_point;

struct Timer {
    const char* file;
    int line;
    const char* name;
    time_point start = modern_clock::now();

    template<class Period = std::ratio<1>>
    [[nodiscard]] double get() const {
        return static_cast<std::chrono::duration<double, Period>>(modern_clock::now() - start).count();
    }

    ~Timer() {
#ifdef LOGGING
        log_prefix(std::cout, file, line);
        std::cout << name << " " << std::to_string(get()) << " seconds";
#endif
    }
};

#define TIMER(name)                                                                                                    \
    const Timer name { __FILE__, __LINE__, #name }

TIMER(timer);

namespace rng {

thread_local std::mt19937_64 mt(modern_clock::now().time_since_epoch().count());

template<std::integral T>
T get(T a, T b) {
    std::uniform_int_distribution<T> dist(a, b);
    return dist(mt);
}

template<std::integral T>
T get() {
    std::uniform_int_distribution<T> dist(0, std::numeric_limits<T>::max());
    return dist(mt);
}

template<std::floating_point T>
T get(T a, T b) {
    std::uniform_real_distribution<T> dist(a, b);
    return dist(mt);
}

template<std::floating_point T>
T get() {
    std::uniform_real_distribution<T> dist(0, 1);
    return dist(mt);
}

} // namespace rng

template<class T>
struct CustomHash {
    [[nodiscard]] static constexpr unsigned long long splitmix64(unsigned long long x) {
        // http://xorshift.di.unimi.it/splitmix64.c
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    std::size_t operator()(T x) const {
        static const unsigned long long FIXED_RANDOM = modern_clock::now().time_since_epoch().count();
        return static_cast<std::size_t>(splitmix64(std::hash<T>()(x) + FIXED_RANDOM));
    }
};

#if __has_include(<ext/pb_ds/assoc_container.hpp>)
#include <ext/pb_ds/assoc_container.hpp>

template<typename Key, typename Cmp_Fn = std::less<Key>>
using tree_set = __gnu_pbds::tree<Key, __gnu_pbds::null_type, Cmp_Fn, __gnu_pbds::rb_tree_tag,
                                  __gnu_pbds::tree_order_statistics_node_update>;

template<typename Key, typename Mapped, typename Cmp_Fn = std::less<Key>>
using tree_map =
    __gnu_pbds::tree<Key, Mapped, Cmp_Fn, __gnu_pbds::rb_tree_tag, __gnu_pbds::tree_order_statistics_node_update>;

template<typename Key>
using hash_set = __gnu_pbds::gp_hash_table<Key, __gnu_pbds::null_type, CustomHash<Key>>;

template<typename Key, typename Mapped>
using hash_map = __gnu_pbds::gp_hash_table<Key, Mapped, CustomHash<Key>>;

#else

template<typename Key>
using hash_set = std::unordered_set<Key, CustomHash<Key>>;

template<typename Key, typename Mapped>
using hash_map = std::unordered_map<Key, Mapped, CustomHash<Key>>;

#endif

template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

template<typename Test, template<typename...> class Ref>
inline constexpr bool is_specialization_v = is_specialization<Test, Ref>::value;

template<class T>
struct is_std_array : std::false_type {};

template<class T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template<class T>
struct is_span : std::false_type {};

template<class T, std::size_t Extent>
struct is_span<std::span<T, Extent>> : std::true_type {};

#ifdef PB_DS_ASSOC_CNTNR_HPP

template<class T>
struct is_tree_set : std::false_type {};

template<typename Key, typename Cmp_Fn, typename Tag,
         template<typename Const_Node_Iterator, typename Node_Iterator, typename Cmp_Fn_, typename Allocator_>
         class Node_Update,
         typename Allocator>
struct is_tree_set<__gnu_pbds::tree<Key, __gnu_pbds::null_type, Cmp_Fn, Tag, Node_Update, Allocator>> : std::true_type {
};

template<class T>
struct is_tree_map : std::false_type {};

template<typename Key, typename Mapped, typename Cmp_Fn, typename Tag,
         template<typename Const_Node_Iterator, typename Node_Iterator, typename Cmp_Fn_, typename Allocator_>
         class Node_Update,
         typename Allocator>
struct is_tree_map<__gnu_pbds::tree<Key, Mapped, Cmp_Fn, Tag, Node_Update, Allocator>> : std::true_type {};

template<class T>
struct is_gp_hash_set : std::false_type {};

template<typename Key, typename Hash_Fn, typename Eq_Fn, typename Comp_Probe_Fn, typename Probe_Fn,
         typename Resize_Policy, bool Store_Hash, typename Allocator>
struct is_gp_hash_set<__gnu_pbds::gp_hash_table<Key, __gnu_pbds::null_type, Hash_Fn, Eq_Fn, Comp_Probe_Fn, Probe_Fn,
                                                Resize_Policy, Store_Hash, Allocator>> : std::true_type {};

template<class T>
struct is_gp_hash_map : std::false_type {};

template<typename Key, typename Mapped, typename Hash_Fn, typename Eq_Fn, typename Comp_Probe_Fn, typename Probe_Fn,
         typename Resize_Policy, bool Store_Hash, typename Allocator>
struct is_gp_hash_map<__gnu_pbds::gp_hash_table<Key, Mapped, Hash_Fn, Eq_Fn, Comp_Probe_Fn, Probe_Fn, Resize_Policy,
                                                Store_Hash, Allocator>> : std::true_type {};

#endif

template<class T, class U = std::decay_t<T>>
concept Pair = is_specialization_v<U, std::pair>;

template<class T, class U = std::decay_t<T>>
concept Tuple = is_specialization_v<U, std::tuple>;

template<class T, class U = std::decay_t<T>>
concept List = is_std_array<U>::value || is_specialization_v<U, std::vector> || is_specialization_v<U, std::deque> ||
               is_specialization_v<U, std::forward_list> || is_specialization_v<U, std::list> || is_span<U>::value;

template<class T, class U = std::decay_t<T>>
concept BasicSet = is_specialization_v<U, std::set> || is_specialization_v<U, std::multiset> ||
                   is_specialization_v<U, std::unordered_set> || is_specialization_v<U, std::unordered_multiset>;

template<class T, class U = std::decay_t<T>>
concept BasicDict = is_specialization_v<U, std::map> || is_specialization_v<U, std::multimap> ||
                    is_specialization_v<U, std::unordered_map> || is_specialization_v<U, std::unordered_multimap>;

#ifdef PB_DS_ASSOC_CNTNR_HPP

template<class T, class U = std::decay_t<T>>
concept Set = BasicSet<U> || is_tree_set<U>::value || is_gp_hash_set<U>::value;

template<class T, class U = std::decay_t<T>>
concept Dict = BasicDict<U> || is_tree_map<U>::value || is_gp_hash_map<U>::value;

#else

template<class T>
concept Set = BasicSet<T>;

template<class T>
concept Dict = BasicDict<T>;

#endif

template<class T, class U = std::decay_t<T>>
concept Adaptor = is_specialization_v<U, std::stack> || is_specialization_v<U, std::queue> ||
                  is_specialization_v<U, std::priority_queue>;

template<Adaptor A>
typename A::container_type& get_container(A& a) {
    struct hack : A {
        static typename A::container_type& get(A& a) { return a.*&hack::c; }
    };

    return hack::get(a);
}

struct Fmt {
    std::string open;
    std::string sep;
    std::string close;
    std::string key_val_sep;
};

struct Printer {
    Fmt tuple;
    Fmt list;
    Fmt dict;

    std::ostream& print(std::ostream& os, auto&& x) { return os << x; }

    std::ostream& print(std::ostream& os, Pair auto&& p) {
        os << tuple.open;
        print(os, p.first);
        os << tuple.sep;
        print(os, p.second);
        return os << tuple.close;
    }

    template<Tuple T, std::size_t... Is>
    std::ostream& print_tuple_imp(std::ostream& os, T&& t, std::index_sequence<Is...>) {
        bool print_sep = false;
        auto print_elem = [&](auto&& x) {
            if (print_sep)
                os << tuple.sep;
            print(os, x);
            print_sep = true;
        };
        os << tuple.open;
        (print_elem(std::get<Is>(t)), ...);
        return os << tuple.close;
    }

    template<Tuple T, std::size_t Size = std::tuple_size<std::decay_t<T>>::value>
    std::ostream& print_tuple(std::ostream& os, T&& t) {
        return print_tuple_imp(os, t, std::make_index_sequence<Size>{});
    }

    std::ostream& print(std::ostream& os, Tuple auto&& t) { return print_tuple(os, t); }

    std::ostream& print(std::ostream& os, List auto&& l) {
        os << list.open;
        bool print_sep = false;
        for (auto it = l.begin(); it != l.end(); ++it) {
            if (print_sep)
                os << list.sep;
            print(os, *it);
            print_sep = true;
        }
        return os << list.close;
    }

    std::ostream& print(std::ostream& os, Set auto&& s) {
        os << dict.open;
        bool print_sep = false;
        for (auto it = s.begin(); it != s.end(); ++it) {
            if (print_sep)
                os << dict.sep;
            print(os, *it);
            print_sep = true;
        }
        return os << dict.close;
    }

    std::ostream& print(std::ostream& os, Dict auto&& d) {
        os << dict.open;
        bool print_sep = false;
        for (auto it = d.begin(); it != d.end(); ++it) {
            if (print_sep)
                os << dict.sep;
            print(os, it->first);
            os << dict.key_val_sep;
            print(os, it->second);
            print_sep = true;
        }
        return os << dict.close;
    }

    std::ostream& print(std::ostream& os, Adaptor auto&& a) { return print(os, get_container(a)); }
};

Fmt basic_fmt{"", " ", "", " "};
Fmt pair_fmt{"(", ", ", ")"};
Fmt list_fmt{"[", ", ", "]"};
Fmt dict_fmt{"{", ", ", "}", ": "};
Printer basic_printer{basic_fmt, basic_fmt, basic_fmt};
Printer pretty_printer{pair_fmt, list_fmt, dict_fmt};

std::ostream& operator<<(std::ostream& os, Pair auto&& p) { return basic_printer.print(os, p); }

std::ostream& operator<<(std::ostream& os, Tuple auto&& t) { return basic_printer.print(os, t); }

std::ostream& operator<<(std::ostream& os, List auto&& l) { return basic_printer.print(os, l); }

std::ostream& operator<<(std::ostream& os, Set auto&& s) { return basic_printer.print(os, s); }

std::ostream& operator<<(std::ostream& os, Dict auto&& d) { return basic_printer.print(os, d); }

std::ostream& operator<<(std::ostream& os, Adaptor auto&& a) { return basic_printer.print(os, a); }

[[nodiscard]] constexpr bool is_whitespace(char c) {
    std::array<char, 6> ws{' ', '\n', '\r', '\f', '\v', '\t'};
    return std::ranges::find(ws, c) != ws.end();
};

[[nodiscard]] constexpr bool is_quote(char c) { return c == '\'' || c == '"'; }

constexpr void match_quote(std::string_view s, int& i, char c) {
    ++i;
    while (s[i] != c)
        ++i;
}

constexpr void match_parenthesis(std::string_view s, int& i) {
    ++i;
    while (s[i] != ')') {
        if (is_quote(s[i]))
            match_quote(s, i, s[i]);
        else if (s[i] == '(')
            match_parenthesis(s, i);
        ++i;
    }
}

[[nodiscard]] consteval std::size_t count_args(std::string_view s) {
    if (std::ranges::all_of(s, is_whitespace))
        return 0;
    int i = 0;
    std::size_t count = 1;
    while (i < s.size()) {
        if (is_quote(s[i]))
            match_quote(s, i, s[i]);
        else if (s[i] == '(')
            match_parenthesis(s, i);
        else if (s[i] == ',')
            ++count;
        ++i;
    }
    return count;
}

template<std::size_t N>
[[nodiscard]] consteval std::array<std::string_view, N> split_args(std::string_view s) {
    std::array<std::string_view, N> args;
    int i = -1;
    for (std::size_t index = 0; index < N; ++index) {
        int j = i + 1;
        while (j < s.size() && s[j] != ',') {
            if (is_quote(s[j]))
                match_quote(s, j, s[j]);
            else if (s[j] == '(')
                match_parenthesis(s, j);
            ++j;
        }
        std::string_view arg = s.substr(i + 1, j - i - 1);
        while (is_whitespace(arg.front()))
            arg.remove_prefix(1);
        while (is_whitespace(arg.back()))
            arg.remove_suffix(1);
        args.at(index) = arg;
        i = j;
    }
    return args;
}

template<std::size_t N, typename... Args>
std::ostream& log(std::ostream& os, const char* file, int line, std::array<std::string_view, N>&& arg_names,
                  Args&&... args) {
    log_prefix(os, file, line);
    int index = 0;
    auto log_pair = [&](auto&& x) {
        if (index)
            os << ", ";
        os << arg_names[index] << " = ";
        pretty_printer.print(os, x);
        ++index;
    };
    (log_pair(args), ...);
    return os;
}

#ifdef LOGGING
#define LOG(...)                                                                                                       \
    log(std::cout, __FILE__, __LINE__, split_args<count_args(#__VA_ARGS__)>(#__VA_ARGS__), __VA_ARGS__) << '\n'        \
                                                                                                        << std::flush
#else
#define LOG(...)
#endif
