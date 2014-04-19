// WaveAnalysis.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <ctime>
#include <iomanip>
#include <climits>


using namespace std;

// Length of the FFT to perform
#define FFT_LEN 2048

// Array that the Window will be multiplied by
vector<double> window;

/**
 * Definition of the WAV header
 */
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

/**
 * Check the wave_header for the expected values
 * @param header WAV header
 */
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
	else 
	{
		return 0;
	}
}

// Must provide type or MSVC++ barfs with "ambiguous call to overloaded function"
double pi = 4 * atan(1.0);

/**
 * Calculate FFT on the buffer
 */
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

/**
 * Load in FFT_LEN samples from the file
 */
int loadBuffer(int numChannels, ifstream &inputFile, vector<complex<double>> &fftBuff)
{
	// Load fftBuff
	short* sampleSet = new short[numChannels];
	int fftCount = 0;
	for (unsigned int i = 0; i < FFT_LEN; i++) {
		// Read in a set of samples
		inputFile.read((char*)sampleSet, numChannels * 2);

		// If no exception occured, add sample
		if (inputFile)
		{
			// Load the first channel, and multiply by the window
			fftBuff.push_back(sampleSet[0] * window[i]);
		}
		else
		{
			// If an exception occured, return
			return fftCount;
		}

		fftCount++;
	}
	delete [] sampleSet;

	// Return the number of sample sets read
	return fftCount;
}

/**
 * Process a window at the given sample rate
 * @param sampleRate Sample rate
 * @param fftBuff Buffer of samples of length FFT_LEN
 * @returns Maximum frequency component in the window
 */
double processWindow(unsigned int sampleRate, vector<complex<double>> &fftBuff)
{
	// Initialize
	double maxSpec = (fftBuff.at(0).real())*(fftBuff.at(0).real()) + (fftBuff.at(0).imag())*(fftBuff.at(0).imag());
	double tmp = 0;
	int maxIndex = 0;

	// Find max value's index
	for (int j = 1; j < FFT_LEN; j++) {
		tmp = (fftBuff.at(j).real())*(fftBuff.at(j).real()) + (fftBuff.at(j).imag())*(fftBuff.at(j).imag());
		if (tmp > maxSpec) {
			maxSpec = tmp;
			maxIndex = j;
		}
	}

	// Return the frequency
	return (double)maxIndex * (double)sampleRate / (double)FFT_LEN;
}

/**
 * Process a WAV file
 * @param numChannels Number of channels in the WAV file
 * @param sampleRate Sample rate of the WAV file
 * @param inputFile WAV file, stream should be at the beginning of the data
 * @returns Maximum frequency component in the spectrum
 */
double processFile(unsigned short numChannels, unsigned int sampleRate, ifstream &inputFile)
{
	// Max frequency found per window
	vector<double> maxFreqs;
	// Keep processing until the inputFile errors
	while (inputFile)
	{
		// Load the buffer from file
		vector<complex<double>> fftBuff;
		int fftCount = loadBuffer(numChannels, inputFile, fftBuff);
		
		// Pad the buffer with 0's if needed, this will happen at the end of the WAV
		if (fftCount < FFT_LEN)
		{
			for (int i = 0; i < FFT_LEN - fftCount; i++)
			{
				fftBuff.push_back(0.0);
			}
		}

		// Checking that loading/padding worked correctly
		if (fftBuff.size() != FFT_LEN)
		{
			cerr << "Error loading fftBuff to FFT length. Was \"" << fftCount << "\" not \"" << FFT_LEN << "\"." << endl;
			return 1;
		}

		// Perform FFT on the window
		fft(1, fftBuff);

		// Analyse the FFT
		double localMaxFreq = processWindow(sampleRate, fftBuff);
		maxFreqs.push_back(localMaxFreq);

		// Shift reading position back FFT_LEN / 2
		streamoff position = inputFile.tellg();
		inputFile.seekg(position - (FFT_LEN / 2));
	}

	// Find highest frequency
	double max = 0.0;
	for (int i = 0; i < maxFreqs.size(); i++)
	{
		// Had an issue where frequencies around the sampleRate were being returned from the fft
		// Sample Rate / 2 should be the maximum expected frequency (Nyquist)
		if (maxFreqs[i] > max && maxFreqs[i] < sampleRate / 2)
			max = maxFreqs[i];
	}
	return max;
}

/**
 * Accepts one command line argument, the file to analyse
 */
int main(int argc, char* argv[])
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

	// Check the header info for errors
	int error = checkHeader(header);
	if (error > 0) {
		return error;
	}

	// Create the window
	for (int i = 0; i < FFT_LEN - 1; i++)
	{
		window.push_back(sin((pi * i) / (FFT_LEN - 1)));
	}
	// This will make sure that it both begins and ends with 0, 
	// otherwise the final value would be a very small number, not 0.
	window.push_back(0.0);

	// Mark the start time
	clock_t startTime = clock();
	
	// Process the file
	double maxFreq = processFile(header.NumChannels, header.SampleRate, inputFile);

	// Mark the end time
	clock_t endTime = clock();

	// Report results
	cout << "Analysing time: " << setprecision(4) << fixed << (float)(endTime - startTime) / CLOCKS_PER_SEC << " seconds" << endl;
	cout << "Frequency: " << maxFreq << " Hz" << endl;

	// Close the file
	inputFile.close();

	return 0;
}

