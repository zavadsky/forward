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

//Create different maps and dictionaries
int WordBasedText::word_frequences() {
double pi;
string word;
int size=0,i=0;
    // Create the map <word,frequency> - wf_map
	while ( ! buffer.eof() ) {
		buffer>>word;
		if(wf_map.find(word)!=wf_map.end()) {
			wf_map[word]++;
        } else {
			wf_map.insert(make_pair(word,1));
		}
		size++;
	}
	cout<<endl<<"Input file processed. <word,frequency> map buit.\n";
	// Create the multimap <frequency,word> consisting all different words - freq_rmd
	for(auto it=wf_map.begin();it!=wf_map.end();it++,diff_words++) {
		freq_rmd.insert(make_pair(it->second,it->first));
		// Calculate Shannon entropy
		pi=(double)it->second;
		entropy-=pi*(long double)log2((long double)pi/size);
	}
    cout<<"<frequency,word> map built"<<endl;
int prev_f=0;
    for(auto it=freq_rmd.begin();it!=freq_rmd.end() && it->first<=prev_f+1;it++) {
        if(it->first==prev_f+1)
            MaxF++;
        prev_f=it->first;
    }
	// Create 1) the map <word,index> which maps words of text to integers according to descending order of their frequences;
	// 2) ordered dictionary Dict_rmd; 3) vector of Frequencies of all unique words; 4) vector of different Frequencies DiffFreq
int j=MaxF;
int frq=0;
NFreq=0;
multimap<int,string> :: iterator it1;
	for(it1=freq_rmd.end(),i=0;it1!=freq_rmd.begin();i++) {
        it1--;
        if(it1->first!=frq) {
            frq=it1->first;
            DiffFreq.push_back(frq);
            NFreq++;
            if(it1->first==j) {
                wordsF[j]=i;
                j--;
            }
        }
		rmd_map_sorted.insert(make_pair(it1->second,i)); //insert first element from freq_rmd map
		Dict_rmd.push_back(it1->second);
		Frequencies.push_back(it1->first);
	}
	rmd_map_sorted.insert(make_pair(it1->second,i));
	Dict_rmd.push_back(it1->second);
	cout<<"<word,index> map built. Different words in text: "<<diff_words<<". Entropy H0="<<(int)entropy/8<<" bytes."<<endl;

	return size;
}

void WordBasedText::output_dic(string fname) {
ofstream ofile(fname);
    for(auto it=wf_map.begin();it!=wf_map.end();it++)
        ofile<<it->first<<" ";
}

WordBasedText::~WordBasedText()
{
    //dtor
}

WordTextReplacement::WordTextReplacement(string s):WordBasedText(s) {
    int pos=diff_words;
    //wordsF is the array consisting of numbers of words of particular frequency
	for(int j=1;j<MaxF;j++) {
        wordsF[j]=pos-wordsF[j];
        if(j==1 || (log2(wordsF[j])+(j-1)*log2(j-1)>j*log2(j)))
            R.insert(j);
        pos-=wordsF[j];
	}
}

void WordTextReplacement::output_dic(string fname) {
ofstream ofile(fname);
map<string,int> m=wf_map;
string word;
    auto it=freq_rmd.begin();
    for(auto i=DiffFreq.begin();i!=DiffFreq.end();i++) {
        it=freq_rmd.find(*i);
        if(R.find(*i)==R.end())
            while ( it!=freq_rmd.end() && it->first==*i ) {
                ofile<<it->second<<" ";
                it++;
            }
        else {
            text_rewind();
            while ( ! eof() ) {
                word=get_word();
                int freq=m[word],init_freq=wf_map[word];
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
     for(auto it=wf_map.begin();it!=wf_map.end();it++)
        freqs_sorted.push_back(it->second-1);
    int bit_size=r.encode_rmd(freqs_sorted);
    cout<<endl<<"Size of compressed frequencies="<<(int)bit_size/8<<" bytes."<<endl;
    cout<<"Compressed frequencies saved in the file "<<fname<<endl<<endl;
    r.rmd_to_file(fname);
}
