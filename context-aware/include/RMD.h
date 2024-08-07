#ifndef RMD_H
#define RMD_H
#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

class UniversalCode {
public:
    UniversalCode(){};
    virtual int encode(vector<uint64_t>){};
};

class DAC: public UniversalCode {
public:
    DAC(int x):d(x){};
    int encode(vector<uint64_t>);
private:
    int d;
};

class unary: public UniversalCode {
public:
    unary(){};
    int encode(vector<uint64_t>);
};

class RMD: public UniversalCode
{
    public:
        RMD(vector<int>,int);
        virtual ~RMD();
        int encode(vector<uint64_t>);
        void code_output(int);
        void rmd_to_file(string);
        int code_size(){return cur_byte+1;};
    protected:

    private:
        int t=sizeof(unsigned int)*8;
        int cur_byte=0,cur_bit=7,cur_value=0;
        vector<uint8_t> buffer;
        void flush_to_byte_rmd(uint64_t);
        vector<uint64_t> rmd;
};

#endif // RMD_H
