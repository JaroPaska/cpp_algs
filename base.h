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
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

std::filesystem::path current_path = std::filesystem::current_path();

std::ostream& log_prefix(std::ostream& os, const char* file, int line) {
    std::filesystem::path file_path(file);
    return os << "[" << std::filesystem::relative(file_path, current_path).string() << ":" << line << "]: ";
}

using modern_clock = std::chrono::high_resolution_clock;
using time_point = modern_clock::time_point;

time_point now() { return modern_clock::now(); }

struct Timer {
    const char* file;
    int line;
    const char* name;
    time_point start = now();

    template<class Period = std::ratio<1>>
    [[nodiscard]] double get() const {
        return static_cast<std::chrono::duration<double, Period>>(now() - start).count();
    }

    ~Timer() {
#ifndef NDEBUG
        log_prefix(std::cout, file, line);
        std::cout << name << " " << std::to_string(get<std::milli>()) << " ms";
#endif
    }
};

#define TIMER(name)                                                                                                    \
    Timer name { __FILE__, __LINE__, #name }

TIMER(timer);

namespace rng {

thread_local std::mt19937_64 mt(now().time_since_epoch().count());

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

struct CustomHash {
    [[nodiscard]] static constexpr unsigned long long splitmix64(unsigned long long x) {
        // http://xorshift.di.unimi.it/splitmix64.c
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    std::size_t operator()(unsigned long long x) const {
        static const unsigned long long FIXED_RANDOM = now().time_since_epoch().count();
        return static_cast<std::size_t>(splitmix64(x + FIXED_RANDOM));
    }
};

#if __has_include(<ext/pb_ds/assoc_container.hpp>)
#include <ext/pb_ds/assoc_container.hpp>

template<typename Key>
using ordered_set = __gnu_pbds::tree<Key, __gnu_pbds::null_type, std::less<Key>, __gnu_pbds::rb_tree_tag,
                                     __gnu_pbds::tree_order_statistics_node_update>;

template<typename Key, typename Mapped>
using ordered_map = __gnu_pbds::tree<Key, Mapped, std::less<Key>, __gnu_pbds::rb_tree_tag,
                                     __gnu_pbds::tree_order_statistics_node_update>;

template<typename Key>
using hash_set = __gnu_pbds::gp_hash_table<Key, __gnu_pbds::null_type, CustomHash>;

template<typename Key, typename Mapped>
using hash_map = __gnu_pbds::gp_hash_table<Key, Mapped, CustomHash>;

#else

template<typename Key>
using hash_set = std::unordered_set<Key, CustomHash>;

template<typename Key, typename Mapped>
using hash_map = std::unordered_map<Key, Mapped, CustomHash>;

#endif

template<class Adapter>
typename Adapter::container_type& get_container(Adapter& a) {
    struct hack : Adapter {
        static typename Adapter::container_type& get(Adapter& a) { return a.*&hack::c; }
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

    template<class T1, class T2>
    std::ostream& print_pair(std::ostream& os, const std::pair<T1, T2>& p) const {
        os << tuple.open;
        print(os, p.first);
        os << tuple.sep;
        print(os, p.second);
        return os << tuple.close;
    }

    template<class InputIt>
    std::ostream& print_list(std::ostream& os, InputIt first, InputIt last) const {
        os << list.open;
        for (bool print_sep = false; first != last; ++first) {
            if (print_sep)
                os << list.sep;
            print(os, *first);
            print_sep = true;
        }
        return os << list.close;
    }

    template<class InputIt>
    std::ostream& print_set(std::ostream& os, InputIt first, InputIt last) const {
        os << dict.open;
        for (bool print_sep = false; first != last; ++first) {
            if (print_sep)
                os << dict.sep;
            print(os, *first);
            print_sep = true;
        }
        return os << dict.close;
    }

    template<class InputIt>
    std::ostream& print_dict(std::ostream& os, InputIt first, InputIt last) const {
        os << dict.open;
        for (bool print_sep = false; first != last; ++first) {
            if (print_sep)
                os << dict.sep;
            print(os, first->first);
            os << dict.key_val_sep;
            print(os, first->second);
            print_sep = true;
        }
        return os << dict.close;
    }

    template<class T>
    std::ostream& print(std::ostream& os, T&& x) const {
        return os << x;
    }

    template<class T1, class T2>
    std::ostream& print(std::ostream& os, std::pair<T1, T2>& p) const {
        return print_pair(os, p);
    }

    template<class T1, class T2>
    std::ostream& print(std::ostream& os, const std::pair<T1, T2>& p) const {
        return print_pair(os, p);
    }

    template<class T1, class T2>
    std::ostream& print(std::ostream& os, std::pair<T1, T2>&& p) const {
        return print_pair(os, p);
    }

    template<class T, std::size_t N>
    std::ostream& print(std::ostream& os, const std::array<T, N>& a) const {
        return print_list(os, a.begin(), a.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, std::vector<T, Allocator>& v) const {
        return print_list(os, v.begin(), v.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, const std::vector<T, Allocator>& v) const {
        return print_list(os, v.begin(), v.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, std::vector<T, Allocator>&& v) const {
        return print_list(os, v.begin(), v.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, std::deque<T, Allocator>& d) const {
        return print_list(os, d.begin(), d.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, const std::deque<T, Allocator>& d) const {
        return print_list(os, d.begin(), d.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, std::deque<T, Allocator>&& d) const {
        return print_list(os, d.begin(), d.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, const std::forward_list<T, Allocator>& fl) const {
        return print_list(os, fl.begin(), fl.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, const std::list<T, Allocator>& l) const {
        return print_list(os, l.begin(), l.end());
    }

    template<class Key, class Compare, class Allocator>
    std::ostream& print(std::ostream& os, const std::set<Key, Compare, Allocator>& s) const {
        return print_set(os, s.begin(), s.end());
    }

    template<class Key, class T, class Compare, class Allocator>
    std::ostream& print(std::ostream& os, const std::map<Key, T, Compare, Allocator>& m) const {
        return print_dict(os, m.begin(), m.end());
    }

    template<class Key, class Compare, class Allocator>
    std::ostream& print(std::ostream& os, const std::multiset<Key, Compare, Allocator>& s) const {
        return print_set(os, s.begin(), s.end());
    }

    template<class Key, class T, class Compare, class Allocator>
    std::ostream& print(std::ostream& os, const std::multimap<Key, T, Compare, Allocator>& m) const {
        return print_dict(os, m.begin(), m.end());
    }

    template<class Key, class Hash, class KeyEqual, class Allocator>
    std::ostream& print(std::ostream& os, const std::unordered_set<Key, Hash, KeyEqual, Allocator>& us) const {
        return print_set(os, us.begin(), us.end());
    }

    template<class Key, class T, class Hash, class KeyEqual, class Allocator>
    std::ostream& print(std::ostream& os, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& um) const {
        return print_dict(os, um.begin(), um.end());
    }

    template<class Key, class Hash, class KeyEqual, class Allocator>
    std::ostream& print(std::ostream& os, const std::unordered_multiset<Key, Hash, KeyEqual, Allocator>& us) const {
        return print_set(os, us.begin(), us.end());
    }

    template<class Key, class T, class Hash, class KeyEqual, class Allocator>
    std::ostream& print(std::ostream& os, const std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& um) const {
        return print_dict(os, um.begin(), um.end());
    }

    template<class T, class Container>
    std::ostream& print(std::ostream& os, std::stack<T, Container>& s) const {
        return print(os, get_container(s));
    }

    template<class T, class Container>
    std::ostream& print(std::ostream& os, std::queue<T, Container>& q) const {
        return print(os, get_container(q));
    }

    template<class T, class Container, class Compare>
    std::ostream& print(std::ostream& os, std::priority_queue<T, Container, Compare>& pq) const {
        return print(os, get_container(pq));
    }

#ifdef PB_DS_ASSOC_CNTNR_HPP

    template<typename Key, typename Cmp_Fn, typename Tag,
             template<typename Const_Node_Iterator, typename Node_Iterator, typename Cmp_Fn_, typename Allocator_>
             class Node_Update,
             typename Allocator>
    std::ostream&
    print(std::ostream& os,
          const __gnu_pbds::tree<Key, __gnu_pbds::null_type, Cmp_Fn, Tag, Node_Update, Allocator>& t) const {
        return print_set(os, t.begin(), t.end());
    }

    template<typename Key, typename Mapped, typename Cmp_Fn, typename Tag,
             template<typename Const_Node_Iterator, typename Node_Iterator, typename Cmp_Fn_, typename Allocator_>
             class Node_Update,
             typename Allocator>
    std::ostream& print(std::ostream& os,
                        const __gnu_pbds::tree<Key, Mapped, Cmp_Fn, Tag, Node_Update, Allocator>& t) const {
        return print_dict(os, t.begin(), t.end());
    }

    template<typename Key, typename Hash_Fn, typename Eq_Fn, typename Comp_Probe_Fn, typename Probe_Fn,
             typename Resize_Policy, bool Store_Hash, typename Allocator>
    std::ostream& print(std::ostream& os,
                        const __gnu_pbds::gp_hash_table<Key, __gnu_pbds::null_type, Hash_Fn, Eq_Fn, Comp_Probe_Fn,
                                                        Probe_Fn, Resize_Policy, Store_Hash, Allocator>& ght) const {
        return print_set(os, ght.begin(), ght.end());
    }

    template<typename Key, typename Mapped, typename Hash_Fn, typename Eq_Fn, typename Comp_Probe_Fn, typename Probe_Fn,
             typename Resize_Policy, bool Store_Hash, typename Allocator>
    std::ostream& print(std::ostream& os,
                        const __gnu_pbds::gp_hash_table<Key, Mapped, Hash_Fn, Eq_Fn, Comp_Probe_Fn, Probe_Fn,
                                                        Resize_Policy, Store_Hash, Allocator>& ght) const {
        return print_dict(os, ght.begin(), ght.end());
    }

#endif
};

Fmt basic_fmt{"", " ", "", " "};
Fmt pair_fmt{"(", ", ", ")"};
Fmt list_fmt{"[", ", ", "]"};
Fmt dict_fmt{"{", ", ", "}", ": "};
const Printer basic_printer{basic_fmt, basic_fmt, basic_fmt};
const Printer pretty_printer{pair_fmt, list_fmt, dict_fmt};

template<class T1, class T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p) {
    return basic_printer.print(os, p);
}

template<class T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& a) {
    return basic_printer.print(os, a);
}

template<class T, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::vector<T, Allocator>& v) {
    return basic_printer.print(os, v);
}

template<class T, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::deque<T, Allocator>& d) {
    return basic_printer.print(os, d);
}

template<class T, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::forward_list<T, Allocator>& fl) {
    return basic_printer.print(os, fl);
}

template<class T, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::list<T, Allocator>& l) {
    return basic_printer.print(os, l);
}

template<class Key, class Compare, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::set<Key, Compare, Allocator>& s) {
    return basic_printer.print(os, s);
}

template<class Key, class T, class Compare, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::map<Key, T, Compare, Allocator>& m) {
    return basic_printer.print(os, m);
}

template<class Key, class Compare, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::multiset<Key, Compare, Allocator>& s) {
    return basic_printer.print(os, s);
}

template<class Key, class T, class Compare, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::multimap<Key, T, Compare, Allocator>& m) {
    return basic_printer.print(os, m);
}

template<class Key, class Hash, class KeyEqual, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<Key, Hash, KeyEqual, Allocator>& us) {
    return basic_printer.print(os, us);
}

template<class Key, class T, class Hash, class KeyEqual, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& um) {
    return basic_printer.print(os, um);
}

