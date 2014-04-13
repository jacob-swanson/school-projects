// WaveAnalysis.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <ctime>
#include <iomanip>


using namespace std;

#define FFT_LEN 1024

struct wave_header {
	char ChunkID[4];
	unsigned int ChunkSize;
	char Format[4];
	char Subchunk1ID[4];
	unsigned int Subchunk1Size;
	unsigned short AudioFormat;
	unsigned short NumChannels;
	unsigned int SampleRate;
	unsigned int ByteRate;
	unsigned short BlockAlign;
	unsigned short BitsPerSample;
	char Subchunk2ID[4];
	unsigned int Subchunk2Size;
};

int checkHeader(wave_header header)
{
	// Error checks
	if (string(header.ChunkID, 4).compare("RIFF")) {
		cerr << "ChunkID was \"" << string(header.ChunkID, 4) << "\" not \"RIFF\"" << endl;
		return 1;
	}
	else if (string(header.Format, 4).compare("WAVE")) {
		cerr << "Format was \"" << string(header.Format, 4) << "\" not \"WAVE\"" << endl;
		return 2;
	}
	else if (string(header.Subchunk1ID, 4).compare("fmt ")) {
		cerr << "Subchunk1ID was \"" << string(header.Subchunk1ID, 4) << "\" not \"fmt \"" << endl;
		return 3;
	}
	else if (header.AudioFormat != 1) {
		cerr << "AudioFormat was \"" << header.AudioFormat << "\" not \"1\"" << endl;
		return 4;
	}
	else if (string(header.Subchunk2ID, 4).compare("data")) {
		cerr << "Subchunk2ID was \"" << string(header.Subchunk2ID, 4) << "\" not \"data\"" << endl;
		return 5;
	}
	else if (header.BitsPerSample != 16) {
		cerr << "BitsPerSample was \"" << header.BitsPerSample << "\" not 16" << endl;
		return 6;
	}
	else if (!(header.SampleRate == 44100 || header.SampleRate == 22000)) {
		cerr << "Sample rate was \"" << header.SampleRate << "\" not 44100 or 22000" << endl;
		return 7;
	}
	else {
		return 0;
	}
}

// Must provide type or MSVC++ barfs with "ambiguous call to overloaded function"
double pi = 4 * atan(1.0);

void fft(int sign, vector<complex<double>> &zs) {
	unsigned int j = 0;
	// Warning about signed vs unsigned comparison
	for (unsigned int i = 0; i<zs.size() - 1; ++i) {
		if (i < j) {
			auto t = zs.at(i);
			zs.at(i) = zs.at(j);
			zs.at(j) = t;
		}
		int m = zs.size() / 2;
		j ^= m;
		while ((j & m) == 0) { m /= 2; j ^= m; }
	}
	for (unsigned int j = 1; j<zs.size(); j *= 2)
	for (unsigned int m = 0; m<j; ++m) {
		auto t = pi * sign * m / j;
		auto w = complex<double>(cos(t), sin(t));
		for (unsigned int i = m; i<zs.size(); i += 2 * j) {
			complex<double> zi = zs.at(i), t = w * zs.at(i + j);
			zs.at(i) = zi + t;
			zs.at(i + j) = zi - t;
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Check input arguments
	if (argc != 2) {
		cerr << "Incorrect number of arguments." << endl;
		cerr << "Usage: " << argv[0] << " <source file>" << endl;
		return 1;
	}

	// Open the input file
	ifstream inputFile(argv[1], ios::in | ios::binary | ios::ate);
	if (!inputFile.is_open()) {
		cerr << "Could not open input file." << endl;
		return 1;
	}

	// Read the header information
	wave_header header;
	inputFile.seekg(0, ios::beg);
	inputFile.read((char*)&header, sizeof(header));

	// Check the header info
	int error = checkHeader(header);
	if (error > 0) {
		return error;
	}

	// Calculate the number of sample sets
	unsigned int numSamples = (header.Subchunk2Size / 2) / header.NumChannels;

	// Mark the start time
	clock_t startTime = clock();

	// Initialize fftBuff
	short* sampleSet = new short[header.NumChannels];
	vector<complex<double>> fftBuff;
	int fftCount = 0;
	for (unsigned int i = 0; i < FFT_LEN; i++) {
		// Read in a set of samples
		inputFile.read((char*)sampleSet, header.NumChannels * 2);

		fftBuff.push_back(sampleSet[0]);
		fftCount++;
	}

	// Perform FFT
	fft(1, fftBuff);

	// Analyse the FFT
	double maxSpec = (fftBuff.at(0).real())*(fftBuff.at(0).real()) + (fftBuff.at(0).imag())*(fftBuff.at(0).imag());
	double tmp = 0;
	int maxIndex = 0;

	for (int j = 1; j < FFT_LEN; j++) {
		tmp = (fftBuff.at(j).real())*(fftBuff.at(j).real()) + (fftBuff.at(j).imag())*(fftBuff.at(j).imag());
		if (tmp > maxSpec) {
			maxSpec = tmp;
			maxIndex = j;
		}
	}

	// Mark the end time
	clock_t endTime = clock();

	// Report results
	cout << "Analysing time: " << setprecision(12) << fixed << (float)(endTime - startTime) / CLOCKS_PER_SEC << " seconds" << endl;
	cout << "Frequency: " << maxIndex * header.SampleRate / FFT_LEN << " Hz" << endl;

	// Delete the sample set
	delete[] sampleSet;

	inputFile.close();

	return 0;
}

