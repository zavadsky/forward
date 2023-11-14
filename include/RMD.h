#ifndef RMD_H
#define RMD_H
#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

class RMD
{
    public:
        RMD(vector<int>,int,int);
        virtual ~RMD();
        int encode_rmd(vector<int>,unsigned char*);
        void code_output(int);
        void rmd_to_file(string);
        int code_size(){return cur_byte+1;};
    protected:

    private:
        int t=sizeof(unsigned int)*8;
        int cur_byte=0,cur_bit=7;
        void flush_to_byte_rmd(unsigned int,unsigned char*);
        vector<int> rmd;
};

#endif // RMD_H
