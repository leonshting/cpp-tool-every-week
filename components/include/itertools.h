#pragma once

#include <tuple>

namespace itertools {

template <class Iterator>
struct IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {
    }

    Iterator begin() const {
        return begin_;
    }
    Iterator end() const {
        return end_;
    }

private:
    Iterator begin_, end_;
};

template <typename T, typename TIter = decltype(std::declval<T>().begin())>
constexpr auto enumerate(T&& iterable) {
    struct Iterator {
        size_t i;
        TIter iter;
        bool operator!=(const Iterator& other) const {
            return iter != other.iter;
        }
        void operator++() {
            ++i;
            ++iter;
        }
        auto operator*() {
            return std::make_tuple(i, *iter);
        }
    };

    struct Wrapper {
        T iterable;
        Iterator begin() {
            return {0, iterable.begin()};
        }
        Iterator end() {
            return {0, iterable.end()};
        }
    };

    return Wrapper{std::forward<T>(iterable)};
}

namespace zip_internal {

template <typename T>
using select_iterator_for = decltype(std::declval<T>().begin());

template <typename T>
using get_value_type_for = std::remove_reference_t<decltype(*std::declval<T>().begin())>;

template <typename... Containers>
struct Iterator {
    using ValueType = std::tuple<get_value_type_for<Containers>...>;

    std::tuple<select_iterator_for<Containers>...> iterators;

    bool operator!=(const Iterator& other) const {
        auto out = std::apply(
            [&](auto&&... x) {
                return std::apply([&](auto&&... y) { return ((x != y) || ...); }, iterators);
            },
            other.iterators);

        return out;
    }

    void operator++() {
        std::apply([&](auto&&... x) { ((++x), ...); }, iterators);
    }

    ValueType operator*() {
        return std::apply([](auto&&... args) { return std::make_tuple(*args...); }, iterators);
    }
};

template <typename... T>
struct Wrapper {
    explicit Wrapper(T&&... contents) {
        auto content_tuple = std::tie(contents...);

        begin_ = std::apply(
            [](auto&&... args) { return Iterator<T...>{std::make_tuple(std::begin(args)...)}; },
            content_tuple);

        end_ = std::apply(
            [](auto&&... args) { return Iterator<T...>{std::make_tuple(std::end(args)...)}; },
            content_tuple);
    }

    auto begin() {
        return begin_;
    }

    auto end() {
        return end_;
    }

private:
    Iterator<T...> begin_, end_;
};

}  // namespace zip_internal

template <typename... Containers>
constexpr auto zip(Containers&&... containers) {
    return zip_internal::Wrapper<Containers...>(std::forward<Containers>(containers)...);
}

}  // namespace itertools
