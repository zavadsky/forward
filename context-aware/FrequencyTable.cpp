/*
 * Reference arithmetic coding
 * Copyright (c) Project Nayuki
 *
 * https://www.nayuki.io/page/reference-arithmetic-coding
 * https://github.com/nayuki/Reference-arithmetic-coding
 */

#include <stdexcept>
#include <math.h>
#include "include\FrequencyTable.hpp"

using std::uint64_t;


FrequencyTable::~FrequencyTable() {}

FlatFrequencyTable::FlatFrequencyTable(uint64_t numSyms, uint64_t unit) :
	numSymbols(numSyms), one_unit(unit) {
	if (numSyms < 1)
		throw std::domain_error("Number of symbols must be positive");
}

FlatFrequencyTable::FlatFrequencyTable(uint64_t numSyms) :
		numSymbols(numSyms) {
	if (numSyms < 1)
		throw std::domain_error("Number of symbols must be positive");
	one_unit = 1;
}


uint64_t FlatFrequencyTable::getSymbolLimit() const {
	return numSymbols;
}


uint64_t FlatFrequencyTable::get(uint64_t symbol) const  {
	checkSymbol(symbol);
	return one_unit;
}


uint64_t FlatFrequencyTable::getTotal() const  {
	return numSymbols * one_unit;
}


uint64_t FlatFrequencyTable::getLow(uint64_t symbol) const  {
	checkSymbol(symbol);
	return symbol * one_unit;
}


uint64_t FlatFrequencyTable::getHigh(uint64_t symbol) const  {
	checkSymbol(symbol);
	return (symbol + 1) * one_unit;
}


void FlatFrequencyTable::set(uint64_t, uint64_t)  {
	throw std::logic_error("Unsupported operation");
}


void FlatFrequencyTable::increment(uint64_t) {
	throw std::logic_error("Unsupported operation");
}

void FlatFrequencyTable::increment(uint64_t, uint64_t) {
    throw std::logic_error("Unsupported operation");
}

void FlatFrequencyTable::decrement(uint64_t) {
    throw std::logic_error("Unsupported operation");
}

void FlatFrequencyTable::decrement(uint64_t, uint64_t) {
    throw std::logic_error("Unsupported operation");
}


void FlatFrequencyTable::checkSymbol(uint64_t symbol) const {
	if (symbol >= numSymbols)
		throw std::domain_error("Symbol out of range");
}

double FlatFrequencyTable::probability(uint64_t symbol) const {
	checkSymbol(symbol);
	return 1.0 / numSymbols;
}


SimpleFrequencyTable::SimpleFrequencyTable(const std::vector<uint64_t> &freqs) {
	if (freqs.size() > UINT64_MAX - 1)
		throw std::length_error("Too many symbols");
	uint64_t size = static_cast<uint64_t>(freqs.size());
	if (size < 1)
		throw std::invalid_argument("At least 1 symbol needed");

	frequencies = freqs;
	cumulative.reserve(size + 1);
	initCumulative(false);
	total = getHigh(size - 1);
}


SimpleFrequencyTable::SimpleFrequencyTable(const FrequencyTable &freqs) {
	uint64_t size = freqs.getSymbolLimit();
	if (size < 1)
		throw std::invalid_argument("At least 1 symbol needed");
	if (size > UINT64_MAX - 1)
		throw std::length_error("Too many symbols");

	frequencies.reserve(size + 1);
	for (uint64_t i = 0; i < size; i++)
		frequencies.push_back(freqs.get(i));

	cumulative.reserve(size + 1);
	initCumulative(false);
	total = getHigh(size - 1);
}

SimpleFrequencyTable::SimpleFrequencyTable(const std::vector<bool>& exists) {
	uint64_t size = exists.size();
	if (size < 1)
		throw std::invalid_argument("At least 1 symbol needed");
	if (size > UINT64_MAX - 1)
		throw std::length_error("Too many symbols");

	frequencies.reserve(size + 1);
	for (uint64_t i = 0; i < size; i++)
		frequencies.push_back(exists.at(i) ? 1 : 0);

	cumulative.reserve(size + 1);
	initCumulative(false);
	total = getHigh(size - 1);
}


