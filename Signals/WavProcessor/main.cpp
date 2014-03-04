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
#include <string.h>

using namespace std;

struct wave_header {
    char ChunkID[4];
    int ChunkSize;
    char Format[4];
    char Subchunk1ID[4];
    int Subchunk1Size;
    short AudioFormat;
    short NumChannels;
    int SampleRate;
    int ByteRate;
    short BlockAlign;
    short BitsPerSample;
    char Subchunk2ID[4];
    int Subchunk2Size;
};

// Wave file header structure
// https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
class WaveFile {
public:
    WaveFile() {
        this->samplesRead = 0;
    }

    wave_header header;

    int samplesRead;
    int numSamples;

    /**
     * @brief readWaveHeader Read in the header of a WAVE file
     * @param file ifstream
     */
    void read(istream &file)
    {
        // Seek to the beginning of the file
        file.seekg(0, ios::beg);

        // Read header information
        file.read((char*)&this->header, sizeof(this->header));

        this->numSamples = this->header.Subchunk2Size / 2;
    }

    /**
     * @brief checkHeaderInfo Check header values to confirm that they are as expected
     * @param header Header to check
     * @return Error code
     */
    int check()
    {
        // Error checks
        if (string(this->header.ChunkID, 4).compare("RIFF"))
        {
            cerr << "ChunkID was \"" << string(this->header.ChunkID, 4) << "\" not \"RIFF\"" << endl;
            return 1;
        }
        else if (string(this->header.Format, 4).compare("WAVE"))
        {
            cerr << "Format was \"" << string(this->header.Format, 4) << "\" not \"WAVE\"" << endl;
            return 2;
        }
        else if (string(this->header.Subchunk1ID, 4).compare("fmt "))
        {
            cerr << "Subchunk1ID was \"" << string(this->header.Subchunk1ID, 4) << "\" not \"fmt \"" << endl;
            return 3;
        }
        else if (this->header.AudioFormat != 1)
        {
            cerr << "AudioFormat was \"" << this->header.AudioFormat << "\" not \"1\"" << endl;
            return 4;
        }
        else if (string(this->header.Subchunk2ID, 4).compare("data"))
        {
            cerr << "Subchunk2ID was \"" << string(this->header.Subchunk2ID, 4) << "\" not \"data\"" << endl;
            return 5;
        }
        else if (this->header.BitsPerSample != 16)
        {
            cerr << "BitsPerSample was \"" << this->header.BitsPerSample << "\" not 16" << endl;
            return 6;
        }
        else
        {
            return 0;
        }
    }

    /**
     * @brief getNextSample Get  the next sample in the file
     * @param sample
     * @return
     */
    bool getNextSample(short &sample, ifstream &file)
    {
        if (this->samplesRead >= this->numSamples)
        {
            return false;
        }

        // Data offset + size of samples read so far
        int sampleOffset = 44 + (this->samplesRead * 2);

        // Read a short
        file.seekg(sampleOffset, ios::beg);
        file.read((char*)&sample, 2);
        this->samplesRead++;

        return true;
    }

    /**
     * @brief print Print the header information to the console
     */
    void print()
    {
        // Output header information
        cout << "==========================" << endl;
        cout << "=== Header Information ===" << endl;
        cout << "==========================" << endl;
        cout << "ChunkID: "         << string(this->header.ChunkID, 4) << endl;
        cout << "ChunkSize: "       << this->header.ChunkSize << endl;
        cout << "Format: "          << string(this->header.Format, 4) << endl;
        cout << "Subchunk1ID: "     << string(this->header.Subchunk1ID, 4) << endl;
        cout << "Subchunk1Size: "   << this->header.Subchunk1Size << endl;
        cout << "AudioFormat: "     << this->header.AudioFormat << endl;
        cout << "NumChannels: "     << this->header.NumChannels << endl;
        cout << "SampleRate: "      << this->header.SampleRate << endl;
        cout << "ByteRate: "        << this->header.ByteRate << endl;
        cout << "BlockAlign: "      << this->header.BlockAlign << endl;
        cout << "BitsPerSample: "   << this->header.BitsPerSample << endl;
        cout << "Subchunk2ID: "     << string(this->header.Subchunk2ID, 4) << endl;
        cout << "Subchunk2Size: "   << this->header.Subchunk2Size << endl;
        cout << "==========================" << endl;
    }

    void writeHeader(ofstream &file)
    {
        file.seekp(0, ios::beg);

        file.write((char*)&this->header, sizeof(this->header));
    }

    void writeSample(short sample, ofstream &file)
    {
        file.write((char*)&sample, sizeof(sample));
    }

};

int main(int argc, char* argv[])
{
	// Check input arguments
    if (argc != 3) {
		cout << "Incorrect number of arguments." << endl;
        cout << "Usage: " << argv[0] << " <source file> <target file>" << endl;
	}

    ifstream inputFile(argv[1], ios::in | ios::binary | ios::ate);
    if (inputFile.is_open())
	{
        // Read header data
        WaveFile inputWave;
        inputWave.read(inputFile);

        // Print out the header
        inputWave.print();

        // Check the header values to make sure they conform to what is expected
        int error = inputWave.check();
        if (error > 0)
        {
            return error;
        }


        // Copy samples from one file to another
        ofstream outFile(argv[2], ios::out | ios::binary | ios::ate);
        inputWave.writeHeader(outFile);
        if (inputWave.header.NumChannels == 1)
        {
            short sample;
            while (inputWave.getNextSample(sample, inputFile))
            {
                inputWave.writeSample(sample, outFile);
            }
        }
        else
        {
            short leftSample, rightSample;
            while (inputWave.getNextSample(leftSample, inputFile) && inputWave.getNextSample(rightSample, inputFile))
            {
                inputWave.writeSample(leftSample, outFile);
                inputWave.writeSample(rightSample, outFile);
            }
        }

		// Close the file
        inputFile.close();
        outFile.close();
	}
	else
	{
		// Could not open file
        cerr << "Unable to open: " << argv[1] << "." << endl;
	}
	return 0;
}

