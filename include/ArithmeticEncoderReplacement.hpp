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

class ArithmeticEncoderReplacement : public ArithmeticEncoder {
private:
    map<string,int> word_symb_map;
    map<string,int> replace_map;
    map<string,bool> first_occurrence;
    SimpleFrequencyTable* freqs;
    WordTextReplacement* text;
    void write(string);
public:
    ArithmeticEncoderReplacement(WordTextReplacement*,BitOutputStream&);
    int encode();
};
