/*
 * Reference arithmetic coding
 * Copyright (c) Project Nayuki
 *
 * https://www.nayuki.io/page/reference-arithmetic-coding
 * https://github.com/nayuki/Reference-arithmetic-coding
 */

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <stdexcept>
#include <math.h>
#include "include\ArithmeticCoder.hpp"

using std::uint64_t;
using std::uint64_t;


ArithmeticCoderBase::ArithmeticCoderBase(int numBits) {
	if (numBits < 1 || numBits > 63)
		throw std::domain_error("State size out of range");
	numStateBits = numBits;
	fullRange = static_cast<decltype(fullRange)>(1) << numStateBits;
	halfRange = fullRange >> 1;  // Non-zero
	quarterRange = halfRange >> 1;  // Can be zero
	minimumRange = quarterRange + 2;  // At least 2
	maximumTotal = std::min(std::numeric_limits<decltype(fullRange)>::max() / fullRange, minimumRange);
	stateMask = fullRange - 1;
	low = 0;
	high = stateMask;
}


ArithmeticCoderBase::~ArithmeticCoderBase() {}


void ArithmeticCoderBase::update(const FrequencyTable &freqs, uint64_t symbol) {
	// State check
	if (low >= high || (low & stateMask) != low || (high & stateMask) != high)
		throw std::logic_error("Assertion error: Low or high out of range");
	uint64_t range = high - low + 1;
	if (range < minimumRange || range > fullRange)
		throw std::logic_error("Assertion error: Range out of range");

	// Frequency table values check
	uint64_t total = freqs.getTotal();
	uint64_t symLow = freqs.getLow(symbol);
	uint64_t symHigh = freqs.getHigh(symbol);
	if (symLow == symHigh)
		throw std::invalid_argument("Symbol has zero frequency");
	if (total > maximumTotal) {
		std::cout << "Total= " << total << " maximum=" << maximumTotal << ".\nCannot code symbol because total is too large" << std::endl;
		throw std::invalid_argument("Cannot code symbol because total is too large");
	}

	// Update range
	uint64_t newLow  = low + symLow  * range / total;
	uint64_t newHigh = low + symHigh * range / total - 1;
	low = newLow;
	high = newHigh;

	// While low and high have the same top bit value, shift them out
	while (((low ^ high) & halfRange) == 0) {
		shift();
		low  = ((low  << 1) & stateMask);
		high = ((high << 1) & stateMask) | 1;
	}
	// Now low's top bit must be 0 and high's top bit must be 1

	// While low's top two bits are 01 and high's are 10, delete the second highest bit of both
	while ((low & ~high & quarterRange) != 0) {
		underflow();
		low = (low << 1) ^ halfRange;
		high = ((high ^ halfRange) << 1) | halfRange | 1;
	}
}


ArithmeticDecoder::ArithmeticDecoder(int numBits, BitInputStream &in) :
		ArithmeticCoderBase(numBits),
		input(in),
		code(0) {
	//for (int i = 0; i < numStateBits; i++)
	//	code = code << 1 | readCodeBit();
}

void ArithmeticDecoder::init()
{
    for (int i = 0; i < numStateBits; i++)
        code = code << 1 | readCodeBit();
}


uint64_t ArithmeticDecoder::read(const FrequencyTable &freqs) {
	// Translate from coding range scale to frequency table scale
	uint64_t total = freqs.getTotal();
	if (total > maximumTotal)
		throw std::invalid_argument("Cannot decode symbol because total is too large");
	uint64_t range = high - low + 1;
	uint64_t offset = code - low;
	uint64_t value = ((offset + 1) * total - 1) / range;
	if (value * range / total > offset)
		throw std::logic_error("Assertion error");
	if (value >= total)
		throw std::logic_error("Assertion error");

	// A kind of binary search. Find highest symbol such that freqs.getLow(symbol) <= value.
	uint64_t start = 0;
	uint64_t end = freqs.getSymbolLimit();
	while (end - start > 1) {
		uint64_t middle = (start + end) >> 1;
		if (freqs.getLow(middle) > value)
			end = middle;
		else
			start = middle;
	}
	if (start + 1 != end)
		throw std::logic_error("Assertion error");

	uint64_t symbol = start;
	if (offset < freqs.getLow(symbol) * range / total || freqs.getHigh(symbol) * range / total <= offset)
		throw std::logic_error("Assertion error");
	update(freqs, symbol);
	if (code < low || code > high)
		throw std::logic_error("Assertion error: Code out of range");
	return symbol;
}


void ArithmeticDecoder::shift() {
	code = ((code << 1) & stateMask) | readCodeBit();
}


void ArithmeticDecoder::underflow() {
	code = (code & halfRange) | ((code << 1) & (stateMask >> 1)) | readCodeBit();
}


int ArithmeticDecoder::readCodeBit() {
	int temp = input.read();
	if (temp == -1)
		temp = 0;
	return temp;
}


ArithmeticEncoder::ArithmeticEncoder(int numBits, BitOutputStream &out) :
	ArithmeticCoderBase(numBits),
	output(out),
	numUnderflow(0) {}


void ArithmeticEncoder::write(const FrequencyTable &freqs, uint64_t symbol) {
	update(freqs, symbol);
}


void ArithmeticEncoder::finish() {
	output.write(1);
}


void ArithmeticEncoder::shift() {
	int bit = static_cast<int>(low >> (numStateBits - 1));
	output.write(bit);

	// Write out the saved underflow bits
	for (; numUnderflow > 0; numUnderflow--)
		output.write(bit ^ 1);
}


void ArithmeticEncoder::underflow() {
	if (numUnderflow == std::numeric_limits<decltype(numUnderflow)>::max())
		throw std::overflow_error("Maximum underflow reached");
	numUnderflow++;
}

void ArithmeticEncoder::encodeCdelta(uint64_t x)
{
	uint64_t num = x;
	uint64_t len = 0;
	uint64_t lengthOfLen = 0;

	len = 1 + floor(log2(num));  // calculate 1+floor(log2(num))
	lengthOfLen = 1 + floor(log2(len)); // calculate floor(log2(len))

	for (uint64_t i = lengthOfLen - 1; i > 0; --i)
	{
		output.write(0);
		//std::cerr << 0;
	}
	for (uint64_t i = lengthOfLen - 1; i + 1 >= 1; --i)
	{
		output.write((len >> i) & 1);
		//std::cerr << ((len >> i) & 1);
	}
	//for (int i = len - 2; i >= 0; i--)
	for (uint64_t i = len - 2; i + 1 >= 1; i--)
	{
		output.write((num >> i) & 1);
		//std::cerr << ((num >> i) & 1);
	}
}

uint64_t ArithmeticDecoder::decodeCdelta()
{
	uint64_t num = 1;
	uint64_t len = 1;
	uint64_t lengthOfLen = 0;
	while (!input.readNoEof())     // potentially dangerous with malformed files.
		lengthOfLen++;
	for (uint64_t i = 0; i < lengthOfLen; i++)
	{
		len <<= 1;
		if (input.readNoEof())
			len |= 1;
	}
	for (uint64_t i = 0; i < len - 1; i++)
	{
		num <<= 1;
		if (input.readNoEof())
			num |= 1;
	}
	return num;
}

void ArithmeticEncoder::writeBit(uint64_t bit) {
	if (bit)
		output.write(1);
	else
		output.write(0);
}

uint64_t ArithmeticDecoder::readBit() {
	if (input.readNoEof())
		return 1;
	return 0;
}
