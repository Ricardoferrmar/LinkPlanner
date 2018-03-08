# include "fft_20180208.h"

# include <complex>
# include <fstream>
# include <iostream>
# include <math.h>
# include <stdio.h>
# include <string>
# include <strstream>
# include <algorithm>
# include <vector>
#include <iomanip>

using namespace std;

int main()
{	
	//////////////////////////////// Section 1 ///////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	/////////// Read the input text file (import "time_function.txt" ) ///////////
	//////////////////////////////////////////////////////////////////////////////
	ifstream inFile;
	inFile.precision(15);
	double ch;
	vector <double> inTimeDomain;
	inFile.open("time_function.txt");

	// First data (at 0th position) appled to the ch it is similar to the "cin".
	inFile >> ch;

	// It'll count the length of the vector to verify with the MATLAB
	int count=0; 

	while (!inFile.eof())
	{
		// push data one by one into the vector
		inTimeDomain.push_back(ch);	

		// it'll increase the position of the data vector by 1 and read full vector.
		inFile >> ch;

		count++;
	}

	inFile.close();	// It is mandatory to close the file at the end. 


	//////////////////////////////// Section 2 ///////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Calculate FFT ////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	
	vector <complex<double>> inTimeDomainComplex(inTimeDomain.size());
	vector <complex<double>> fourierTransformed;
	vector <double> re(inTimeDomain.size());
	vector <double> im(inTimeDomain.size());

	for (unsigned int i = 0; i < inTimeDomain.size(); i++) 
	{
		// Real data of the signal
		re[i] = inTimeDomain[i]; 

		// Imaginary data of the signal
		im[i] = 0; 
	} 

	// Next, Real and Imaginary vector to complex vector conversion
	inTimeDomainComplex = reImVect2ComplexVector(re, im);

	// calculate FFT
	fourierTransformed = fft(inTimeDomainComplex);
	fourierTransformed = ifft(inTimeDomainComplex);

	//////////////////////////////// Section 3 ///////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////// Save FFT calculated data (export "frequency_function.txt" ) /////////
	//////////////////////////////////////////////////////////////////////////////
	ofstream outFile;
	complex<double> outFileData;
	outFile.open("frequency_function.txt");
	outFile.precision(15);
	for (unsigned int i = 0; i <fourierTransformed.size(); i++)
	{
		outFile <<  fourierTransformed[i].real() << endl;
		outFile << fourierTransformed[i].imag() << endl;
	}


	//////////////////////////////// Section 4 ///////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Display Section //////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	for (unsigned int i = 0; i < fourierTransformed.size(); i++)
	{
		// Display all FFT calculated data
		cout << fourierTransformed[i] << endl;	
	}

	// Display length of data
	cout <<"\nTotal length of of data :"<< count << endl;

	getchar();
	return 0;
}