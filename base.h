//
// Created by jaroslavp on 5/16/2022.
//

#pragma once

#include <array>
#include <chrono>
#include <concepts>
#include <filesystem>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
    Fmt pair;
    Fmt list;
    Fmt dict;

    template<class T1, class T2>
    std::ostream& print_pair(std::ostream& os, const std::pair<T1, T2>& p) const {
        os << pair.open;
        os << print(p.first);
        os << pair.sep;
        os << print(p.second);
        return os << pair.close;
    }

    template<class InputIt>
    std::ostream& print_list(std::ostream& os, InputIt first, InputIt last) const {
        os << list.open;
        for (bool b = false; first != last; ++first) {
            if (b)
                os << list.sep;
            print(os, *first);
            b = true;
        }
        return os << list.close;
    }

    template<class InputIt>
    std::ostream& print_set(std::ostream& os, InputIt first, InputIt last) const {
        os << dict.open;
        for (bool b = false; first != last; ++first) {
            if (b)
                os << dict.sep;
            print(os, *first);
            b = true;
        }
        return os << dict.close;
    }

    template<class InputIt>
    std::ostream& print_dict(std::ostream& os, InputIt first, InputIt last) const {
        os << dict.open;
        for (bool b = false; first != last; ++first) {
            if (b)
                os << dict.sep;
            print(os, first->first);
            os << dict.key_val_sep;
            print(os, first->second);
            b = true;
        }
        return os << dict.close;
    }

    template<class T>
    std::ostream& print(std::ostream& os, T&& x) const {
        return os << x;
    }

    template<class T1, class T2>
    std::ostream& print(std::ostream& os, const std::pair<T1, T2>& p) const {
        return print_pair(os, p);
    }

    template<class T, std::size_t N>
    std::ostream& print(std::ostream& os, const std::array<T, N>& a) const {
        return print_list(os, a.begin(), a.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, const std::vector<T, Allocator>& v) const {
        return print_list(os, v.begin(), v.end());
    }

    template<class T, class Allocator>
    std::ostream& print(std::ostream& os, const std::deque<T, Allocator>& d) const {
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
        return print(std::cout, get_container(s));
    }

    template<class T, class Container>
    std::ostream& print(std::ostream& os, std::queue<T, Container>& q) const {
        return print(std::cout, get_container(q));
    }

    template<class T, class Container, class Compare>
    std::ostream& print(std::ostream& os, std::priority_queue<T, Container, Compare>& pq) const {
        return print(std::cout, get_container(pq));
    }
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

template<class T1, class T2>
std::ostream& log_pair(std::ostream& os, const std::pair<T1, T2>& p) {
    pretty_printer.print(os, p.first);
    os << " = ";
    return pretty_printer.print(os, p.second);
}

template<typename T>
std::ostream& log(std::ostream& os, bool b, T first) {
    if (b)
        os << ", ";
    return log_pair(os, first);
}

template<typename T, typename... Args>
std::ostream& log(std::ostream& os, bool b, T first, Args... args) {
    if (b)
        os << ", ";
    log_pair(os, first);
    return log(os, true, args...);
}

std::filesystem::path current_path = std::filesystem::current_path();

std::ostream& log_prefix(std::ostream& os, const char* file, int line) {
    std::filesystem::path file_path(file);
    return os << "[" << std::filesystem::relative(file_path, current_path).string() << ":" << line << "]: ";
}

template<typename... Args>
std::ostream& log(std::ostream& os, const char* file, int line, Args... args) {
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

using high_res_clock = std::chrono::high_resolution_clock;

struct Timer {
    const char* file;
    int line;
    const char* name;
    high_res_clock::time_point start = high_res_clock::now();

    template<class Period = std::ratio<1>>
    double get() const {
        return static_cast<std::chrono::duration<double, Period>>(high_res_clock::now() - start).count();
    }

    ~Timer() {
        log_prefix(std::cout, file, line);
        std::cout << name << " " << get<std::milli>() << " ms";
    }
};

#define TIMER(name)                                                                                                    \
    Timer name { __FILE__, __LINE__, #name }
#ifndef NDEBUG
#define DEBUG_TIMER(name) TIMER(name)
#else
#define DEBUG_TIMER(name)
#endif

namespace rng {

thread_local std::mt19937_64 mt(high_res_clock::now().time_since_epoch().count());

template<typename T>
requires std::integral<T>
T get(T a, T b) {
    std::uniform_int_distribution<T> dist(a, b);
    return dist(mt);
}

template<typename T>
requires std::integral<T>
T get() {
    std::uniform_int_distribution<T> dist(0, std::numeric_limits<T>::max());
    return dist(mt);
}

template<typename T>
requires std::floating_point<T>
T get(T a, T b) {
    std::uniform_real_distribution<T> dist(a, b);
    return dist(mt);
}

template<typename T>
requires std::floating_point<T>
T get() {
    std::uniform_real_distribution<T> dist(0, 1);
    return dist(mt);
}

} // namespace rng
