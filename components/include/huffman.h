#pragma once

#include <vector>
#include <memory>
#include <optional>

namespace huffman {

template <class ValueType>
struct HuffmanNode {

    HuffmanNode<ValueType> *parent = nullptr, *left = nullptr, *right = nullptr;
    std::optional<ValueType> value = std::nullopt;
    size_t level = 0;

    HuffmanNode *AddNode(ValueType new_value, size_t new_level) {
        HuffmanNode *current = this;

        while (new_level != current->level) {
            if (current == nullptr) {
                throw std::runtime_error("huffman tree build failed");
            }

            if (current->left == nullptr) {
                current->left = new HuffmanNode<ValueType>;
                current->left->parent = current;
                current->left->level = current->level + 1;
                current = current->left;
            } else if (current->right == nullptr) {
                current->right = new HuffmanNode<ValueType>;
                current->right->parent = current;
                current->right->level = current->level + 1;
                current = current->right;
            } else {
                current = current->parent;
            }
        }

        current->value = new_value;
        return current;
    }

    bool IsTerminal() {
        return this->left == nullptr && this->right == nullptr;
    }

    ~HuffmanNode() {
        if (left != nullptr) {
            delete left;
            left = nullptr;
        }
        if (right != nullptr) {
            delete right;
            right = nullptr;
        }
    }
};

template <class ShapeType = std::uint8_t, class ValueType = std::uint8_t>
struct HuffmanTree {

    static HuffmanTree FromSequence(std::vector<ShapeType> per_level,
                                    std::vector<ValueType> values) {
        if (values.empty()) {
            return HuffmanTree();
        }

        HuffmanTree tree;
        tree.root = std::make_shared<HuffmanNode<ValueType>>();

        HuffmanNode<ValueType> *current = tree.root.get();

        auto value_it = values.begin();
        for (size_t level_idx = 0; level_idx < per_level.size(); ++level_idx) {
            size_t level_value = level_idx + 1;
            ShapeType card = per_level[level_idx];

            for (size_t inner = 0; inner < card; ++inner) {
                auto inserted = current->AddNode(*value_it, level_value);
                current = inserted->parent;
                ++value_it;
            }
        }

        return tree;
    }

    std::shared_ptr<HuffmanNode<ValueType>> root = nullptr;
};

}  // namespace huffman