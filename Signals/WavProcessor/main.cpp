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
#include <ctime>

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
        this->samplesWritten = 0;
        this->numSamples = 0;
    }

    char* ChunkID;
    unsigned int ChunkSize;
    char* Format;
    char* Subchunk1ID;
    unsigned int Subchunk1Size;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short BlockAlign;
    unsigned short BitsPerSample;
    char* Subchunk2ID;
    unsigned int Subchunk2Size;

    unsigned int samplesRead;
    unsigned int samplesWritten;
    unsigned int numSamples;

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

        // Assuming 16 bit samples
        this->numSamples = this->Subchunk2Size / 2;
    }

    /**
     * @brief checkHeaderInfo Check header values to confirm that they are as expected
     * @param header Header to check
     * @return Error code
     */
    int check(ostream &out)
    {
        // Error to see if the header was as expected
        if (strcmp(this->ChunkID, "RIFF"))
        {
            out << "ChunkID was \"" << this->ChunkID << "\" not \"RIFF\"" << endl;
            return 1;
        }
        else if (strcmp(this->Format, "WAVE"))
        {
            out << "Format was \"" << this->Format << "\" not \"WAVE\"" << endl;
            return 2;
        }
        else if (strcmp(this->Subchunk1ID, "fmt "))
        {
            out << "Subchunk1ID was \"" << this->Subchunk1ID << "\" not \"fmt \"" << endl;
            return 3;
        }
        else if (this->AudioFormat != 1)
        {
            out << "AudioFormat was \"" << this->AudioFormat << "\" not \"1\"" << endl;
            return 4;
        }
        else if (strcmp(this->Subchunk2ID, "data"))
        {
            out << "Subchunk2ID was \"" << this->Subchunk2ID << "\" not \"data\"" << endl;
            return 5;
        }
        else if (this->NumChannels > 2 && this->NumChannels > 0)
        {
            out << "NumChannels was not 1 or 2" << endl;
            return 6;
        }
        else if (this->BitsPerSample != 16)
        {
            out << "BitsPerSample was \"" << this->BitsPerSample << "\" not 16" << endl;
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
    short getNextSample(ifstream &file)
    {
        // Data offset + size of samples read so far
        int sampleOffset = 44 + (this->samplesRead * 2);

        // Read sample
        short sample;
        file.seekg(sampleOffset, ios::beg);
        file.read((char*)&sample, 2);

        // Increment number of samples read
        this->samplesRead++;

        // Return the sample
        return sample;
    }

    /**
     * @brief print Print the header information to the console
     */
    void printHeader(ostream &out)
    {
        // Print the header information
        out << "==========================" << endl;
        out << "=== Header Information ===" << endl;
        out << "==========================" << endl;
        out << "ChunkID: " << this->ChunkID << endl;
        out << "ChunkSize: " << this->ChunkSize << endl;
        out << "Format: " << this->Format << endl;
        out << "Subchunk1ID: " << this->Subchunk1ID << endl;
        out << "Subchunk1Size: " << this->Subchunk1Size << endl;
        out << "AudioFormat: " << this->AudioFormat << endl;
        out << "NumChannels: " << this->NumChannels << endl;
        out << "SampleRate: " << this->SampleRate << endl;
        out << "ByteRate: " << this->ByteRate << endl;
        out << "BlockAlign: " << this->BlockAlign << endl;
        out << "BitsPerSample: " << this->BitsPerSample << endl;
        out << "Subchunk2ID: " << this->Subchunk2ID << endl;
        out << "Subchunk2Size: " << this->Subchunk2Size << endl;
        out << "==========================" << endl;
    }

    /**
     * @brief writeHeader Write the header out to a file
     * @param file
     */
    void writeHeader(ofstream &file)
    {
        // Write out the header information
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
        // Calculate sample offset
        int sampleOffset = 44 + (this->samplesWritten * 2);

        // Seek to location of next sample and write
        file.seekp(sampleOffset);
        file.write((char*)&sample, sizeof(sample));

        // Increment number samples written
        this->samplesWritten++;
    }

    /**
     * @brief getMaxSample Get the maximum sample value from the file, multiple channels don't matter
     * @param file
     * @return
     */
    short getMaxSample(ifstream &file)
    {
        // Process samples to find the maximum
        short maximumSample = 0;
        while (this->hasMoreSamples())
        {
            short sample = this->getNextSample(file);
            // |sample| > maximumSample
            if (abs(sample) > maximumSample)
            {
                maximumSample = abs(sample);
            }
        }

        // Reset the reader
        this->resetRead();

        // Return the maximum sample found
        return maximumSample;
    }

    /**
     * @brief resetRead Reset the number of samples read
     */
    void resetRead()
    {
        this->samplesRead = 0;
    }

    /**
     * @brief resetWrite Reset the number of samples written
     */
    void resetWrite()
    {
        this->samplesWritten = 0;
    }

    /**
     * @brief hasMoreSamples Check if wave file has more samples
     * @return
     */
    bool hasMoreSamples()
    {
        // Check if the file has more samples to read
        if (this->samplesRead >= this->numSamples)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
};

/**
 * @brief addOverflow Add two shorts without overflowing
 * @param a
 * @param b
 * @return Either a + b, or SHRT_MAX if a + b would overflow
 */
short addOverflow(short a, short b)
{
    // Check if a is greater than the remainder
    if (a > SHRT_MAX - b)
    {
        // Will overflow, return max value
        return SHRT_MAX;
    }
    else
    {
        // Will not overflow, add the numbers
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

    // Store starting time
    time_t startTime;
    time(&startTime);

    // Open the input file
    ifstream inputFile(argv[1], ios::in | ios::binary | ios::ate);
    WaveFile waveFile;
    if (inputFile.is_open())
	{
        // Read header data
        waveFile.read(inputFile);

        // Check the header values to make sure they conform to what is expected
        int error = waveFile.check(cerr);
        if (error > 0)
        {
            // Exit the program if the check failed
            return error;
        }

        // Print out the header
        waveFile.printHeader(cout);

        // Open the output file, disreguarding the contents of it
        ofstream outFile(argv[2], ios::out | ios::binary | ios::trunc);

        // Rewrite the header
        waveFile.writeHeader(outFile);

        // Find maximum amplitude
        cout << "Finding maximum amplitude..." << endl;
        short maximumSample = waveFile.getMaxSample(inputFile);

        // Output the amplitude of the sine wave that will be added
        cout << "Sine Wave Amplitude: " << maximumSample/2 << endl;

        // Add sine wave
        // Iteration number is number of samples / number of channels
        cout << "Adding sine wave..." << endl;
        unsigned int iterationNumber = 0;
        while (waveFile.hasMoreSamples())
        {
            // Expicit casting to double for time calculation
            double t = ((double)iterationNumber) / ((double)waveFile.SampleRate);

            // Calculation of sine wave sample, should not overflow
            // Half of maximum amplitude * sine wave at 2500 Hz
            short sineWave = (maximumSample/2) * sin(2.0*3.14*2500.0*t);

            // Loop through the channels
            for (int i = 0; i < waveFile.NumChannels; i++)
            {
                // Read new sample in
                short sample = waveFile.getNextSample(inputFile);

                // Calculation of new sample, can overflow
                short newSample = addOverflow(sineWave, sample);

                // Write new sample to output file
                waveFile.writeSample(newSample, outFile);
            }
            iterationNumber++;
        }

        // Close the files
        inputFile.close();
        outFile.close();
	}
	else
	{
		// Could not open file
        cerr << "Unable to open: " << argv[1] << "." << endl;
	}

    cout << "Done." << endl;

    // Store ending time
    time_t endTime;
    time(&endTime);

    // Calculate time taken
    double seconds = difftime(endTime, startTime);
    cout << "Execution Time: " << seconds << " seconds" << endl;

    // Create summary text file
    ofstream summaryFile("Summary.txt", ios::out | ios::trunc);
    if (summaryFile.is_open())
    {
        summaryFile << "Input File: " << argv[1] << endl;
        summaryFile << "Output File: " << argv[2] << endl;
        summaryFile << "Sampling Frequency (Hz): " << waveFile.SampleRate << endl;
        summaryFile << "Length (s): " << (waveFile.numSamples / waveFile.NumChannels) / waveFile.SampleRate << endl;
        summaryFile << "Execution time (s): " << seconds << endl;
        summaryFile.close();
    }
    else
    {
        cerr << "Unable to open summary file." << endl;
        return 1;
    }

	return 0;
}

