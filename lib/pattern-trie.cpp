#include <string>
#include <memory>
#include <utility>
#include "pattern-trie.hpp"
#include "pattern-trie-node.hpp"

PatternTrie::PatternTrie() {
    root = std::make_unique<PatternTrieNode>();
}
void PatternTrie::insert(std::vector<size_t> vs, std::string word) {
    root->insert(vs.begin(), vs.end(), std::move(word));
}
std::string PatternTrie::get_cross_check_result(std::vector<size_t> vs) {
    return root->get_cross_check_result(vs.begin(), vs.end());
}
void PatternTrie::print() {
    root->print();
}