template<class Key, class Hash, class KeyEqual, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_multiset<Key, Hash, KeyEqual, Allocator>& us) {
    return basic_printer.print(os, us);
}

template<class Key, class T, class Hash, class KeyEqual, class Allocator>
std::ostream& operator<<(std::ostream& os, const std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& um) {
    return basic_printer.print(os, um);
}

template<class T, class Container>
std::ostream& operator<<(std::ostream& os, std::stack<T, Container>& s) {
    return basic_printer.print(os, s);
}

template<class T, class Container>
std::ostream& operator<<(std::ostream& os, std::queue<T, Container>& q) {
    return basic_printer.print(os, q);
}

template<class T, class Container, class Compare>
std::ostream& operator<<(std::ostream& os, std::priority_queue<T, Container, Compare>& pq) {
    return basic_printer.print(os, pq);
}

#ifdef PB_DS_ASSOC_CNTNR_HPP

template<typename Key, typename Cmp_Fn, typename Tag,
         template<typename Const_Node_Iterator, typename Node_Iterator, typename Cmp_Fn_, typename Allocator_>
         class Node_Update,
         typename Allocator>
std::ostream& operator<<(std::ostream& os,
                         const __gnu_pbds::tree<Key, __gnu_pbds::null_type, Cmp_Fn, Tag, Node_Update, Allocator>& t) {
    return basic_printer.print(os, t);
}

