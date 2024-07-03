#ifndef WORDBASEDTEXT_H
#define WORDBASEDTEXT_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <math.h>
#include "RMD.h"

using namespace std;

// class for the input text
class WordBasedText
{
    public:
        int glob=0;
        WordBasedText(string,char);
        virtual ~WordBasedText();
        int word_frequences(); // the main function that builds all maps and calculates statistics of the text
        virtual void output_dic(string); // output the dictionary to the file
        map<string,int> word_freq; // <word,frequency> map
        void text_rewind(){buffer.clear(std::stringstream::goodbit); buffer.seekg(0);}; // rewind the input stream
        bool eof(){return buffer.eof();};
        string get_word(); // get the next word from the input stream
        virtual int CompressFrequencyTable(RMD,string);
        int getMaxSymb(){return diff_words;};
        map<string,int> word_symbol; // map <word; number in the list, ordered by frequency>
        vector<uint64_t> Frequencies; // vector of descending frequencies of all words
        map<int,int> freq_freq; // <frequency, number of words of this frequency> map
        int Nwords; //total number of words in the text
        void output_stream(string);
        char is_alpha_num() {return alpha_num;}; // return the type of a textstream
    protected:
        multimap<int,string,greater<int>> freq_word; // <frequency,word> multimap
        vector<int> DiffFreq; // vector of distinct frequencies
        int NFreq;   // number of distinct frequencies
        int diff_words=0;
        char alpha_num; // 1 - alphanumeric, 0 - punctuation, 2 - alphanumeric with 'backspace' characters, 3 - punctuation with 'backspace' characters, # - punctuation for poetry

    private:
        stringstream buffer; // buffer containing the input stream
        double entropy;
};

// input text + some data for low frequent characters replacement
class WordTextReplacement: public WordBasedText
{
    public:
        WordTextReplacement(string,char);
        virtual ~WordTextReplacement(){};
        set<int> R;  // set of replacement frequencies
        void output_dic(string);
        int CompressFrequencyTable(RMD,string);
    private:
        vector<uint64_t> calc_f_deltas(int);
        void CreateContextMap();
};

#endif // WORDBASEDTEXT_H
