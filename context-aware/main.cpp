#include <string.h>
#include <iostream>
#include <fstream>
#include "include\RMD.h"
#include "include\WordBasedText.h"
#include "include\BitIoStream.hpp"
#include "include\ArithmeticCoder.hpp"
#include "include\ArithmeticEncoderReplacement.hpp"

using namespace std;

int main(int argc, char** argv) {
int tot=0,pt,th_f_m,th_g_m,th_f_p,th_g_p;
    if(argc<5) {
        cout<<"Incorrect number of command line arguments."<<endl;
        cout<<"Arguments format: <text file name> <th_f mainstream> <th_g mainstream> <th_f punctuation> <th_g punctuation> [poetry|bks]"<<endl;
    } else {
        string ifname(argv[1]);
        std::ofstream out(ifname+".enc", std::ios::binary);
        BitOutputStream bout(out);
        th_f_m=atoi(argv[2]); th_g_m=atoi(argv[3]); th_f_p=atoi(argv[4]); th_g_p=atoi(argv[5]);
        WordBasedText *wa,*wn;
        RMD *r;
        ArithmeticEncoder *enca,*encn;
        if(argc>6 && !strcmp(argv[6],"bks")) {
            wa = new WordTextReplacement(ifname,2);  // Pre-process the main text stream, 'backspace character' approach
            wn = new WordTextReplacement(ifname,3);  // Pre-process the punctuation, 'backspace character' approach
        } else {
            wa = new WordTextReplacement(ifname,1);  // Pre-process the main text stream
            wn = new WordTextReplacement(ifname,0);  // Pre-process the punctuation
        }
        if(argc>6 && !strcmp(argv[6],"poetry")) {
            wn->output_stream("s_p_stream.txt");        // for poetry
            wn = new WordTextReplacement("s_p_stream.txt",'#');  // Pre-process the punctuation stream for poetry
        }
        enca = new EncoderReplacementContext((WordTextReplacement*)wa,bout,th_f_m,th_g_m); // Encoder for the main text stream
        encn = new EncoderReplacementContext((WordTextReplacement*)wn,bout,th_f_p,th_g_p); // Encoder for the punctuation stream
        //
        r = new RMD({0,2,30},3000000); //Generating the set of the RMD(1,3-inf) codewords
        UniversalCode *r1=new RMD({0,4,30},50000); UniversalCode *r2=new RMD({0,30},50000); UniversalCode *r3=new RMD({0,4,30},3000000); UniversalCode *r4=new RMD({0,1,30},3000000);
        UniversalCode *r5=new DAC(1); UniversalCode *r6=new DAC(1); UniversalCode *r7=new RMD({0,3,30},3000000); UniversalCode *r8=new RMD({0,2,30},50000);
        cout<<endl<<"======== Alphanumeric ========"<<endl;
        tot=enca->Compress_c_table(r1,r2,r3,r4);
        tot+=enca->emulate();
        tot+=wa->CompressFrequencyTable(*r,"a_"+ifname+".frq"); // Compress the frequency table and print it to the file
        wa->output_dic("adic_"+ifname); // Print the sorted dictionary to the file
        cout<<endl<<"======== Punctuation ========"<<endl;
        tot+=encn->Compress_c_table(r5,r6,r7,r8);
        tot+=encn->emulate();
        tot+=wn->CompressFrequencyTable(*r,"n_"+ifname+".frq"); // Compress the frequency table and print it to the file
        wn->output_dic("ndic_"+ifname); // Print the sorted dictionary to the file
        bout.finish();
        cout<<endl<<"Total size without dictionaries: "<<tot<<endl;
    }
	system("pause");
}

