#ifndef PATTERN_TRIE_NODE_HPP
#define PATTERN_TRIE_NODE_HPP

#include <unordered_map>
#include <vector>

class PatternTrieNode {
public:
    PatternTrieNode();
    std::string get_cross_check_result(std::vector<size_t>::iterator, std::vector<size_t>::iterator);
    void print();
    void insert(std::vector<size_t>::iterator, std::vector<size_t>::iterator, std::string);

private:
    PatternTrieNode(size_t, size_t);
    size_t key = 0;
    size_t count = 0;
    size_t depth = 0;
    std::string word = "";
    std::unordered_map<size_t, std::unique_ptr<PatternTrieNode>> children;
};
#endif //PATTERN_TRIE_NODE_HPP
