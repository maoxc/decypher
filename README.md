# decypher

### A substitution cipher
In cryptography, a substitution cipher is a method of encrypting by which units of plaintext are replaced with ciphertext, according to a fixed system; the "units" may be single letters (the most common), pairs of letters, triplets of letters, mixtures of the above, and so forth. The receiver deciphers the text by performing the inverse substitution. 

### A word pattern
Two identical characters in a ciphertext will represent the same character in the plaintext. Enter word patterns:  
pattern("hello") = "1.2.3.3.4"  
pattern("banana") = "1.2.3.2.3.2"  
..


## Usage:

[//]: # (`./decypher -p dictionary.txt` - Creates the file dictionary-patterns.txt containing patterns of each word in dictionary.txt)  


[//]: # (`./decypher -q sample.txt` - Creates the file sample-quadgrams.txt containing a tally of quadgrams occuring in sample.txt)  


`./decypher ciphertext.txt` - Decrypts the file ciphertext.txt outputting to ciphertext-decrypted.txt
