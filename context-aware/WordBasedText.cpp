#include "include\WordBasedText.h"

WordBasedText::WordBasedText(string fname,char t)
{
    std::ifstream file(fname);
    buffer << file.rdbuf();
    alpha_num=t;
    word_frequences();
}

bool isalnum0(char c,char prev) {
    return isalnum(c) || c=='-' || (int(c)==39 && isalnum(prev));
}

void WordBasedText::output_stream(string fname) {
string word;
    std::ofstream file(fname);
    text_rewind();
    while ( ! buffer.eof() ) {
		word=get_word();
		if(alpha_num!=0 && alpha_num!=3)
            file<<word<<" ";
        else {
            if(word==" ")
                word="";
            file<<word<<"a";
        }
    }
    file.close();
}

// getting the next word from the input stream
string WordBasedText::get_word() {
string sa="",sn="";
char a,c=0,prev;
    if(buffer.eof())
        return "";
    if(alpha_num=='#') { // punctuation for poetry; the set of non-alphanumeric characters in one line is treated as one character; a line should end with # character
        buffer>>noskipws>>c;
        if(c) {
            sn=c;
            while(c!='#' && !buffer.eof()) {
                buffer>>noskipws>>c;
                sn+=c;
            }
        }
        return sn;
    }
    do { // get the sequence of non-alphanumeric characters
        prev=c;
        buffer>>noskipws>>c;
        sn+=c==10?'#':c;
    } while(!isalnum0(c,prev) && ! buffer.eof());
    if(buffer.eof())
        return sn;
    buffer.unget();
    sn=sn.substr(0,sn.length()-1);
    do { // get the sequence of alphanumeric characters
        prev=c;
        buffer>>noskipws>>c;
        sa+=c;
    } while(isalnum0(c,prev) && ! buffer.eof());
    if(buffer.eof())
        return sa;
    buffer.unget();
    if(alpha_num==2) { // alphanumeric with 'backspace'
        if(sa[sa.length()-1]!=' ')
            sa[sa.length()-1]='~';
        else
            sa=sa.substr(0,sa.length()-1);
        return sa;
    } else
        if(alpha_num==1) { // alphanumeric without 'backspace'
            sa=sa.substr(0,sa.length()-1);
            return sa;
        }
    if(sn!="") // if alphanumeric word not yet returned, we have seek a non-alphanumeric one
        return sn;
    do {
        prev=c;
        buffer>>noskipws>>c;
        sn+=c==10?'#':c;
    } while(!isalnum0(c,prev) && ! buffer.eof());
    if(buffer.eof())
        return sn;
    buffer.unget();
    sn=sn.substr(0,sn.length()-1);
    return sn;
}

//Create different maps and dictionaries
int WordBasedText::word_frequences() {
double pi;
string word;
ofstream out("punctuation");
int i=0,k=0;
    Nwords=0;
    // Create the map <word,frequency> - word_freq
	while ( ! buffer.eof() ) {
		word=get_word();
		if(!(alpha_num==3 && word==" ")) {
            if(word_freq.find(word)!=word_freq.end())
                word_freq[word]++;
            else
                word_freq.insert(make_pair(word,1));
            Nwords++;
        }
    }
	// Create the multimap <frequency,word> consisting all different words - freq_word
	for(auto it=word_freq.begin();it!=word_freq.end();it++,diff_words++) {
		freq_word.insert(make_pair(it->second,it->first));
		// Calculate Shannon entropy
		pi=(double)it->second;
		entropy-=pi*(long double)log2((long double)pi/Nwords);
	}
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
	return Nwords;
}

// output the dictionary to the file
void WordBasedText::output_dic(string fname) {
ofstream ofile(fname);
string s=(alpha_num!=0 && alpha_num!=3)?" ":"a";
    for(auto it=word_freq.begin();it!=word_freq.end();it++)
        ofile<<it->first<<s;
}

WordBasedText::~WordBasedText()
{
    //dtor
}

// Initialize the textstream + the set of words for 1st occurrence replacement
WordTextReplacement::WordTextReplacement(string s,char t):WordBasedText(s,t) {
	for(auto it=freq_freq.begin();it!=freq_freq.end();it++) {
        int j=it->first;
        if(j==1 || (log2(it->second)+(j-1)*log2(j-1)>j*log2(j)))
            R.insert(j);
	}
}

// If first occurrences of words having a given frequency have to be replaced wit MC,
// output words to the dictionary in the order of first occurrence, otherwise - in the alphabetical order
void WordTextReplacement::output_dic(string fname) {
ofstream ofile(fname);
map<string,int> m=word_freq;
string word;
string s=(alpha_num!=0 && alpha_num!=3)?" ":"a";
int j=1;
    auto it=freq_word.begin();
    cout<<"Create the dictionary. Replacement Frequencies processed (of "<<R.size()<<"): ";
    for(auto i=DiffFreq.begin();i!=DiffFreq.end();i++) {
        if(R.find(*i)==R.end()) {
            it=freq_word.find(*i);
            while ( it!=freq_word.end() && it->first==*i ) {
                ofile<<it->second<<s;
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
                    ofile<<word<<s;
                    m[word]--;
                }
            }
        }
    }
    cout<<endl<<"Dictionary output to the file "<<fname<<endl;
}

// Create the vector of differences between consequent frequencies; needed for the frequency table compression
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

// Compressing the frequency table taking into account first occurence replacement
int WordTextReplacement::CompressFrequencyTable(RMD r,string fname) {
    int deltas_min=1000000,deltas_minN;
    for(int N=NFreq*0.4;N<NFreq*0.6;N++) {
        vector<uint64_t> f=calc_f_deltas(N);
        int bit_size=r.encode(f);
        if(bit_size<deltas_min) {
            deltas_min=bit_size;
            deltas_minN=N;
        }
    }
    vector<uint64_t> f=calc_f_deltas(deltas_minN);
    int bit_size=r.encode(f);
    cout<<endl<<"Size of compressed frequencies="<<(int)bit_size/8<<" bytes."<<endl;
    cout<<"Compressed frequencies saved in the file "<<fname<<endl<<endl;
    r.rmd_to_file(fname);
    return (int)bit_size/8;
}

// Compressing the basic frequency table
int WordBasedText::CompressFrequencyTable(RMD r,string fname) {
vector<uint64_t> freqs_sorted;
     for(auto it=word_freq.begin();it!=word_freq.end();it++)
        freqs_sorted.push_back(it->second-1);
    int bit_size=r.encode(freqs_sorted);
    cout<<endl<<"Size of compressed frequencies="<<(int)bit_size/8<<" bytes."<<endl;
    cout<<"Compressed frequencies saved in the file "<<fname<<endl<<endl;
    r.rmd_to_file(fname);
    return (int)bit_size/8;
}



