#include <map>
#include <vector>
#include <string>

#include "traverse.h"
#include "itertools.h"

namespace aho_corasick {

struct AutomatonNode {
    AutomatonNode() : suffix_link(nullptr), terminal_link(nullptr) {
    }

    // Stores ids of strings which are ended at this node.
    std::vector<size_t> terminated_string_ids;
    // Stores tree structure of nodes.
    std::map<char, AutomatonNode> trie_transitions;

    std::map<char, AutomatonNode *> automaton_transitions_cache;
    AutomatonNode *suffix_link;
    AutomatonNode *terminal_link;
};

AutomatonNode *GetTrieTransition(AutomatonNode *node, char character);

AutomatonNode *GetAutomatonTransition(AutomatonNode *node, const AutomatonNode *root,
                                      char character);



class NodeReference {
public:
    NodeReference() : node_(nullptr), root_(nullptr) {
    }

    NodeReference(AutomatonNode *node, AutomatonNode *root) : node_(node), root_(root) {
    }

    NodeReference Next(char character) const;

    template <class Callback>
    void TraverseTerminal(Callback on_hit) const {
        for (auto string_id : TerminatedStringIds()) {
            on_hit(string_id);
        }

        AutomatonNode *current = node_;
        while (current->terminal_link != nullptr) {
            current = current->terminal_link;
            for (auto string_id : current->terminated_string_ids) {
                on_hit(string_id);
            }
        }
    }

    bool IsTerminal() const {
        return !node_->terminated_string_ids.empty();
    }

    explicit operator bool() const {
        return node_ != nullptr;
    }

    bool operator==(NodeReference other) const {
        return node_ == other.node_;
    }

private:
    using IDsRange = itertools::IteratorRange<std::vector<size_t>::const_iterator>;

    NodeReference TerminalLink() const {
        return {node_->terminal_link, root_};
    }

    IDsRange TerminatedStringIds() const {
        return {node_->terminated_string_ids.cbegin(), node_->terminated_string_ids.cend()};
    }

    AutomatonNode *node_;
    AutomatonNode *root_;
};

class AutomatonBuilder;

class Automaton {
public:
    Automaton() = default;

    Automaton(const Automaton &) = delete;
    Automaton &operator=(const Automaton &) = delete;

    NodeReference Root() {
        return {&root_, &root_};
    }

private:
    AutomatonNode root_;

    friend class AutomatonBuilder;
};

class AutomatonBuilder {
public:
    void Add(const std::string &string, size_t id);

    std::unique_ptr<Automaton> Build();

private:
    static void BuildTrie(const std::vector<std::string> &words, const std::vector<size_t> &ids,
                          Automaton *automaton);

    static void AddString(AutomatonNode *root, size_t string_id, const std::string &string);

    static void BuildSuffixLinks(Automaton *automaton);

    static void BuildTerminalLinks(Automaton *automaton);

    std::vector<std::string> words_;
    std::vector<size_t> ids_;
};

}  // namespace aho_corasick
