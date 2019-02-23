#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <utility>
#include <vector>
#include <math.h>
#include <memory>
#include <unordered_map>
#include <queue>
#include <random>
#include <chrono>
#include <float.h>
using namespace std;

bool isLower(const char &c) { return 'a' <= c && c <= 'z';}
bool isUpper(const char &c) { return 'A' <= c && c <= 'Z';}
char toLower(const char &c) { return c - 'A' + 'a';}
char toUpper(const char &c) { return c - 'a' + 'A';}


/*
    Returns all n(n-1)/2 possible transpositions of a string
*/ 
vector<string> transpositions(string s) {
    vector<string> vec;
    for (size_t i = 0; i < s.size(); i++) {
        for (size_t j = i + 1; j < s.size(); j++) {
            swap(s[i], s[j]);
            vec.push_back(s);
            swap(s[i], s[j]);
        }
    }
    return vec;
}

/* 
    Create the pattern of a word e.g.:
    pattern("hello") = "1.2.3.3.4"
*/
string pattern(const string & word) {
    if (word.size() == 0) return "";
    map<char, size_t> m;
    size_t unique_chars = 1;
    string pattern;
    pattern.reserve(3 * word.size());
    for (size_t i = 0; i < word.size() - 1; i++) {
        if (!m[word[i]]) m[word[i]] = unique_chars++;
        pattern += to_string(m[word[i]]);
        pattern.push_back('.');
    }
    if (!m[word[word.size()-1]]) m[word[word.size()-1]] = unique_chars++;
    pattern += to_string(m[word[word.size()-1]]);
    return pattern;
}

/*
    Takes as input a textual dictionary and maps word
    patterns to the list of corresponding words

    --dict.txt--   ->  -------patterns.txt-------
    | cat      |       | 1.2.3:cat,dog;         |
    | dog      |       | 1.2.1.1.3:mommy,puppy; |
    | mommy    |       --------------------------
    | puppy    |
    ------------

    Writes into a file with name given by second argument
    Returns false if some file operation failed
*/
bool patternify(const string& in, const string& out) {
    ifstream fin(in);
    ofstream fout(out);
    if (!fin || !fout) return false;
    map<string, vector<string>>  patterns;
    for (string line; getline(fin, line);
        patterns[pattern(line)].push_back(line)) {}
    for (auto p : patterns) {
        fout << p.first << ":";
        for (size_t i = 0; i < p.second.size() - 1; i++)
            fout << p.second[i] << ",";
        fout << p.second[p.second.size()-1] << ";" << endl;
    }
    fin.close();
    fout.close();
    return true;
}

struct LogProbTable {
    size_t n, sum;
    map<string, double> freq_table;
    double missing;

    explicit LogProbTable(size_t n) : n(n) { init();}

    void init() {
        sum = 0;
        freq_table.clear();
        missing = 0;
    }

    bool loadTableFile(const string & in) {
        init();
        ifstream fin(in);
        if (!fin) {
            cerr << "ERROR file " + in + " not found" << endl;
            return false;
        } else {
            unordered_map<string, size_t> tally;
            string ngram;
            while (getline(fin, ngram, ' ')) {
                if (ngram.size() > n) n = ngram.size();
                string count;
                getline(fin, count);
                stringstream ss(count);
                ss >> tally[ngram];
                sum += tally[ngram];
            }
            double dsum = static_cast<double>(sum);
            for (auto p : tally) freq_table[p.first] = log10(p.second/dsum);
            missing = log10(0.01/dsum);
        }
        return true;
    }
};


/*
    Each letter in a given word is substituted according to a key
*/

