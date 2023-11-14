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


int arithm_forward(std::vector<std::uint64_t> freq_vector,int maxSymbol,std::string inputFile,std::string outputFile,map<string,int> rmd_map_sorted) {

	// Perform file compression
	std::ifstream in(inputFile);
	std::ofstream out(outputFile, std::ios::binary);
	BitOutputStream bout(out);
	std::string word;
	try {

		SimpleFrequencyTable freqs(freq_vector);
		ArithmeticEncoder enc(32, bout);
		while(! in.eof()) {
			// Read and encode one byte
			int symbol;
			in>>word;
            if(rmd_map_sorted.find(word)==rmd_map_sorted.end())
                throw std::logic_error("Word not found");
            symbol=rmd_map_sorted[word];

			if (symbol == std::char_traits<char>::eof())
				break;
			if (!(0 <= symbol && symbol <= maxSymbol))
				throw std::logic_error("Assertion error");
			enc.write(freqs, static_cast<uint32_t>(symbol));
            freqs.decrement(static_cast<uint32_t>(symbol)); //Forward-looking compression. Comment this line to make static
			//freqs.increment(static_cast<uint32_t>(symbol));
		}

		enc.write(freqs, maxSymbol);  // EOF
		enc.finish();  // Flush remaining code bits
		bout.finish();
		cout<<"Arithmetic encoding completed."<<endl;
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
/*    for (uint64_t i: repl_freq)
        std::cout << i << ' ';*/
    freqs = new SimpleFrequencyTable(repl_freq);
    text=w;
    cout<<endl<<"Arithmetic encoder with replacement initialized."<<endl;
}

int ArithmeticEncoderReplacement::encode() {
	std::string word;
	try {
        text->text_rewind();
		while(! text->eof()) {
			word=text->get_word();
			write(word);
    	}
		finish();  // Flush remaining code bits
		cout<<"Arithmetic encoding completed ";
		return EXIT_SUCCESS;

	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}
