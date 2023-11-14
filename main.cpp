#include <time.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <bitset>
#include <vector>
#include <map>
#include <math.h>
#include <fstream>
#include <sys/timeb.h>
#include <windows.h>
#include <functional>
#include <queue>
#include <set>
#include "include\RMD.h"
#include "include\WordBasedText.h"
#include "include\BitIoStream.hpp"
#include "include\ArithmeticCoder.hpp"
#include "include\ArithmeticEncoderReplacement.hpp"

using namespace std;

typedef unsigned char uchar;
typedef unsigned int uint;

int main(int argc, char** argv) {
    if(argc<2)
        cout<<"Incorrect number of command line arguments.";
    else
        if(argc==2 || argv[3]=="-r") { // Forward Replacement Arithmetic
            string ifname(argv[1]);
            WordTextReplacement w(ifname);  // Pre-process the text
            std::ofstream out(ifname+".enc", std::ios::binary);
            BitOutputStream bout(out);
            ArithmeticEncoderReplacement enc(&w,bout); // Create the arithmetic encoder
            enc.encode(); cout<<"to the file "+ifname+".enc"<<endl;
            bout.finish();
            w.output_dic("dic_"+ifname); // Print the dictionary to the file
            RMD r({0,2,29},27,10000); //Generating the set of the RMD(1,3-inf) codewords
            w.CompressFrequencyTable(r,ifname+".frq"); // Compress the frequency table and print it to the file
        }
	system("pause");
}