// Returned string here is case-sensitive
// For decrypting
string apply_key_preserving(const string & word, const string & key) {
    string s;
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
string apply_key(const string & word, const string & key) {
    string s;
    s.reserve(word.size());
    for (char c : word) {
        if (isLower(c)) s.push_back(key[c-'a']);
        else if (isUpper(c)) s.push_back(key[c-'A']);
        else s.push_back(c);
    }
    return s;
}

vector<string> apply_key(const vector<string> & words, const string & key) {
    vector<string> new_words;
    for (auto word : words) new_words.push_back(apply_key(word, key));
    return new_words;
}

/*
    Generate a random permutation of the english alphabet
*/
string generate_key() {
    string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::mt19937_64 rng;

    // initialize the random number generator with time-dependent seed
    uint64_t timeSeed = std::chrono::high_resolution_clock::now()
                                        .time_since_epoch().count();
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss);

    // initialize a uniform distribution between 0 and 1
    std::uniform_real_distribution<double> unif(0, 1);

    for (size_t i = 0; i < alphabet.size()-1; i++) {
        if (unif(rng) >= 0.5) {
            char tmp = alphabet[i];
            alphabet[i] = alphabet[i+1];
            alphabet[i+1] = tmp;
        }
    }
    return alphabet;
}

/*
    Encrypts file with a randomly generated key
*/
bool random_encrypt(const string & in) {
    ifstream fin(in);
    ofstream fout("_" + in);
    if (!fin) {
        cerr << "ERROR file " + in + " not found" << endl;
        return false;
    } else {
        string line, key = generate_key();
        while (getline(fin, line)) {
            fout << apply_key(line, key) << endl;
        }
    }
    return true;
}

/*
    Decrypts file with a given key
*/
bool decrypt(const string & in, const string & key) {
    ifstream fin(in);
    size_t dot_index = in.find('.');
    string out = in.substr(0, dot_index) + "-decrypted" +
    in.substr(dot_index, in.size() - dot_index);
    ofstream fout(out);
    if (!fin) {
        cerr << "ERROR file " + in + " not found" << endl;
        return false;
    } else {
        string line;
        while (getline(fin, line)) {
            fout << apply_key_preserving(line, key) << endl;
        }
    }
    return true;
}
/*
    Fetches all ngrams from a file
*/
vector<string> vectorize_ngrams(const string & in, const size_t & n) {
    ifstream fin(in);
    vector<string> strings;
    if (!fin) {
        cerr << "ERROR file " + in + " not found" << endl;
        return vector<string>();
    } else {
        string line;
        queue<char> q;
        while (getline(fin, line)) {
            for (char c : line) {
                if (isUpper(c)) q.push(c);
                else if (isLower(c)) q.push(toUpper(c));
                else if (c == ' ') while (q.size()) q.pop();
                if (q.size() == n) {
                    string ngram;
                    ngram.resize(n);
                    for (size_t i = 0; i < n; i++) {
                        ngram[i] = *(i+&q.front());
                    }
                    strings.push_back(ngram);
                    q.pop();
                }
            }
        }
    }
    return strings;
}

double score(const LogProbTable & table, const vector<string> & words) {
    double score = 0;
    for (auto word : words) {
        auto it = table.freq_table.find(word);
        if (it != table.freq_table.end()) score += it->second;
        else score += table.missing;
    }
    return score;
}

string solve(const LogProbTable & table, const vector<string> & ngrams) {
    double max = score(table, ngrams);
    string key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    double tmp = max;
    while (true) {
        for (auto t : transpositions(key)) {
            double fitness = score(table, apply_key(ngrams, t));
            if (fitness > tmp) {
                tmp = fitness;
                key = t;
            }
        }
        if (tmp == max) break;
        max = tmp;
    }
    return key;
}

int main(int argc, char ** argv) {
    vector<string> args(argv + 1, argv + argc);
    vector<string> ngrams;
    LogProbTable table(4);
    string in;
    if (args.size() == 1) {
        in = args[0];
        ngrams = vectorize_ngrams(in, 4);
        table.loadTableFile("sample-quadgrams.txt");
    }
    if (ngrams.size()) {
        string sol = solve(table, ngrams);
        decrypt(in, sol);
    }

    return 0;
}