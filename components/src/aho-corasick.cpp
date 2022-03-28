#include "aho-corasick.h"

namespace aho_corasick {

AutomatonNode *GetTrieTransition(AutomatonNode *node, char character) {
    auto map_it = node->trie_transitions.find(character);
    if (map_it != node->trie_transitions.end()) {
        return &map_it->second;
    }
    return nullptr;
}

AutomatonNode *GetAutomatonTransition(AutomatonNode *node, const AutomatonNode *root,
                                      char character) {
    auto found = node->automaton_transitions_cache.find(character);
    if (found != node->automaton_transitions_cache.end()) {
        return found->second;
    }
    AutomatonNode *trie_trans = GetTrieTransition(node, character);
    if (trie_trans != nullptr) {
        node->automaton_transitions_cache[character] = trie_trans;
        return trie_trans;
    }
    auto *current = node;
    while (current != root) {
        current = current->suffix_link;
        auto *child = GetTrieTransition(current, character);
        if (child != nullptr) {
            node->automaton_transitions_cache[character] = child;
            return child;
        }
    }
    node->automaton_transitions_cache[character] = nullptr;
    return nullptr;
}

namespace internal {

class AutomatonGraph {
public:
    AutomatonGraph() = default;

    struct Edge {
        Edge(AutomatonNode *source, AutomatonNode *target, char character)
            : source(source), target(target), character(character) {
        }

        AutomatonNode *source;
        AutomatonNode *target;
        char character;
    };
};

std::vector<typename AutomatonGraph::Edge> OutgoingEdges(const AutomatonGraph &,
                                                         AutomatonNode *vertex) {
    std::vector<typename AutomatonGraph::Edge> edges;
    for (auto &[character, node] : vertex->trie_transitions) {
        edges.emplace_back(vertex, &node, character);
    }
    return edges;
}

AutomatonNode *GetTarget(const AutomatonGraph &graph, const AutomatonGraph::Edge &edge) {
    return edge.target;
}

class SuffixLinkCalculator : public traverses::BfsVisitor<AutomatonNode *, AutomatonGraph::Edge> {
public:
    explicit SuffixLinkCalculator(AutomatonNode *root) : root_(root) {
    }

    void ExamineVertex(AutomatonNode *node) override {
        if (node == root_) {
            node->suffix_link = root_;
        }
    }

    void ExamineEdge(const AutomatonGraph::Edge &edge) override {
        if (edge.source == root_) {
            edge.target->suffix_link = root_;
            return;
        }
        AutomatonNode *current_source = edge.source;
        while (GetTrieTransition(current_source->suffix_link, edge.character) == nullptr) {
            if (current_source == root_) {
                edge.target->suffix_link = root_;
                return;
            }
            current_source = current_source->suffix_link;
        }
        edge.target->suffix_link = GetTrieTransition(current_source->suffix_link, edge.character);
    }

private:
    AutomatonNode *root_;
};

class TerminalLinkCalculator : public traverses::BfsVisitor<AutomatonNode *, AutomatonGraph::Edge> {
public:
    explicit TerminalLinkCalculator(AutomatonNode *root) : root_(root) {
    }

    void DiscoverVertex(AutomatonNode *node) override {
        auto *current_suffix = node->suffix_link;

        if (node == root_) {
            node->terminal_link = nullptr;
            return;
        }

        if (current_suffix->terminated_string_ids.empty()) {
            node->terminal_link = current_suffix->terminal_link;
        } else {
            node->terminal_link = current_suffix;
        }
    }

private:
    AutomatonNode *root_;
};

}  // namespace internal

NodeReference NodeReference::Next(char character) const {
    AutomatonNode *transition = GetAutomatonTransition(node_, root_, character);
    if (transition == nullptr) {
        return {root_, root_};
    }
    return {transition, root_};
}

void AutomatonBuilder::Add(const std::string &string, size_t id) {
    words_.push_back(string);
    ids_.push_back(id);
}

std::unique_ptr<Automaton> AutomatonBuilder::Build() {
    auto automaton = std::make_unique<Automaton>();
    BuildTrie(words_, ids_, automaton.get());
    BuildSuffixLinks(automaton.get());
    BuildTerminalLinks(automaton.get());
    return automaton;
}

void AutomatonBuilder::BuildTrie(const std::vector<std::string> &words,
                                 const std::vector<size_t> &ids, Automaton *automaton) {
    for (size_t i = 0; i < words.size(); ++i) {
        AddString(&automaton->root_, ids[i], words[i]);
    }
}

void AutomatonBuilder::AddString(AutomatonNode *root, size_t string_id, const std::string &string) {
    auto *current_node = root;
    for (const auto character : string) {
        auto *child = GetTrieTransition(current_node, character);
        if (child == nullptr) {
            current_node->trie_transitions[character] = AutomatonNode();
            current_node = GetTrieTransition(current_node, character);
        } else {
            current_node = child;
        }
    }
    current_node->terminated_string_ids.push_back(string_id);
}

void AutomatonBuilder::BuildSuffixLinks(Automaton *automaton) {
    internal::AutomatonGraph graph;

    traverses::BreadthFirstSearch(
        &automaton->root_, graph,
        aho_corasick::internal::SuffixLinkCalculator(&automaton->root_));
}

void AutomatonBuilder::BuildTerminalLinks(Automaton *automaton) {
    internal::AutomatonGraph graph;

    traverses::BreadthFirstSearch(
        &automaton->root_, graph,
        aho_corasick::internal::TerminalLinkCalculator(&automaton->root_));
}

}  // namespace aho_corasick
