#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include "include\ArithmeticEncoderReplacement.hpp"
#include "include\BitIoStream.hpp"
#include "include\FrequencyTable.hpp"

using std::uint32_t;
using namespace std;


ArithmeticEncoderForward::ArithmeticEncoderForward(WordBasedText* w,BitOutputStream &out):
    ArithmeticEncoder(32, out) {
    freqs = new SimpleFrequencyTable(w->Frequencies);
    text=w;
    cout<<endl<<"Arithmetic encoder initialized."<<endl;
}


int ArithmeticEncoderForward::encode() {
	// Perform file compression
	std::string word;
	int maxSymbol=text->getMaxSymb();
	try {
        text->text_rewind();
		while(! text->eof()) {
			word=text->get_word();
            if(word=="")
                break;
			int symbol=text->rmd_map_sorted[word];
            if (symbol == std::char_traits<char>::eof())
                break;
            if (!(0 <= symbol && symbol <= maxSymbol))
                throw std::logic_error("Assertion error");
            write(*freqs, static_cast<uint32_t>(symbol));
            freqs->decrement(static_cast<uint32_t>(symbol));
		}
		finish();  // Flush remaining code bits
		cout<<"Arithmetic encoding completed ";
		return EXIT_SUCCESS;

	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}

void ArithmeticEncoderReplacement::write(string word) {
int symbol;
    if(word=="")
        return;
    if(replace_map.find(word)!=replace_map.end() && first_occurrence[word]) {
        symbol=replace_map[word];
    } else
        symbol=word_symb_map[word];
    ArithmeticEncoder::write(*freqs,symbol);
    freqs->decrement(static_cast<uint32_t>(symbol));
    first_occurrence[word]=0;
}

ArithmeticEncoderReplacement::ArithmeticEncoderReplacement(WordTextReplacement* w,BitOutputStream &out):
    ArithmeticEncoder(32, out) {
map<int,int> K;
int i=0;
vector<uint64_t> repl_freq;
    for(auto it=w->wf_map.begin();it!=w->wf_map.end();it++) {
        first_occurrence.insert(make_pair(it->first,1));
        if(w->R.find(it->second)==w->R.end()) {
            repl_freq.push_back(it->second);
            word_symb_map.insert(make_pair(it->first,i));
            i++;
        } else {
            if(it->second>1) {
                repl_freq.push_back(it->second-1);
                word_symb_map.insert(make_pair(it->first,i));
                i++;
            }
            replace_map.insert(make_pair(it->first,it->second-1));
            if(K.find(it->second-1) != K.end())
                K[it->second-1]++;
            else
                K.insert(make_pair(it->second-1,1));
        }
    }
    for(auto it=replace_map.begin();it!=replace_map.end();it++)
        it->second+=i;
    for(auto it=K.begin();it!=K.end();it++)
        repl_freq.push_back(it->second);
    freqs = new SimpleFrequencyTable(repl_freq);
    text=w;
    cout<<endl<<"Arithmetic encoder with replacement initialized."<<endl;
}

int ArithmeticEncoderReplacement::encode() {
	std::string word;
	try {
        text->text_rewind();
        cout<<"10KB words processed: ";
        int size=0;
		while(! text->eof()) {
			word=text->get_word();
			write(word);
            if(size%10000==9999)
                cout<<int((size+1)/10000)<<" ";
            size++;
    	}
		finish();  // Flush remaining code bits
		cout<<"Arithmetic encoding completed ";
		return EXIT_SUCCESS;

	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}
