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

class WordBasedText
{
    public:
        WordBasedText(string);
        virtual ~WordBasedText();
        int word_frequences();
        virtual void output_dic(string);
        map<string,int> wf_map;
        void text_rewind(){buffer.clear(std::stringstream::goodbit); buffer.seekg(0);};
        bool eof(){return buffer.eof();};
        string get_word();
        virtual void CompressFrequencyTable(RMD,string);
        int getMaxSymb(){return diff_words;};
        map<string,int> rmd_map_sorted; // The map <word; number in the list, ordered by frequency>
        vector<uint64_t> Frequencies; // Vector of descending frequencies of all words

    protected:
        int MaxF=0;
        map<int,int> wordsF; // <frequency, number of words of this frequency> map
        multimap<int,string,greater<int>> freq_rmd;
        vector<int> DiffFreq;
        int NFreq;
        int diff_words=0;

    private:
        stringstream buffer;
        double entropy;
};

class WordTextReplacement: public WordBasedText
{
    public:
        WordTextReplacement(string);
        virtual ~WordTextReplacement(){};
        set<int> R;
        void output_dic(string);
        void CompressFrequencyTable(RMD,string);
    private:
        vector<uint64_t> calc_f_deltas(int);
};

#endif // WORDBASEDTEXT_H
