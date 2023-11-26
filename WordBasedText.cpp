#include "include\WordBasedText.h"

WordBasedText::WordBasedText(string fname)
{
    std::ifstream file(fname);
    buffer << file.rdbuf();
    word_frequences();
}

string WordBasedText::get_word(){
string s;
    buffer>>s;
    return s;
}

//Create different maps and dictionaries. Return number of words
int WordBasedText::word_frequences() {
double pi;
string word;
int size=0,i=0;
    // Create the map <word,frequency> - word_freq
	while ( ! buffer.eof() ) {
		buffer>>word;
		if(word_freq.find(word)!=word_freq.end()) {
			word_freq[word]++;
        } else {
			word_freq.insert(make_pair(word,1));
		}
		size++;
	}
	cout<<endl<<"Input file processed. <word,frequency> map buit.\n";
	// Create the multimap <frequency,word> consisting all different words - freq_word
	for(auto it=word_freq.begin();it!=word_freq.end();it++,diff_words++) {
		freq_word.insert(make_pair(it->second,it->first));
		// Calculate Shannon entropy
		pi=(double)it->second;
		entropy-=pi*(long double)log2((long double)pi/size);
	}
    cout<<"<frequency,word> map built"<<endl;
	// Create 1) map <word,symbol> which maps words of text to integers according to descending order of their frequencies;
	// 2) vector of Frequencies of all unique words; 3) vector of different Frequencies DiffFreq; 4) map freq_freq<frequency, number of words having this frequency>
int j=-1;
int frq=0;
NFreq=0;
multimap<int,string> :: iterator it1;
	for(it1=freq_word.begin(),i=0;it1!=freq_word.end();i++,it1++) {
        if(it1->first!=frq) {
            if(frq)
                freq_freq.insert(make_pair(frq,i-j));
            frq=it1->first;
            j=i;
            DiffFreq.push_back(frq);
            NFreq++;
        }
		word_symbol.insert(make_pair(it1->second,i)); //insert first element from freq_word map
		Frequencies.push_back(it1->first);
	}
    freq_freq.insert(make_pair(frq,i-j));
	cout<<"<word,symbol> map built. Different words in text: "<<diff_words<<". Entropy H0="<<(int)entropy/8<<" bytes."<<endl;

	return size;
}

void WordBasedText::output_dic(string fname) {
ofstream ofile(fname);
    for(auto it=word_freq.begin();it!=word_freq.end();it++)
        ofile<<it->first<<" ";
}

WordBasedText::~WordBasedText()
{
    //dtor
}

WordTextReplacement::WordTextReplacement(string s):WordBasedText(s) {
	for(auto it=freq_freq.begin();it!=freq_freq.end();it++) {
        int j=it->first;
        if(j==1 || (log2(it->second)+(j-1)*log2(j-1)>j*log2(j)))
            R.insert(j);
	}
}

void WordTextReplacement::output_dic(string fname) {
ofstream ofile(fname);
map<string,int> m=word_freq;
string word;
int j=1;
    auto it=freq_word.begin();
    cout<<"Create the dictionary. Replacement Frequencies processed (of "<<R.size()<<"): ";
    for(auto i=DiffFreq.begin();i!=DiffFreq.end();i++) {
        if(R.find(*i)==R.end()) {
            it=freq_word.find(*i);
            while ( it!=freq_word.end() && it->first==*i ) {
                ofile<<it->second<<" ";
                it++;
            }
        } else {
            cout<<j<<" ";
            j++;
            text_rewind();
            while ( ! eof() ) {
                word=get_word();
                int freq=m[word],init_freq=word_freq[word];
                if(init_freq==freq && freq==(*i)) {
                    ofile<<word<<" ";
                    m[word]--;
                }
            }
        }
    }
    cout<<endl<<"Dictionary output to the file "<<fname<<endl;
}

vector<uint64_t> WordTextReplacement::calc_f_deltas(int b) {
    vector<uint64_t> Freq_deltas;
    Freq_deltas.push_back(b);
    Freq_deltas.push_back(Frequencies[0]);
    int i,m;
    for(i=1;i<b;i++)
        Freq_deltas.push_back(Frequencies[i-1]-Frequencies[i]);
    int j=i;
    while(i<diff_words){
        int s=Frequencies[i-1]-Frequencies[i];
        for(m=1;m<s;m++,j++)
            Freq_deltas.push_back(0);
        int t=Frequencies[i],k=i;
        while(Frequencies[i]==t && i<diff_words)
            i++;
        Freq_deltas.push_back(i-k);
        j++;
    }
    return Freq_deltas;
}

void WordTextReplacement::CompressFrequencyTable(RMD r,string fname) {
    int deltas_min=1000000,deltas_minN;
    for(int N=NFreq*0.4;N<NFreq*0.6;N++) {
        vector<uint64_t> f=calc_f_deltas(N);
        int bit_size=r.encode_rmd(f);
        if(bit_size<deltas_min) {
            deltas_min=bit_size;
            deltas_minN=N;
        }
    }
    vector<uint64_t> f=calc_f_deltas(deltas_minN);
    int bit_size=r.encode_rmd(f);
    cout<<endl<<"Size of compressed frequencies="<<(int)bit_size/8<<" bytes."<<endl;
    cout<<"Compressed frequencies saved in the file "<<fname<<endl<<endl;
    r.rmd_to_file(fname);
}

void WordBasedText::CompressFrequencyTable(RMD r,string fname) {
vector<uint64_t> freqs_sorted;
     for(auto it=word_freq.begin();it!=word_freq.end();it++)
        freqs_sorted.push_back(it->second-1);
    int bit_size=r.encode_rmd(freqs_sorted);
    cout<<endl<<"Size of compressed frequencies="<<(int)bit_size/8<<" bytes."<<endl;
    cout<<"Compressed frequencies saved in the file "<<fname<<endl<<endl;
    r.rmd_to_file(fname);
}
