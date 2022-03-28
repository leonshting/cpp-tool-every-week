#include <vector>

#include <gtest/gtest.h>

#include "itertools.h"
#include "aho-corasick.h"

TEST(Automaton, Build) {
    std::vector<std::string> for_trie = {"a", "ab", "bb", "bz"};

    auto builder = aho_corasick::AutomatonBuilder();
    for (const auto& [idx, word] : itertools::enumerate(for_trie)) {
        builder.Add(word, idx);
    }

    auto trie = builder.Build();

    std::vector<char> transitions = {'a', 'b', 'z'};
    std::vector<std::vector<size_t>> hit_ids = {{0}, {1}, {3}};

    aho_corasick::NodeReference ref = trie->Root();
    for (const auto [hits, transition] : itertools::zip(hit_ids, transitions)) {
        ref = ref.Next(transition);

        std::vector<size_t> actual_hits;
        ref.TraverseTerminal([&actual_hits](size_t match_id) { actual_hits.push_back(match_id); });

        ASSERT_EQ(hits.size(), actual_hits.size());

        for (const auto [exp, actual] : itertools::zip(hits, actual_hits)) {
            ASSERT_EQ(exp, actual);
        }
    }
}