template<typename Key, typename Mapped, typename Cmp_Fn, typename Tag,
         template<typename Const_Node_Iterator, typename Node_Iterator, typename Cmp_Fn_, typename Allocator_>
         class Node_Update,
         typename Allocator>
std::ostream& operator<<(std::ostream& os,
                         const __gnu_pbds::tree<Key, Mapped, Cmp_Fn, Tag, Node_Update, Allocator>& t) {
    return basic_printer.print(os, t);
}

template<typename Key, typename Hash_Fn, typename Eq_Fn, typename Comp_Probe_Fn, typename Probe_Fn,
         typename Resize_Policy, bool Store_Hash, typename Allocator>
std::ostream& operator<<(std::ostream& os,
                         const __gnu_pbds::gp_hash_table<Key, __gnu_pbds::null_type, Hash_Fn, Eq_Fn, Comp_Probe_Fn,
                                                         Probe_Fn, Resize_Policy, Store_Hash, Allocator>& ght) {
    return basic_printer.print(os, ght);
}

template<typename Key, typename Mapped, typename Hash_Fn, typename Eq_Fn, typename Comp_Probe_Fn, typename Probe_Fn,
         typename Resize_Policy, bool Store_Hash, typename Allocator>
std::ostream& operator<<(std::ostream& os,
                         const __gnu_pbds::gp_hash_table<Key, Mapped, Hash_Fn, Eq_Fn, Comp_Probe_Fn, Probe_Fn,
                                                         Resize_Policy, Store_Hash, Allocator>& ght) {
    return basic_printer.print(os, ght);
}

