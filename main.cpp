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
    if(argc<2)
        cout<<"Incorrect number of command line arguments.";
    else {
        string ifname(argv[1]);
        std::ofstream out(ifname+".enc", std::ios::binary);
        BitOutputStream bout(out);
        WordBasedText *w;
        RMD *r;
        ArithmeticEncoder *enc;
        if(argc==2 || strcmp(argv[2], "-r") == 0) { // Forward Replacement Arithmetic
            w = new WordTextReplacement(ifname);  // Pre-process the text
            enc = new ArithmeticEncoderReplacement((WordTextReplacement*)w,bout); // Create the arithmetic encoder
            r = new RMD({0,2,30},500000); //Generating the set of the RMD(1,3-inf) codewords
        } else {
            w = new WordBasedText(ifname);  // Pre-process the text
            enc = new ArithmeticEncoderForward(w,bout); // Create the arithmetic encoder
            r = new RMD({0,3,30},500000); //Generating the set of the RMD(1-3,5-inf) codewords
        }
        if(argc>3 && strcmp(argv[3], "-e") == 0)
            enc->emulate();
        else {
            enc->encode();
            cout<<"to the file "+ifname+".enc"<<endl;
        }
        bout.finish();
        w->output_dic("dic_"+ifname); // Print the dictionary to the file
        w->CompressFrequencyTable(*r,ifname+".frq"); // Compress the frequency table and print it to the file
    }
	system("pause");
}

