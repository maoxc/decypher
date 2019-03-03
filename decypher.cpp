#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <random>
#include <chrono>
#include <getopt.h>
using vs = std::vector<std::string>;

constexpr auto eng = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool isLower(const char &c) { return 'a' <= c && c <= 'z';}
bool isUpper(const char &c) { return 'A' <= c && c <= 'Z';}
char toLower(const char &c) { return c - 'A' + 'a';}
char toUpper(const char &c) { return c - 'a' + 'A';}

void die(std::string && msg) {
    std::cout << msg << std::endl;
    exit(0);
}

void fail_input() {
    std::cout << "decypher --encrypt file..." << std::endl
              << "decypher [-t tally] [-d dictionary] [--silent] file..."
              << std::endl;
    exit(0);
}

/*
    Add a suffix to a filename
    file.txt -> file-with-suffix.txt
*/
std::string add_filename_suffix(const std::string & fname, std::string && suffix) {
    size_t dot_index = fname.find('.');
    if (dot_index == std::string::npos) return fname + suffix;
    return fname.substr(0, dot_index) + suffix +
        fname.substr(dot_index, fname.size() - dot_index);
}

/* 
    Create the pattern of a word e.g.:
    pattern("hello") = "1.2.3.3.4"
*/
std::string pattern(const std::string & word) {
    if (word.size() == 0) return "";
    std::unordered_map<char, size_t> m;
    size_t unique_chars = 1;
    std::string pattern;
    pattern.reserve(3 * word.size());
    for (size_t i = 0; i < word.size() - 1; i++) {
        if (!m[word[i]]) m[word[i]] = unique_chars++;
        pattern += std::to_string(m[word[i]]);
        pattern.push_back('.');
    }
    if (!m[word[word.size()-1]]) m[word[word.size()-1]] = unique_chars++;
    pattern += std::to_string(m[word[word.size()-1]]);
    return pattern;
}

/*
    Takes as input a dictionary file and maps word
    patterns to the list of its corresponding words

    --dict.txt--
    | cat      |      1.2.3:cat,dog;
    | dog      |  ->  1.2.1.1.3:mummy,puppy;
    | mummy    |      ...
    | puppy    |
    | ...      |
    ------------

*/
std::unordered_map<std::string, vs> patternify(const std::string& in) {
    std::ifstream fin(in);
    if (!fin) return std::unordered_map<std::string, vs>();
    std::unordered_map<std::string, vs> patterns;
    for (std::string line; getline(fin, line);
        patterns[pattern(line)].push_back(line)) {}
    return patterns;
}

/*
    A struct for storing scores of ngrams
    Score is obtained by taking logarithm of an ngram's frequency
    With logs we have the nice property:
    score("hello") = score("hell") + score("ello");
*/
struct LogProbTable {
    size_t n, sum;
    std::unordered_map<std::string, double> score_table;
    double missing_penalty;

    explicit LogProbTable(size_t n) : n(n) { init();}

    void init() {
        sum = 0;
        score_table.clear();
        missing_penalty = 0;
    }

    bool loadTableFile(const std::string & in) {
        init();
        std::ifstream fin(in);
        if (!fin) return false;
        else {
            std::unordered_map<std::string, size_t> tally;
            std::string ngram;
            while (getline(fin, ngram, ' ')) {
                std::string count;
                getline(fin, count);
                std::stringstream ss(count);
                ss >> tally[ngram];
                sum += tally[ngram];
            }
            double dsum = static_cast<double>(sum);
            for (auto p : tally) score_table[p.first] = log10(p.second/dsum);
            missing_penalty = log10(0.01/dsum);
        }
        return true;
    }
};


/*
    Each letter in a given word is substituted according to a key
*/

// Returned string here is case-sensitive
// For decrypting
std::string apply_key_preserving(const std::string & word, const std::string & key) {
    std::string s;
    s.reserve(word.size());
    for (char c : word) {
        if (isLower(c)) s.push_back(toLower(key[c-'a']));
        else if (isUpper(c)) s.push_back(key[c-'A']);
        else s.push_back(c);
    }
    return s;
}

