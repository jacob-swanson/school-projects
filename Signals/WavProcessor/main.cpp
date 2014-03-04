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
        this->samplesWritten = 0;
        this->numSamples = 0;
    }

    wave_header header;

    unsigned int samplesRead;
    unsigned int samplesWritten;
    unsigned int numSamples;

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
    void printHeader()
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

    /**
     * @brief writeHeader Write the header out to a file
     * @param file
     */
    void writeHeader(ofstream &file)
    {
        // Write out the header information
        file.seekp(0, ios::beg);

        file.write((char*)&this->header, sizeof(this->header));
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
    WaveFile inputWave;
    if (inputFile.is_open())
	{
        // Read header data
        inputWave.read(inputFile);

        // Print out the header
        inputWave.printHeader();

        // Check the header values to make sure they conform to what is expected
        int error = inputWave.check();
        if (error > 0)
        {
            // Exit the program if the check failed
            return error;
        }

        // Open the output file, disreguarding the contents of it
        ofstream outFile(argv[2], ios::out | ios::binary | ios::trunc);
        if (outFile.is_open())
        {

            // Rewrite the header
            inputWave.writeHeader(outFile);

            // Find maximum amplitude
            cout << "Finding maximum amplitude..." << endl;
            short maximumSample = inputWave.getMaxSample(inputFile);

            // Output the amplitude of the sine wave that will be added
            cout << "Sine Wave Amplitude: " << maximumSample/2 << endl;

            // Add sine wave
            // Iteration number is number of samples / number of channels
            cout << "Adding sine wave..." << endl;
            unsigned int iterationNumber = 0;
            while (inputWave.hasMoreSamples())
            {
                // Expicit casting to double for time calculation
                double t = ((double)iterationNumber) / ((double)inputWave.header.SampleRate);

                // Calculation of sine wave sample, should not overflow
                // Half of maximum amplitude * sine wave at 2500 Hz
                short sineWave = (maximumSample/2) * sin(2.0*3.14*2500.0*t);

                // Loop through the channels
                for (int i = 0; i < inputWave.header.NumChannels; i++)
                {
                    // Read new sample in
                    short sample = inputWave.getNextSample(inputFile);

                    // Calculation of new sample, can overflow
                    short newSample = addOverflow(sineWave, sample);

                    // Write new sample to output file
                    inputWave.writeSample(newSample, outFile);
                }
                iterationNumber++;
            }

            // Close the files
            inputFile.close();
            outFile.close();
        }
        else
        {
            // Could not open output file
            cerr << "Unable to open: " << argv[2] << endl;
            return 1;
        }
	}
	else
	{
        // Could not open input file
        cerr << "Unable to open: " << argv[1] << "." << endl;
        return 1;
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
        summaryFile << "Sampling Frequency (Hz): " << inputWave.header.SampleRate << endl;
        summaryFile << "Length (s): " << (inputWave.numSamples / inputWave.header.NumChannels) / inputWave.header.SampleRate << endl;
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

