#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include "BitIoStream.hpp"
#include "FrequencyTable.hpp"
#include "WordBasedText.h"
#include "ArithmeticCoder.hpp"

using namespace std;

// Forward encoder + replacement of low-frequency symbols with meta-characters
class ArithmeticEncoderReplacement : public ArithmeticEncoder {
protected:
    map<string,int> word_symb_map; // mapping words to integer symbols used in arithmetic encoding
    map<string,int> replace_map; // <word, which first occurrence to be replaced with meta-character; new frequency of this word>
    map<string,bool> first_occurrence; //<word, if it has already occurred flag>
    SimpleFrequencyTable* freqs; // Frequency table used by the arithmetic encoder
    WordTextReplacement* text; // input text
    void write(string); // encode the word with the arithmetic encoder
public:
    ArithmeticEncoderReplacement(WordTextReplacement*,BitOutputStream&);
    int encode();
    double emulate(); // Calculate the size of the encoded text without actual encoding
    int get_symbol(string,map<string,bool>&); // Convert a word to an integer symbol
};

// Forward encoder + meta-characters + context-aware PPM-style encoding
class EncoderReplacementContext : public ArithmeticEncoderReplacement {
private:
    int T=200;
    double P=20;
    map<int,map<int,int>> c_table; // <symbol x, <symbol next to x,context frequency>> map
    map<int,int> c_freq; // <symbol x, total frequency of the limited context of x>
    /*map<int,int> csymbols_freq; // <symbol x, total frequency of the limited context of x>
    map<int,vector<int>> symbol_context; // <symbol x, list of contexts where x occurs> */
    map<string,int> new_word_symb; // mapping words to integer symbols used in arithmetic encoding
    map<string,int> new_replace;
    map<int,int> ooc; // <symbol, frequency of symbol out of all contexts> map
    //map<int,int> nfreq;
    int Ctotal; // summary frequency of symbols that define contexts
    int inCtotal; // total number of context defined symbols
    map<int,int> symb_freq; // total frequency of each symbol
public:
    EncoderReplacementContext(WordTextReplacement*,BitOutputStream&,int,double);
    double emulate();
    int Compress_c_table(UniversalCode*,UniversalCode*,UniversalCode*,UniversalCode*);
};

// Arithmetic encoder based on forward coding
class ArithmeticEncoderForward : public ArithmeticEncoder {
private:
    SimpleFrequencyTable* freqs;
    WordBasedText* text;
public:
    ArithmeticEncoderForward(WordBasedText*,BitOutputStream&);
    int encode();
    double emulate();
};
