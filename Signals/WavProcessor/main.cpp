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
#include <math.h>
#include <cmath>
#include <climits>

using namespace std;

// Wave file header structure
// https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
class WaveFile {
public:
    WaveFile() {
        ChunkID = new char[4];
        Format = new char[4];
        Subchunk1ID = new char[4];
        Subchunk2ID = new char[4];
        this->samplesRead = 0;
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

    int samplesRead;
    int numSamples;

    /**
     * @brief readWaveHeader Read in the header of a WAVE file
     * @param file ifstream
     */
    void read(ifstream &file)
    {
        // Seek to the beginning of the file
        file.seekg(0, ios::beg);

        // Read header information
        file.read(this->ChunkID, 4);
        file.read((char*)&this->ChunkSize, sizeof(this->ChunkSize));
        file.read(this->Format, 4);
        file.read(this->Subchunk1ID, 4);
        file.read((char*)&this->Subchunk1Size, sizeof(this->Subchunk1Size));
        file.read((char*)&this->AudioFormat, sizeof(this->AudioFormat));
        file.read((char*)&this->NumChannels, sizeof(this->NumChannels));
        file.read((char*)&this->SampleRate, sizeof(this->SampleRate));
        file.read((char*)&this->ByteRate, sizeof(this->ByteRate));
        file.read((char*)&this->BlockAlign, sizeof(this->BlockAlign));
        file.read((char*)&this->BitsPerSample, sizeof(this->BitsPerSample));
        file.read(this->Subchunk2ID, 4);
        file.read((char*)&this->Subchunk2Size, sizeof(this->Subchunk2Size));

        this->numSamples = this->Subchunk2Size / 2;
    }

    /**
     * @brief checkHeaderInfo Check header values to confirm that they are as expected
     * @param header Header to check
     * @return Error code
     */
    int check()
    {
        // Error checks
        if (strcmp(this->ChunkID, "RIFF"))
        {
            cerr << "ChunkID was \"" << this->ChunkID << "\" not \"RIFF\"" << endl;
            return 1;
        }
        else if (strcmp(this->Format, "WAVE"))
        {
            cerr << "Format was \"" << this->Format << "\" not \"WAVE\"" << endl;
            return 2;
        }
        else if (strcmp(this->Subchunk1ID, "fmt "))
        {
            cerr << "Subchunk1ID was \"" << this->Subchunk1ID << "\" not \"fmt \"" << endl;
            return 3;
        }
        else if (this->AudioFormat != 1)
        {
            cerr << "AudioFormat was \"" << this->AudioFormat << "\" not \"1\"" << endl;
            return 4;
        }
        else if (strcmp(this->Subchunk2ID, "data"))
        {
            cerr << "Subchunk2ID was \"" << this->Subchunk2ID << "\" not \"data\"" << endl;
            return 5;
        }
        else if (this->NumChannels > 2 && this->NumChannels > 0)
        {
            cerr << "NumChannels was not 1 or 2" << endl;
            return 6;
        }
        else if (this->BitsPerSample != 16)
        {
            cerr << "BitsPerSample was \"" << this->BitsPerSample << "\" not 16" << endl;
            return 7;
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
        cout << "ChunkID: " << this->ChunkID << endl;
        cout << "ChunkSize: " << this->ChunkSize << endl;
        cout << "Format: " << this->Format << endl;
        cout << "Subchunk1ID: " << this->Subchunk1ID << endl;
        cout << "Subchunk1Size: " << this->Subchunk1Size << endl;
        cout << "AudioFormat: " << this->AudioFormat << endl;
        cout << "NumChannels: " << this->NumChannels << endl;
        cout << "SampleRate: " << this->SampleRate << endl;
        cout << "ByteRate: " << this->ByteRate << endl;
        cout << "BlockAlign: " << this->BlockAlign << endl;
        cout << "BitsPerSample: " << this->BitsPerSample << endl;
        cout << "Subchunk2ID: " << this->Subchunk2ID << endl;
        cout << "Subchunk2Size: " << this->Subchunk2Size << endl;
        cout << "==========================" << endl;
    }

    /**
     * @brief writeHeader Write the header out to a file
     * @param file
     */
    void writeHeader(ofstream &file)
    {
        file.seekp(0, ios::beg);
        file.write(this->ChunkID, 4);
        file.write((char*)&this->ChunkSize, sizeof(this->ChunkSize));
        file.write(this->Format, 4);
        file.write(this->Subchunk1ID, 4);
        file.write((char*)&this->Subchunk1Size, sizeof(this->Subchunk1Size));
        file.write((char*)&this->AudioFormat, sizeof(this->AudioFormat));
        file.write((char*)&this->NumChannels, sizeof(this->NumChannels));
        file.write((char*)&this->SampleRate, sizeof(this->SampleRate));
        file.write((char*)&this->ByteRate, sizeof(this->ByteRate));
        file.write((char*)&this->BlockAlign, sizeof(this->BlockAlign));
        file.write((char*)&this->BitsPerSample, sizeof(this->BitsPerSample));
        file.write(this->Subchunk2ID, 4);
        file.write((char*)&this->Subchunk2Size, sizeof(this->Subchunk2Size));
    }

    /**
     * @brief writeSample Write a sample out to a file
     * @param sample
     * @param file
     */
    void writeSample(short sample, ofstream &file)
    {
        file.write((char*)&sample, sizeof(sample));
    }

};

/**
 * @brief addOverflow Add two shorts without overflowing
 * @param a
 * @param b
 * @return
 */
short addOverflow(short a, short b)
{
    if (a > SHRT_MAX - b)
    {
        return SHRT_MAX;
    }
    else
    {
        return a + b;
    }
}

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

        // Check the header values to make sure they conform to what is expected
        int error = inputWave.check();
        if (error > 0)
        {
            return error;
        }

        // Print out the header
        inputWave.print();

        ofstream outFile(argv[2], ios::out | ios::binary | ios::ate);
        inputWave.writeHeader(outFile);
        short maximumSample = 0;

        // Find maximum amplitude
        if (inputWave.NumChannels == 1)
        {
            // Process mono to find the maximum sample
            short sample;
            while (inputWave.getNextSample(sample, inputFile))
            {
                if (abs(sample) > maximumSample)
                {
                    maximumSample = abs(sample);
                }
            }
        }
        else
        {
            // Process stereo to find the maximum sample
            short leftSample, rightSample;
            while (inputWave.getNextSample(leftSample, inputFile) && inputWave.getNextSample(rightSample, inputFile))
            {
                if (abs(leftSample) > maximumSample)
                {
                    maximumSample = abs(leftSample);
                }
                else if (abs(rightSample) > maximumSample)
                {
                    maximumSample = abs(rightSample);
                }
            }
        }

        cout << "Maximum Sample: " << maximumSample << endl;
        cout << "Sine Wave Amplitude: " << maximumSample/2 << endl;

        inputWave.samplesRead = 0;

        // Add sine wave
        if (inputWave.NumChannels == 1)
        {
            // Process mono
            short sample;
            while (inputWave.getNextSample(sample, inputFile))
            {
                inputWave.writeSample(sample, outFile);
            }
        }
        else
        {
            // Process stereo
            short leftSample, rightSample;
            int sampleNumber = 0;

            while (inputWave.getNextSample(leftSample, inputFile) && inputWave.getNextSample(rightSample, inputFile))
            {
                // Expicit casting to double
                double t = ((double)sampleNumber) / ((double)inputWave.SampleRate);

                // Should not overflow
                short sineWave = (maximumSample/2) * sin(2.0*3.14*2500.0*t);

                // Can and will overflow
                short newLeftSample = addOverflow(sineWave, leftSample);
                short newRightSample = addOverflow(sineWave, rightSample);

                // Write the samples
                inputWave.writeSample(newLeftSample, outFile);
                inputWave.writeSample(newRightSample, outFile);

                sampleNumber++;
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

