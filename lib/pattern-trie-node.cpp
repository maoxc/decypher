#include <iostream>
#include <memory>
#include <utility>
#include "pattern-trie-node.hpp"

PatternTrieNode::PatternTrieNode() = default;

PatternTrieNode::PatternTrieNode(size_t key, size_t depth) : key(key), depth(depth) {}

void PatternTrieNode::insert(std::vector<size_t>::iterator current, std::vector<size_t>::iterator end, std::string _word) {
    if (current == end) {
        if (count == 0) {
            word = std::move(_word);
        } else if (count == 1) {
            word.clear();
        }
        count++;
        return;
    }
    if (!children[*current]) children[*current].reset(new PatternTrieNode(*current, depth+1));
    children[*current]->insert(current + 1, end, _word);
}

std::string PatternTrieNode::get_cross_check_result(std::vector<size_t>::iterator current, std::vector<size_t>::iterator end) {
    if (current == end) {
        return count == 1 ? word : "";
    }
    if (!children[*current]) return "";
    return children[*current]->get_cross_check_result(current + 1, end);
}

void PatternTrieNode::print() {
    std::cout << "key: " << key << ", depth: " << depth << ", word: " << word << " len: " << word.size() << std::endl;
    for (auto & it : children) it.second->print();
}