uint64_t SimpleFrequencyTable::getSymbolLimit() const {
	return static_cast<uint64_t>(frequencies.size());
}


uint64_t SimpleFrequencyTable::get(uint64_t symbol) const {
	return frequencies.at(symbol);
}


void SimpleFrequencyTable::set(uint64_t symbol, uint64_t freq) {
	if (total < frequencies.at(symbol))
		throw std::logic_error("Assertion error");
	uint64_t temp = total - frequencies.at(symbol);
	total = checkedAdd(temp, freq);
	frequencies.at(symbol) = freq;
	cumulative.clear();
}


void SimpleFrequencyTable::increment(uint64_t symbol) {
	if (frequencies.at(symbol) == UINT64_MAX)
		throw std::overflow_error("Arithmetic overflow");
	total = checkedAdd(total, 1);
	frequencies.at(symbol)++;
	cumulative.clear();
}

void SimpleFrequencyTable::increment(uint64_t symbol, std::uint64_t x) {
    if (UINT64_MAX - frequencies.at(symbol) < x)
        throw std::overflow_error("Arithmetic overflow");
    total = checkedAdd(total, x);
    frequencies.at(symbol) += x;
    cumulative.clear();
}


void SimpleFrequencyTable::decrement(uint64_t symbol) {
    if (frequencies.at(symbol) == 0)
        throw std::overflow_error("Arithmetic overflow");
    total = checkedReduction(total, 1);
    frequencies.at(symbol)--;
    cumulative.clear();
}

void SimpleFrequencyTable::decrement(uint64_t symbol, std::uint64_t x) {
    if (frequencies.at(symbol) < x)
        throw std::overflow_error("Arithmetic overflow");
    total = checkedReduction(total, x);
    frequencies.at(symbol) -= x;
    cumulative.clear();
}

void SimpleFrequencyTable::divAllFreqs()
{
	uint64_t new_total = 0;
	for (uint64_t i = 0; i < frequencies.size(); i++) {
		uint64_t freq = get(i);
		freq = (1 + freq) / 2;
		new_total += freq;
		set(i, freq);
	}
	total = new_total;
}

void SimpleFrequencyTable::divAllFreqs(double div)
{
	int new_total = 0;
	for (uint64_t i = 0; i < frequencies.size(); i++) {
		uint64_t freq = get(i);
		freq = ceil(freq / div);
		new_total += freq;
		set(i, freq);
	}
	total = new_total;
}

uint64_t SimpleFrequencyTable::getTotal() const {
	return total;
}


uint64_t SimpleFrequencyTable::getLow(uint64_t symbol) const {
	initCumulative();
	return cumulative.at(symbol);
}


uint64_t SimpleFrequencyTable::getHigh(uint64_t symbol) const {
	initCumulative();
	return cumulative.at(symbol + 1);
}


void SimpleFrequencyTable::initCumulative(bool checkTotal) const {
	if (!cumulative.empty())
		return;
	uint64_t sum = 0;
	cumulative.push_back(sum);
	for (uint64_t freq : frequencies) {
		// This arithmetic should not throw an exception, because invariants are being maintained
		// elsewhere in the data structure. This implementation is just a defensive measure.
		sum = checkedAdd(freq, sum);
		cumulative.push_back(sum);
	}
	if (checkTotal && sum != total)
		throw std::logic_error("Assertion error");
}


uint64_t SimpleFrequencyTable::checkedAdd(uint64_t x, uint64_t y) {
	if (x > UINT64_MAX - y)
		throw std::overflow_error("Arithmetic overflow");
	return x + y;
}


uint64_t SimpleFrequencyTable::checkedReduction(uint64_t x, uint64_t y) {
    if (x - y < 0)
        throw std::overflow_error("Arithmetic overflow");
    return x - y;
}

double SimpleFrequencyTable::probability(uint64_t symbol) const {
	return (double)get(symbol) / getTotal();
}
