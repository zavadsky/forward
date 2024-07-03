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
			int symbol=text->word_symbol[word];
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

double ArithmeticEncoderForward::emulate() {
std::string word;
double ent=0;
	int maxSymbol=text->getMaxSymb();
	try {
        text->text_rewind();
		while(! text->eof()) {
			word=text->get_word();
            if(word!="") {
             	int symbol=text->word_symbol[word];
                if (symbol == std::char_traits<char>::eof())
                    break;
                if (!(0 <= symbol && symbol <= maxSymbol))
                    throw std::logic_error("Assertion error");
                ent-=(long double)log2(freqs->probability(symbol));
                freqs->decrement(static_cast<uint32_t>(symbol));
            }
		}
		printf("Forward entropy=%.2f bytes.\n",ent/8);
        return ent;
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
int i=0;
vector<uint64_t> repl_freq; // list of all symbol frequencies
    for(auto it=w->word_freq.begin();it!=w->word_freq.end();it++) {
        first_occurrence.insert(make_pair(it->first,1)); // set flags denoting first occurrences of words
        if(w->R.find(it->second)==w->R.end()) { // if a word is not to be replaced
            repl_freq.push_back(it->second);
            word_symb_map.insert(make_pair(it->first,i));
            i++;
        } else {
            if(it->second>1) { // if a word is to be replaced, but occurs more than once
                repl_freq.push_back(it->second-1);
                word_symb_map.insert(make_pair(it->first,i));
                i++;
            }
            replace_map.insert(make_pair(it->first,it->second-1));
        }
    }
    for(auto it=replace_map.begin();it!=replace_map.end();it++)
        it->second+=i; // i is the number of words that appear in word_symb_map (words of frequency > 1)
    for(auto it=w->R.begin();it!=w->R.end();it++) {
        repl_freq.push_back(w->freq_freq[*it]);
    }
    freqs = new SimpleFrequencyTable(repl_freq);
    text=w;
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


double ArithmeticEncoderReplacement::emulate() {
std::string word;
double ent=0;
int symbol;
	try {
        text->text_rewind();
        cout<<"100KB words processed: ";
        int size=0;
		while(! text->eof()) {
			word=text->get_word();
            if(word!="") {
                if(replace_map.find(word)!=replace_map.end() && first_occurrence[word])
                    symbol=replace_map[word];
                else {
                    symbol=word_symb_map[word];
                }
                ent-=(long double)log2(freqs->probability(symbol));
                freqs->decrement(static_cast<uint32_t>(symbol));
                first_occurrence[word]=0;
            }
            size++;
		}
	}   catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
	printf("Forward entropy=%.2f bytes.\n",ent/8);
	return ent;
}

void print_map(map<int,int> a) {
    cout<<"======= Print map ========="<<endl;
    for(auto it=a.begin();it!=a.end();it++)
        cout<<it->first<<" "<<it->second<<endl;
}

void print_map(map<int,map<int,int>> a) {
    cout<<"======= Print map ========="<<endl;
    for(auto it=a.begin();it!=a.end();it++) {
        cout<<endl<<it->first<<"------------------"<<endl;
        for(auto it1=it->second.begin();it1!=it->second.end();it1++)
            cout<<it1->first<<" "<<it1->second<<" | ";
    }
}

void print_vector(vector<uint64_t> a) {
    cout<<"======= Print vector ========="<<endl;
    for(auto it=a.begin();it!=a.end();it++)
        cout<<*it<<" ";
    cout<<endl;
}

// Initialize context-aware encoder
EncoderReplacementContext::EncoderReplacementContext(WordTextReplacement* w,BitOutputStream &out,int t,double p): T(t), P(p),
    ArithmeticEncoderReplacement(w, out) {
multimap<int,int,greater<int>> freq_symb; // <frequency, symbol> multimap sorted in descending frequency order
map<int,int> symb_symb; //old to new (ordered by descending frequencies) symbol map
map<int,map<int,int>> rt;
vector<uint64_t> repl_freq;
map<string,bool> fo=first_occurrence; // set flags denoting first occurrences of words
int i=0,ts=0;
    for(auto it=w->word_freq.begin();it!=w->word_freq.end();it++) {
        if(w->R.find(it->second)==w->R.end()) { // if the first occurrence of a word is not to be replaced
            freq_symb.insert(make_pair(it->second,i));
            i++;
        } else
            if(it->second>1) { // if a word is to be replaced, but occurs more than once
                freq_symb.insert(make_pair(it->second-1,i));
                i++;
            }
    }
    for(auto it=w->R.begin();it!=w->R.end();it++)
        freq_symb.insert(make_pair(w->freq_freq[*it],*it+i-1));
    int j=0;
    for(auto it = freq_symb.begin();it != freq_symb.end(); ++it,j++) {
        symb_symb.insert(make_pair(it->second,j));
        symb_freq.insert(make_pair(j,it->first));
        repl_freq.push_back(it->first);
    }
    freqs = new SimpleFrequencyTable(repl_freq);
    for(auto it=replace_map.begin();it!=replace_map.end();it++)
        new_replace.insert(make_pair(it->first,symb_symb[it->second]));
    for(auto it=word_symb_map.begin();it!=word_symb_map.end();it++)
        new_word_symb.insert(make_pair(it->first,symb_symb[it->second]));
    replace_map=new_replace;
    word_symb_map=new_word_symb;
int symbol,prev_symbol=1000000,k=0,u=0;
string word;
    text->text_rewind();
    while(! text->eof()) {
		word=text->get_word();
        if(word!="" && !(text->is_alpha_num()==3 && word==" ")) {
            symbol=get_symbol(word,fo);
            if(symb_freq[prev_symbol]>T)
                if(rt[prev_symbol].find(symbol)==rt[prev_symbol].end())
                    rt[prev_symbol].insert(make_pair(symbol,1));
                else
                    rt[prev_symbol][symbol]++;
        }
        prev_symbol=symbol;
    }
    Ctotal=inCtotal=0;
    ooc=symb_freq;
    for(auto it=rt.begin();it!=rt.end();it++) {
         int sum=0;
        for(auto it1=(it->second).begin();it1!=(it->second).end();it1++) {
            double prob=log2(((double)it1->second/symb_freq[it->first])/((double)symb_freq[it1->first]/text->Nwords))*it1->second;
            if(abs(prob)>P) { // if symbol it1 has to be included into the context table CT[it]
                c_table[it->first].insert(*it1);
                ooc[it1->first]-=it1->second;
                sum+=it1->second;
            }
        }
        if(sum>0) {
            c_freq.insert(make_pair(it->first,sum));
            Ctotal+=symb_freq[it->first];
            inCtotal+=sum;
        }
    }
    //print_map(symb_freq);
    //print_map(c_table);

}

// If the word occurs for the first time and has to be replaced with MC, return MC(word), otherwise return symbol(word)
int ArithmeticEncoderReplacement::get_symbol(string word,map<string,bool> &fo) {
int symbol;
    if(replace_map.find(word)!=replace_map.end() && fo[word])
        symbol=replace_map[word];
    else
        symbol=word_symb_map[word];
    fo[word]=0;
    return symbol;
}

// Calculate the size of the encoded text. Context table and other auxiliary information will be destroyed.
double EncoderReplacementContext::emulate() {
std::string word;
double ent=0; // entropy = emulated compressed size, excluding tables
int symbol,prev_symbol=1000000; // prev_symbol relates to c, while symbol relates to w in Alg. 5.2 in the paper
map<string,bool> fo=first_occurrence;
	try {
    int k=0;
        text->text_rewind();
        while(! text->eof()) {
			word=text->get_word();
            if(word!="" && !(text->is_alpha_num()==3 && word==" ")) {
                symbol=get_symbol(word,fo); //Get the symbol, corresponding to a word, possibly meta-characters

                // Encode-and-Update-the-Model
                if(c_table.find(prev_symbol)!=c_table.end()) { // if context symbol in CT (line 1)
                    if(c_table[prev_symbol].find(symbol)!=c_table[prev_symbol].end()) { // if current symbol in CT[context] (line 2)
                        ent-=(long double)log2((long double)c_table[prev_symbol][symbol]/(freqs->get(prev_symbol)+1)); // line 3
                        c_table[prev_symbol][symbol]--; // line 4
                        c_freq[prev_symbol]--; // line 5
                        if(c_table[prev_symbol][symbol]<=0)
                            c_table[prev_symbol].erase(symbol);
                    } else {
                        // line 6
                        int sum=0;
                        for(auto it=c_table[prev_symbol].begin();it!=c_table[prev_symbol].end();it++)
                            sum+=freqs->get(it->first);
                        // line 7
                        ent-=(long double)log2((long double)freqs->get(symbol)/((long double)freqs->getTotal()-sum)*((long double)(freqs->get(prev_symbol)+1-c_freq[prev_symbol])/(freqs->get(prev_symbol)+1)));
                    }
                } else
                   ent-=(long double)log2((long double)freqs->get(symbol)/(long double)freqs->getTotal()); // if context symbol not in CT (line 8)
                freqs->decrement(static_cast<uint32_t>(symbol)); //line 10
                prev_symbol=symbol; // symbol in the context becomes the context itself at next iteration
            }
		}
	}   catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
	printf("Context-aware forward entropy=%.2f bytes.",ent/8);
	return ent/8;
}

// Compressing the Partial context frequencies table using universal codes
int EncoderReplacementContext::Compress_c_table(UniversalCode *r1,UniversalCode *r2,UniversalCode *r3,UniversalCode *r4) {
vector<uint64_t> Sizes,Deltas,freqs,symbs;
int Prev_symb=-1,prev_symb,freq_min,s1,s2,s3,s4;
    Sizes.push_back(c_table.size()); // Save the number of context symbols
    for(auto it=c_table.begin();it!=c_table.end();it++) {
        Deltas.push_back(it->first-Prev_symb-1);
        Prev_symb=it->first;
        Sizes.push_back(it->second.size()-1);
        prev_symb=-1;
        freq_min=1000000;
        for(auto it1=it->second.begin();it1!=it->second.end();it1++) {
            symbs.push_back(it1->first-prev_symb-1);
            prev_symb=it1->first;
            if(it1->second<freq_min)
                freq_min=it1->second;
        }
        freqs.push_back(freq_min);
        if(it->second.size()>1)
            for(auto it1=it->second.begin();it1!=it->second.end();it1++)
                freqs.push_back(it1->second-freq_min);
    }
    s1=r1->encode(Sizes)/8;
    s2=r2->encode(Deltas)/8;
    s3=r3->encode(freqs)/8;
    s4=r4->encode(symbs)/8;
    cout<<"CT compressed size: "<<s1+s2+s3+s4<<" (sizes="<<s1<<"; deltas="<<s2<<"; frequencies="<<s3<<"; symbols="<<s4<<")."<<endl;
    return s1+s2+s3+s4;
}