// Return string here is UPPERCASE
// For working with ngrams (that are uppercase) 
std::string apply_key(const std::string & word, const std::string & key) {
    std::string s;
    s.reserve(word.size());
    for (char c : word) {
        if (isLower(c)) s.push_back(key[c-'a']);
        else if (isUpper(c)) s.push_back(key[c-'A']);
        else s.push_back(c);
    }
    return s;
}
// Previous function mapped to a vector of strings
vs apply_key(const vs & words, const std::string & key) {
    vs new_words;
    for (auto word : words) new_words.push_back(apply_key(word, key));
    return new_words;
}

/*
    Generate a random permutation of the english alphabet
*/
std::string generate_key() {
    std::string key = eng;
    std::mt19937_64 rng;

    // initialize the random number generator with time-dependent seed
    uint64_t timeSeed = std::chrono::high_resolution_clock::now()
                                        .time_since_epoch().count();
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss);

    // initialize a uniform distribution between 0 and 1
    std::uniform_real_distribution<double> unif(0, 1);

    for (size_t i = 0; i < key.size(); i++) {
        for (size_t j = 0; j < key.size(); j++) {
            if (unif(rng) >= 0.5) {
                char tmp = key[i];
                key[i] = key[j];
                key[j] = tmp;
            }
        }
    }
    return key;
}

/*
    Encrypt file with a randomly generated key
*/
bool random_encrypt(const std::string & in, bool silent) {
    std::ifstream fin(in);
    if (!fin) return false;
    else {
        std::ofstream fout("_" + in);
        std::string line, key = generate_key();
        if (!silent) std::cout << in << ": " << key << std::endl;
        while (getline(fin, line)) {
            fout << apply_key_preserving(line, key) << std::endl;
        }
    }
    return true;
}

/*
    Decrypts file with a given key
*/
bool decrypt_file(const std::string & in, const std::string & key) {
    std::ifstream fin(in);
    if (!fin) return false;
    else {
        std::ofstream fout(add_filename_suffix(in, "-decrypted"));
        std::string line;
        while (getline(fin, line)) {
            fout << apply_key_preserving(line, key) << std::endl;
        }
    }
    return true;
}

/*
    Fetch all words from a file
*/
vs get_words(const std::string & in) {
    std::ifstream fin(in);
    vs words;
    if (!fin) return vs();
    else {
        words.emplace_back();
        std::string line;
        while (getline(fin, line)) {
            if (words[words.size()-1].size()) words.emplace_back();
            for (char c : line) {
                if (isUpper(c)) words[words.size()-1].push_back(c);
                else if (isLower(c))
                    words[words.size()-1].push_back(toUpper(c));
                else if (c == ' ' && words[words.size()-1].size())
                    words.emplace_back();
            }
        }
        if (!words[words.size()-1].size()) words.resize(words.size()-1);
    }
    return words;
}

/*
    Fetch all ngrams from a list of words
*/
vs get_ngrams(const vs & words, size_t n) {
    vs ngrams;
    for (std::string word : words)
        for (size_t i = 0; i + n - 1 < word.size(); i++)
            ngrams.emplace_back(word, i, n);
    return ngrams;
}

/*
    Generates next swap
*/
bool next_transposition(std::string & s, size_t & i, size_t & j) {
    // undo previous swap
    if (j != 0) std::swap(s[i], s[j]);
    if (j == s.size() - 1) {
        // move i
        i++;
        j = i + 1;
    } else {
        // or move j
        j++;
    }
    if (j == s.size()) return false;
    // next swap
    std::swap(s[i], s[j]);
    return true;
}

/*
    Assign a score to a list of ngrams
    Ngrams missing from the table incur a penalty
*/
double score(const LogProbTable & table, const vs & ngrams) {
    double score = 0;
    for (auto ngram : ngrams) {
        auto it = table.score_table.find(ngram);
        if (it != table.score_table.end()) score += it->second;
        else score += table.missing_penalty;
    }
    return score;
}

