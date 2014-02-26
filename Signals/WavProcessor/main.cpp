/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.cpp
 * Copyright (C) 2014 Jacob <jacob@swandiggy.com>
 * 
 * WavProcessor is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * WavProcessor is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>

using namespace std;

// Wave file header structure
// https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
class wav_header {
public:
	wav_header() {
		ChunkID = new char[4];
		Format = new char[4];
		Subchunk1ID = new char[4];
		Subchunk2ID = new char[4];
	}
	char* ChunkID;
	int ChunkSize;
	char* Format;
	char* Subchunk1ID;
	int Subchunk1Size;
	short AudioFormat;
	short NumChannels;
	int SampleRate;
	int ByteRate;
	short BlockAlign;
	short BitsPerSample;
	char* Subchunk2ID;
	int Subchunk2Size;
};

int main(int argc, char* argv[])
{
	// Check input arguments
	if (argc != 2) {
		cout << "Incorrect number of arguments." << endl;
		cout << "Usage: " << argv[0] << " <sound file>" << endl;
	}

	wav_header header;

	ifstream file(argv[1], ios::in | ios::binary | ios::ate);
	if (file.is_open())
	{
		// Seek to the beginning of the file
		file.seekg(0, ios::beg);

		// Read header information
		file.read(header.ChunkID, 4);
		file.read((char*)&header.ChunkSize, sizeof(header.ChunkSize));
		file.read(header.Format, 4);
		file.read(header.Subchunk1ID, 4);
		file.read((char*)&header.Subchunk1Size, sizeof(header.Subchunk1Size));
		file.read((char*)&header.AudioFormat, sizeof(header.AudioFormat));
		file.read((char*)&header.NumChannels, sizeof(header.NumChannels));
		file.read((char*)&header.SampleRate, sizeof(header.SampleRate));
		file.read((char*)&header.ByteRate, sizeof(header.ByteRate));
		file.read((char*)&header.BlockAlign, sizeof(header.BlockAlign));
		file.read((char*)&header.BitsPerSample, sizeof(header.BitsPerSample));
		file.read(header.Subchunk2ID, 4);
		file.read((char*)&header.Subchunk2Size, sizeof(header.Subchunk2Size));

		// Close the file
		file.close();
		
		// Output header information
		cout << "==========================" << endl;
		cout << "=== Header Information ===" << endl;
		cout << "==========================" << endl;
		cout << "ChunkID: " << header.ChunkID << endl;
		cout << "ChunkSize: " << header.ChunkSize << endl;
		cout << "Format: " << header.Format << endl;
		cout << "Subchunk1ID: " << header.Subchunk1ID << endl;
		cout << "Subchunk1Size: " << header.Subchunk1Size << endl;
		cout << "AudioFormat: " << header.AudioFormat << endl;
		cout << "NumChannels: " << header.NumChannels << endl;
		cout << "SampleRate: " << header.SampleRate << endl;
		cout << "ByteRate: " << header.ByteRate << endl;
		cout << "BlockAlign: " << header.BlockAlign << endl;
		cout << "BitsPerSample: " << header.BitsPerSample << endl;
		cout << "Subchunk2ID: " << header.Subchunk2ID << endl;
		cout << "Subchunk2Size: " << header.Subchunk2Size << endl;
		cout << "==========================" << endl;
	}
	else
	{
		// Could not open file
		cout << "Unable to open: " << argv[0] << "." << endl;
	}
	return 0;
}

