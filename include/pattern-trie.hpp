#ifndef PATTERN_TRIE_HPP
#define PATTERN_TRIE_HPP

#include "pattern-trie-node.hpp"

class PatternTrie {
public:
    PatternTrie();
    void insert(std::vector<size_t>, std::string);
    std::string get_cross_check_result(std::vector<size_t>);
    void print();
private:
    std::unique_ptr<PatternTrieNode> root;
};

#endif //PATTERN_TRIE_HPP
