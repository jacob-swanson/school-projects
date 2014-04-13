// WaveFilter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>

#include "Filter.h"

using namespace std;

int* BLPoint;
double* BPoint;

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

void xiir_filter(short* x, short* y, short sample)
{
	// Shift samples right
	for (int i = (*BLPoint) - 1; i > 0; i--) {
		x[i] = x[i - 1];
		y[i] = y[i - 1];
	}
	x[0] = sample;
	y[0] = sample;

	// Apply the FIR filter
	short temp = 0;
	for (int i = 0; i < (*BLPoint); i++) {
		temp += (short)((double)x[i] * BPoint[i]);
	}
	y[0] = temp;
}

int checkHeader(wave_header header)
{
	// Error checks
	if (string(header.ChunkID, 4).compare("RIFF")) {
		cerr << "ChunkID was \"" << string(header.ChunkID, 4) << "\" not \"RIFF\"" << endl;
		return 1;
	} else if (string(header.Format, 4).compare("WAVE")) {
		cerr << "Format was \"" << string(header.Format, 4) << "\" not \"WAVE\"" << endl;
		return 2;
	} else if (string(header.Subchunk1ID, 4).compare("fmt ")) {
		cerr << "Subchunk1ID was \"" << string(header.Subchunk1ID, 4) << "\" not \"fmt \"" << endl;
		return 3;
	} else if (header.AudioFormat != 1) {
		cerr << "AudioFormat was \"" << header.AudioFormat << "\" not \"1\"" << endl;
		return 4;
	} else if (string(header.Subchunk2ID, 4).compare("data")) {
		cerr << "Subchunk2ID was \"" << string(header.Subchunk2ID, 4) << "\" not \"data\"" << endl;
		return 5;
	} else if (header.BitsPerSample != 16) {
		cerr << "BitsPerSample was \"" << header.BitsPerSample << "\" not 16" << endl;
		return 6;
	} else if (!(header.SampleRate == 44100 || header.SampleRate == 22000)) {
		cerr << "Sample rate was \"" << header.SampleRate << "\" not 44100 or 22000" << endl;
		return 7;
	} else {
		return 0;
	}
}

int main(int argc, char* argv[])
{
	// Check input arguments
	if (argc != 3) {
		cerr << "Incorrect number of arguments." << endl;
		cerr << "Usage: " << argv[0] << " <source file> <target file>" << endl;
		return 1;
	}

	// Open the input file
	ifstream inputFile(argv[1], ios::in | ios::binary | ios::ate);
	if (!inputFile.is_open()) {
		cerr << "Could not open input file." << endl;
		return 1;
	}

	// Open the output file
	ofstream outputFile(argv[2], ios::out | ios::binary | ios::trunc);
	if (!outputFile.is_open()) {
		cerr << "Could not open output file." << endl;
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

	// Write the new header
	outputFile.seekp(0, ios::beg);
	outputFile.write((char*)&header, sizeof(header));

	// Calculate the number of sample sets
	unsigned int numSamples = (header.Subchunk2Size / 2) / header.NumChannels;

	// Initialize the filter
	short** x = new short*[header.NumChannels];
	short** y = new short*[header.NumChannels];

	if (header.SampleRate == 44100) {
		BLPoint = &BL;
		BPoint = B;
	} else {
		BLPoint = &BL2;
		BPoint = B2;
	}

	for (int i = 0; i < header.NumChannels; i++) {
		x[i] = new short[(*BLPoint)];
		y[i] = new short[(*BLPoint)];

		for (int j = 0; j < (*BLPoint); j++) {
			x[i][j] = 0;
			y[i][j] = 0;
		}
	}

	// Seek to the beginning of the samples
	inputFile.seekg(44, ios::beg);
	outputFile.seekp(44, ios::beg);

	// Mark the start time
	clock_t startTime = clock();

	// Do the filtering
	short* sampleSet = new short[header.NumChannels];
	for (unsigned int i = 0; i < numSamples; i++) {
		// Read in a set of samples
		inputFile.read((char*)sampleSet, header.NumChannels * 2);

		// Process each channel that was read
		for (int j = 0; j < header.NumChannels; j++) {
			xiir_filter(x[j], y[j], sampleSet[j]);
			sampleSet[j] = y[j][0];
		}

		// Write out the sample set
		outputFile.write((char*)sampleSet, header.NumChannels * 2);
	}
	
	// Delete the sample set
	delete[] sampleSet;

	// Mark the end time
	clock_t endTime = clock();

	// Delete the filter
	for (int i = 0; i < header.NumChannels; i++) {
		delete [] x[i];
		delete [] y[i];
	}
	delete [] x;
	delete [] y;

	// Output results
	cout << "Filtering time: " << setprecision(4) << fixed << (float)(endTime - startTime) / CLOCKS_PER_SEC << " seconds" << endl;

	// Close the files
	inputFile.close();
	outputFile.close();

	return 0;
}

