#ifndef WORDBASEDTEXT_H
#define WORDBASEDTEXT_H

#include<string>
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
    static const int maxRsize=150;
    //static const int FTableSize=1500;
    public:
        WordBasedText(string);
        virtual ~WordBasedText();
        int word_frequences();
        void output_dic_sorted(string);
        map<string,int> wf_map;
        void text_rewind(){buffer.clear(std::stringstream::goodbit); buffer.seekg(0);};
        bool eof(){return buffer.eof();};
        string get_word();
        void EncodeFrequencyTable(RMD,string);
        void CompressFrequencyTable(RMD,string);
        int getMaxSymb(){return diff_words;};
        map<string,int> rmd_map_sorted; // The map <word; number in the list, ordered by frequency>
        vector<uint64_t> Frequencies;

    protected:
        int diff_words=0,MaxF=0;
        int wordsF[maxRsize];
        multimap<int,string> freq_rmd;
        vector<int> DiffFreq;

    private:
        stringstream buffer;
        vector<string> Dict_rmd;
        double entropy;
        int NFreq;
        vector<uint64_t> calc_f_deltas(int);
};

class WordTextReplacement: public WordBasedText
{
    public:
        WordTextReplacement(string);
        virtual ~WordTextReplacement(){};
        set<int> R;
        void output_dic(string);
};

#endif // WORDBASEDTEXT_H
