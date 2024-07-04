The code implements context-aware word-based forward compression, calculating the size of the compressed text and outputting the dictionaries.

Compile: g++ *.cpp -o forward

Run:

forward <text file name> <th_f alphanumeric> <th_g alphanumeric> <th_f punctuation> <th_g punctuation> [poetry|bks]

Alphanumeric and non-alphanumeric (punctuation) word sequences are compressed separately.
<th_f alphanumeric> - the threshold th_f value for the alphanumeric stream

<th_g alphanumeric> - the threshold th_g value for the non-alphanumeric stream 

<th_f punctuation> - the threshold th_f value for the punctuation stream 

<th_g punctuation> - the threshold th_g value for the punctuation stream 

[poetry] - treats all punctuation signs in one poetry line as a single word
[bks] - single spaces are eliminated from the punctuation stream, and words followed by non-single-space characters are marked with some special `backspace' symbol that never occurs in a text

Outputs:

a_<file name>.frq - compressed frequency table for the alphanumeric stream.

n_<file name>.frq - compressed frequency table for the non-alphanumeric stream.

adic_<file name> - the dictionary for the alphanumeric stream.

ndic_<file name> - the dictionary for the non-alphanumeric stream.


A dictionary is a list of unique words from the source text file), ordered by descending frequencies; groups of low-frequency words are second-level ordered by the first occurrence in the text.

The size of the compressed text itself (by means of the arithmetic encoding), the size of the compressed context table, and the size of the main frequencies sequence are displayed.
