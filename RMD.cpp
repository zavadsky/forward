#include <iterator>
#include "include\RMD.h"

const int MAX_LEN=50,MAX_k=50,Lmax=44;
int nlk[MAX_LEN][MAX_k]; // array used in the decoding
int nl[MAX_LEN]; // nl[i] = number of codewords of length i

// 1) Generating the Reverse Multi-Delimiter codewords in rmd array. 2) Filling nlk array for the decoding.
// The parameters m1,m2,... of a code are all integers not present in the array k
// k - numbers which do not form delimiters 01^k, the last = biggest delimiter+1
RMD::RMD(vector<int> k,int N) {
int kmax=k.size(),Lmin; //number of elements in the array k; length of the shortest codeword
for(Lmin=0;Lmin<kmax && k[Lmin+1]==k[Lmin]+1;Lmin++);
    Lmin+=2;
int L,n,i,j,L1_long=-1,L1_long_prev=-1,t=Lmin-1;
uint64_t seq=0,s=0;
    if(N>((uint64_t)1<<Lmax))
        throw std::domain_error("Number of codewords is too big");
    kmax=k.size();
    for(L=Lmin,n=0;n<=N;L++) {
        nl[L]=n;
		for(i=0;(L-k[i]>Lmin) && (i<kmax);i++) {
			if(i==kmax-1 && L1_long==-1)
				L1_long=n;
			nlk[L][i]=n-nl[L-k[i]-1];
			for(j=nl[L-k[i]-1];j<nl[L-k[i]] && n<=N;j++,n++) {
				seq=(((uint64_t)1<<Lmax)-1)>>(Lmax-k[i]);
				rmd.push_back((rmd[j]<<(k[i]+1))|seq);
			}
		}
		if(L1_long_prev>-1) {
			nlk[L][kmax]=n-L1_long_prev;
			for(j=L1_long_prev;j<nl[L] && n<=N;j++,n++)
				rmd.push_back((rmd[j]<<1)|1);
		}
		if(t<kmax && L-1!=k[t]) {
			rmd.push_back(((uint64_t)1<<(L-1))-1);
			n++;
		} else
			t++;
		L1_long_prev=L1_long;
		L1_long=-1;
    }
}

RMD::~RMD()
{
    //dtor
}

// writes a codeword to the end of a code bitstream
// x is a codeword aligned to the right edge of a 32-bit word
void RMD::flush_to_byte_rmd(uint64_t x) {
int k(0),p;
unsigned int j;
// Find the leftmost '1' bit in the last byte of a code. Codeword starts 1 bit left to it
	for(j=1<<(t-1),p=t-1;!(x&j) && j;j>>=1,p--);
	for(j<<=1;j>0;j>>=1) {
		if(j&x)
			cur_value|=(1<<cur_bit);
		cur_bit=cur_bit==0?7:cur_bit-1;
		if(cur_bit==7) {
            buffer.push_back(cur_value);
			cur_value=0;
			cur_byte++;
		}
	}
}

int RMD::encode_rmd(vector<uint64_t> ranks) {
int i;
	cur_byte=0; cur_bit=7;
	buffer.clear();
	for(auto it=ranks.begin();it!=ranks.end();it++) {
		flush_to_byte_rmd(rmd[*it]); // write current codeword to the end of a code bitstream
	}
	return cur_byte*8+(7-cur_bit);
}

void RMD::rmd_to_file(string fname) {
    std::ofstream fout(fname, std::ios::binary);
    for(auto it=buffer.begin();it!=buffer.end();it++)
        fout<<*it;
    fout.close();
}


void RMD::code_output(int n) {
  #define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
  #define BYTE_TO_BINARY(byte)  \
  (byte & 0x800000 ? '1' : '0'), \
  (byte & 0x400000 ? '1' : '0'), \
  (byte & 0x200000 ? '1' : '0'), \
  (byte & 0x100000 ? '1' : '0'), \
  (byte & 0x080000 ? '1' : '0'), \
  (byte & 0x040000 ? '1' : '0'), \
  (byte & 0x020000 ? '1' : '0'), \
  (byte & 0x010000 ? '1' : '0'), \
  (byte & 0x8000 ? '1' : '0'), \
  (byte & 0x4000 ? '1' : '0'), \
  (byte & 0x2000 ? '1' : '0'), \
  (byte & 0x1000 ? '1' : '0'), \
  (byte & 0x0800 ? '1' : '0'), \
  (byte & 0x0400 ? '1' : '0'), \
  (byte & 0x0200 ? '1' : '0'), \
  (byte & 0x0100 ? '1' : '0'), \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

    FILE* file=fopen("code.txt","w");
    for(int i=0;i<n;i++)
        fprintf(file,"\n%d=%d=" BYTE_TO_BINARY_PATTERN,i,rmd[i],BYTE_TO_BINARY(rmd[i]));
    fclose(file);
}