#endif

template<class T>
std::ostream& log_pair(std::ostream& os, T&& p) {
    pretty_printer.print(os, p.first);
    os << " = ";
    return pretty_printer.print(os, p.second);
}

template<typename T>
std::ostream& log(std::ostream& os, bool print_sep, T&& first) {
    if (print_sep)
        os << ", ";
    return log_pair(os, first);
}

template<typename T, typename... Args>
std::ostream& log(std::ostream& os, bool print_sep, T&& first, Args&&... args) {
    if (print_sep)
        os << ", ";
    log_pair(os, first);
    return log(os, true, args...);
}

template<typename... Args>
std::ostream& log(std::ostream& os, const char* file, int line, Args&&... args) {
    log_prefix(os, file, line);
    return log(os, false, args...);
}

#ifndef NDEBUG
#define LOG1(a) log(std::cout, __FILE__, __LINE__, std::pair(#a, a)) << std::endl
#define LOG2(a, b) log(std::cout, __FILE__, __LINE__, std::pair(#a, a), std::pair(#b, b)) << std::endl
#define LOG3(a, b, c)                                                                                                  \
    log(std::cout, __FILE__, __LINE__, std::pair(#a, a), std::pair(#b, b), std::pair(#c, c)) << std::endl
#define LOG4(a, b, c, d)                                                                                               \
    log(std::cout, __FILE__, __LINE__, std::pair(#a, a), std::pair(#b, b), std::pair(#c, c), std::pair(#d, d))         \
        << std::endl
#else
#define LOG1(a)
#define LOG2(a, b)
#define LOG3(a, b, c)
#define LOG4(a, b, c, d)
#endif

#define GET_LOG_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define EXPAND(x) x
#define LOG(...) EXPAND(GET_LOG_MACRO(__VA_ARGS__, LOG4, LOG3, LOG2, LOG1)(__VA_ARGS__))
