The code implements word-based forward compression. 

**Compile:**
g++ *.cpp -o forward

**Run:**

forward <file.txt> [option1] [option2]

**[option1] = -r | -f**

-r  -  Word-based forward compression + low frequency word replacement and frequency table compression.

-f  -  Word-based forward compression.

**Outputs:**

-r

file.txt.enc - word-based forward with low frequency word replacement, compressed using the arithmetic encoder;

file.txt.frq - compressed frequency table;

dic_file.txt - the dictionary (list of unique words from the source text file), ordered by descending frequencies; groups of low-frequency words are second-level ordered by the first occurrence in the text.

-f

file.txt.enc - word-based forward, compressed using the arithmetic encoder;

file.txt.frq - list of frequencies of lexicographically sorted words;

dic_file.txt - the lexicographically sorted dictionary (list of unique words from the source text file).

**[option2] = -e**

If option2 exists, instead of arithmetic encoding, the entropy will be calculated and displayed. file.txt.enc won't be created. With this option, large files are processed times faster.