/*
    Compare scores of all possible letter swaps, stick with
    the best one and repeat exhaustively.
*/
std::string solve(const LogProbTable & table, const vs & ngrams) {
    double max = score(table, ngrams);
    std::string key = eng;
    double tmp = max;
    while (true) {
        std::string tmp_transp = eng;
        size_t i = 0, j = 0;
        size_t count = 0;
        while (next_transposition(tmp_transp, i, j)) {
            double fitness = score(table, apply_key(ngrams, tmp_transp));
            
            if (fitness > tmp) {
                tmp = fitness;
                key = tmp_transp;
            }
        }
        std::cout << tmp << std::endl;
        std::cout << key << std::endl;
        if (tmp == max) break;
        max = tmp;
    }
    return key;
}

/*
    Check cipherwords against a dictionary of patterns
    If there is only one word in the dictionary with a certain pattern, 
    then we may assume to have solved the cipherword.
*/
std::string cross_check(vs words, const std::unordered_map<std::string, vs> & patterns) {
    std::string key = eng;
    for (size_t j = 0; j < words.size(); j++) {
        std::string word = words[j];
        auto it = patterns.find(pattern(word));
        if (it != patterns.end() && it->second.size() == 1) {
            std::string match = it->second[0];
            if (word != match) {
                std::string tmp_key = eng;
                for (size_t i = 0; i < word.size(); i++)
                    std::swap(tmp_key[word[i]-'A'], tmp_key[match[i]-'A']);
                key = apply_key(key, tmp_key);
                words = apply_key(words, tmp_key);
            }
        }
    }
    return key;
}

int main(int argc, char ** argv) {
    bool silent, encrypt, decrypt;
    std::string tally = "default-tally.txt";
    std::string dict = "default-dictionary.txt";

    static struct option long_options[] = {
        {"help",        no_argument,       NULL, 'h'},
        {"silent",      no_argument,       NULL, 's'},
        {"encrypt",     no_argument,       NULL, 'e'},
        {"tally",       required_argument, NULL, 't'},
        {"dictionary",  required_argument, NULL, 'd'},
        {NULL,          0,                 NULL,   0}
    };

    char opt;
    while ((opt = getopt_long(argc, argv, "hset:d:", long_options, NULL)) != -1) {
        switch (opt) {
            case '?':
            case ':':
            case 'h':
                fail_input();
                break;
            case 's':
                silent = true;
                break;
            case 'e':
                encrypt = true;
                break;
            case 't':
                decrypt = true;
                tally = optarg;
                break;
            case 'd':
                decrypt = true;
                dict = optarg;
                break;
            default:
                fail_input();
                break;
        }
    }
    if (encrypt && decrypt) fail_input();
    vs files(argv + optind, argv + argc);
    if (!files.size()) fail_input();

    /*
        encrypt
    */
    if (encrypt) {
        if (!silent) std::cout << "encrypting..." << std::endl;
        for (std::string file : files) {
            if (!random_encrypt(file, silent) && !silent)
                std::cout << "failed to read " << file << std::endl;
        }
        exit(0);
    }

    /*
        or decrypt
    */
    LogProbTable table(4);
    if (!table.loadTableFile(tally)) die("failed to read " + tally);
    if (table.sum < 10000) die("please input larger tally");

    std::unordered_map<std::string, vs> patterns = patternify(dict);
    if (!patterns.size()) die("failed to read " + dict);

    if (!silent) std::cout << "decrypting..." << std::endl;
    for (std::string file : files) {
        std::stringstream ss;
        vs words = get_words(file);
        if (!words.size()) {
            std::cout << "failed to read " << file << std::endl;
            continue;
        }
        std::string key1 = cross_check(words, patterns);
        vs improved_words = apply_key(words, key1);
        if (key1 == eng) {
            ss << "dictionary found no pattern matches" << std::endl;
        }

        vs ngrams = get_ngrams(improved_words, 4);
        std::string key2 = solve(table, ngrams);
        if (key2 == eng) {
            ss << "hill climbing algorithm did nothing (or was not even needed)"
               << std::endl;
        }

        std::string final_key = apply_key(key1, key2);
        ss << file << ": " << final_key << std::endl;
        decrypt_file(file, final_key);
        if (!silent) std::cout << ss.str();
    }
    return 0;
}