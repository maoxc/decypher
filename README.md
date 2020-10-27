# decypher

### A substitution cipher
In cryptography, a substitution cipher is a method of encrypting by which units of plaintext are replaced with ciphertext, according to a fixed system; the "units" may be single letters (the most common), pairs of letters, triplets of letters, mixtures of the above, and so forth. The receiver deciphers the text by performing the inverse substitution. 

### A word pattern
Two identical characters in a ciphertext will represent the same character in the plaintext. Enter word patterns:  
`pattern("hello") = "1.2.3.3.4"`  

A dictionary of plaintext words is mapped into lists of words by their pattern.  
`"1.2.1.1.3": {"mummy", "puppy"}`  

Given a cipherword whose pattern corresponds to only one word in the dictionary, we solve a part of the cipher.  
`"orerer" -> "1.2.3.2.3.2" : {"banana"} -> (o->b, r->a, e->n)`  

### Quadgrams
We use quadgrams to measure how similar a word is to English. The fitness of a ciphertext is measured by decomposing each word into quadgrams and adding up their scores.  
`score(quadgram) = log10(frequency(quadgram))`  

A hill-climbing algorithm is used to improve the fitness exhaustively: in each step a we find the pair of letters whose substitution results in the most fitness gained.  

## Usage:
`1)`  
Use a randomly generated key to encrypt file(s):  
`decypher -e|--encrypt [-s|--silent] file ...`  
&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;outputted file(s) are prefixed by `_`  
&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;key is printed to standard output unless silent flag is supplied

`2)`  
Decrypt file(s) using optional tally/dictionary files and silent flag:  
`decypher [-t tally] [-d dictionary] [-s|--silent] file ...`  
&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;outputted file(s) are suffixed by `-decrypted`  
&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;files `default-tally.txt` and `default-dictionary.txt` are used implicitly unless otherwise specified.  
