/*
 * Reference arithmetic coding
 * Copyright (c) Project Nayuki
 *
 * https://www.nayuki.io/page/reference-arithmetic-coding
 * https://github.com/nayuki/Reference-arithmetic-coding
 */

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <math.h>
#include "include\BitIoStream.hpp"


BitInputStream::BitInputStream(std::istream &in) :
	input(in),
	currentByte(0),
	numBitsRemaining(0) {}


int BitInputStream::read() {
	if (currentByte == -1)
		return -1;
	if (numBitsRemaining == 0) {
		currentByte = input.get();  // Note: istream.get() returns int, not char
		if (currentByte == EOF)
			return -1;
		if (currentByte < 0 || currentByte > 255)
			throw std::logic_error("Assertion error");
		numBitsRemaining = 8;
	}
	if (numBitsRemaining <= 0)
		throw std::logic_error("Assertion error");
	numBitsRemaining--;
	return (currentByte >> numBitsRemaining) & 1;
}

int BitInputStream::readNum(int length)
{
	int num = 0;
	for (int i = 0; i < length; i++) {
		num <<= 1;
		num |= read();
	}
	return num;
}

int BitInputStream::readNumEOF(int length)
{
	int num = 0;
	for (int i = 0; i < length; i++) {
		num <<= 1;
		int next = read();
		if (next == -1)
			return -1;
		num |= next;
	}
	return num;
}

int BitInputStream::readNoEof() {
	int result = read();
	if (result != -1)
		return result;
	else
		throw std::runtime_error("End of stream");
}


BitOutputStream::BitOutputStream(std::ostream& out) :
	output(out),
	currentByte(0),
	numBitsFilled(0),
	totalBytes(0) {}


void BitOutputStream::write(int b) {
	if (b != 0 && b != 1)
		throw std::domain_error("Argument must be 0 or 1");
	currentByte = (currentByte << 1) | b;
	numBitsFilled++;
	if (numBitsFilled == 8) {
		// Note: ostream.put() takes char, which may be signed/unsigned
		if (std::numeric_limits<char>::is_signed)
			currentByte -= (currentByte >> 7) << 8;
		output.put(static_cast<char>(currentByte));
        //std::bitset<8> x(currentByte);
        //std::cerr << x << " " << std::flush;
		currentByte = 0;
		numBitsFilled = 0;
		totalBytes++;
	}
}

void BitOutputStream::writeNum(int num, int length)
{
	for (int i = length - 1; i >= 0; i--) {
		write((num >> i) & 1);
	}
}


void BitOutputStream::finish() {
	while (numBitsFilled != 0)
		write(0);
}


void BitOutputStream::encodeCdelta(uint64_t x)
{
    uint64_t num = x;
    uint64_t len = 0;
    uint64_t lengthOfLen = 0;

    len = 1 + floor(log2(num));  // calculate 1+floor(log2(num))
    lengthOfLen = 1 + floor(log2(len)); // calculate floor(log2(len))

    for (uint64_t i = lengthOfLen - 1; i > 0; --i)
    {
        write(0);
        //std::cerr << 0;
    }
    for (uint64_t i = lengthOfLen - 1; i + 1 >= 1; --i)
    {
        write((len >> i) & 1);
        //std::cerr << ((len >> i) & 1);
    }
    //for (int i = len - 2; i >= 0; i--)
    for (uint64_t i = len - 2; i + 1 >= 1; i--)
    {
        write((num >> i) & 1);
        //std::cerr << ((num >> i) & 1);
    }
}

std::uint64_t BitInputStream::decodeCdelta()
{
    uint64_t num = 1;
    uint64_t len = 1;
    uint64_t lengthOfLen = 0;
    while (!readNoEof())     // potentially dangerous with malformed files.
        lengthOfLen++;
    for (uint64_t i = 0; i < lengthOfLen; i++)
    {
        len <<= 1;
        if (readNoEof())
            len |= 1;
    }
    for (uint64_t i = 0; i < len - 1; i++)
    {
        num <<= 1;
        if (readNoEof())
            num |= 1;
    }
    return num;
}
