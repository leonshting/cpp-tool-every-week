#include <vector>

#include <gtest/gtest.h>

#include "itertools.h"

TEST(ItertoolsEnum, Simple) {
    std::vector<std::string> iter1 = {"a", "ab", "bb", "bz"};
    std::vector<std::string> iter2 = {iter1.rbegin(), iter1.rend()};

    for (const auto& [idx, it1] : itertools::enumerate(iter1)) {
        ASSERT_EQ(iter1[idx], it1);
    }

    for (const auto& [idx, it2] : itertools::enumerate(iter2)) {
        ASSERT_EQ(iter1[iter1.size() - idx - 1], it2);
    }
}

TEST(ItertoolsZip, Simple) {
    std::vector<std::string> iter1 = {"a", "ab", "bb", "bz"};
    std::vector<std::string> iter2 = iter1;

    auto zip1 = itertools::zip(iter1, iter2);
    auto zip2 = itertools::zip(iter1, iter2);

    for (auto [it1, it2] : itertools::enumerate(itertools::zip(zip1, zip2))) {
        ASSERT_EQ(std::get<0>(it2), std::get<1>(it2));
    }
}