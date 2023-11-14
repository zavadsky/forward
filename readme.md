The code implements word-based forward compression with low frequency word replacement and frequency table compression.

Compile:
g++ *.cpp -o forward

Run:
forward <file.txt>

Outputs:
file.txt.enc - word-based forward frequencies with low frequency word replacement, compressed using the arithmetic encoder;
file.txt.frq - compressed frequency table;
dic_file.txt - the dictionary (list of unique words from the source text file), ordered by descending frequencies; groups of low-frequency words are second-level ordered by the first occurrence in the text.
