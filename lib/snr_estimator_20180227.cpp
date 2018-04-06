#include <algorithm>
#include <complex>
#include "fft_20180208.h"
#include "netxpto_20180118.h"
#include "snr_estimator_20180227.h"

void SNREstimator::initialize(void) {
	firstTime = false;

	outputSignals[0]->setSymbolPeriod(inputSignals[0]->getSymbolPeriod());
	outputSignals[0]->setSamplingPeriod(inputSignals[0]->getSamplingPeriod());
	outputSignals[0]->setFirstValueToBeSaved(inputSignals[0]->getFirstValueToBeSaved());
}

bool SNREstimator::runBlock(void) {

	t_real inValue;
	double samplingPeriod = inputSignals[0]->getSamplingPeriod();
	double symbolPeriod = inputSignals[0]->getSymbolPeriod();
	vector<double> segment;
	vector <double> im(segmentSize);
	vector <complex<double>> segmentComplex(segmentSize);
	vector <complex<double>> fourierTransformed;
	vector <double> allSNR;
	double noisePower = 0;
	double signalPower = 0;
	double SNR = 0;
	double stdSNR = 0;

	// If its the first pass, calculate z for finding the confidence interval.
	// Also, calculate the window to be used and the vector of frequencies.
	if (firstPass) {
		firstPass = false;
		double x1 = -2;
		double x2 = 2;
		double x3 = x2 - (erf(x2 / sqrt(2)) + 1 - alpha)*(x2 - x1) / (erf(x2 / sqrt(2)) - erf(x1 / sqrt(2)));
		double exactness = 1e-15;
		while (abs(erf(x3 / sqrt(2)) + 1 - alpha) > exactness)
		{
			x3 = x2 - (erf(x2 / sqrt(2)) + 1 - alpha)*(x2 - x1) / (erf(x2 / sqrt(2)) - erf(x1 / sqrt(2)));
			x1 = x2;
			x2 = x3;
		}
		z = -x3;

		window = getWindow(windowType, measuredIntervalSize);
		
		U = 0;	// Window normalization constant
		for (unsigned int i = 0; i < window.size(); i++) {
			U += pow(window[i], 2);
		}
		U = U / window.size();
		for (int i = 0; i < segmentSize; i++) {
			frequencies[i] = (1 + i - segmentSize / 2) * 1 / samplingPeriod;
		}
	}

	int ready = inputSignals[0]->ready();
	int space = outputSignals[0]->space();
	int available = min(ready, space);
	int process = min(available, measuredIntervalSize - (int)measuredInterval.size());
	
	
	/* Outputting final report */

	if (available == 0)
	{

		/* Calculating average SNR and bounds */
		if (!allSNR.empty()) {
			for (unsigned int i = 0; i < allSNR.size(); i++) {
				SNR += allSNR[i];
			}
			SNR = 10 * log10(SNR/allSNR.size());
		}
		else {
			cout << "ERROR: SNR could not be calculated. It is probably too low." << "\n";
		}

//		double UpperBound = BER + 1 / sqrt(receivedBits) * z  * sqrt(BER*(1 - BER)) + 1 / (3 * receivedBits)*(2 * z * z * (1 / 2 - BER) + (2 - BER));
//		double LowerBound = BER - 1 / sqrt(receivedBits) * z  * sqrt(BER*(1 - BER)) + 1 / (3 * receivedBits)*(2 * z * z * (1 / 2 - BER) - (1 + BER));

//		if (LowerBound<lowestMinorant) {
//			LowerBound = lowestMinorant;
//		}

		/* Outputting a .txt report*/
		ofstream myfile;
		myfile.open("SNR.txt");
		myfile << "SNR= " << SNR << "\n";
//		myfile << "Upper and lower confidence bounds for " << (1 - alpha) * 100 << "% confidence level \n";
//		myfile << "Upper Bound= " << UpperBound << "\n";
//		myfile << "Lower Bound= " << LowerBound << "\n";
//		myfile << "Number of received bits =" << receivedBits << "\n";
		myfile.close();
		return false;
	}
	
	// Get values until the vector/array has the desired number of elements
	for (long int i = 0; i < process; i++) {
		inputSignals[0]->bufferGet(&inValue);
		outputSignals[0]->bufferPut(inValue);
		measuredInterval.insert(measuredInterval.end(), inValue);
	}
	
	if (measuredInterval.size() == measuredIntervalSize) {

		// Get Welch's PSD estimate
		int start = 0;
		int finish = start + segmentSize-1;
		int summed = 0;
		vector<double> periodogramSum;
		vector<double> periodogramTmp;
		

		while (finish < (int)measuredInterval.size()) {
			// Create segment from full interval
			segment.assign(measuredInterval.begin() + start, measuredInterval.begin() + finish);
			

			// Multiply by window (in time domain)
			for (long int i = 0; i < measuredIntervalSize; i++) {
				segment[i] *= window[i];
			}

			// Create imaginary vector and generate complex segment
			for (unsigned int i = 0; i < segment.size(); i++)
			{
				// Imaginary data of the signal
				im[i] = 0;
			}
			segmentComplex = ReImVect2ComplexVector(segment, im);
			
			// Get the FFT
			fourierTransformed = fft(segmentComplex);
			fourierTransformed = fftshift(fourierTransformed);
			for (unsigned int i = 0; i < fourierTransformed.size(); i++) {
				periodogramTmp[i] = (double)pow(abs(fourierTransformed[i]), 2) / (U*segmentSize);
			}

			// Add to summing vector
			if (periodogramSum.empty()) {
				periodogramSum = periodogramTmp;
			} else {
				for (unsigned int i = 0; i < periodogramTmp.size(); i++) {
					periodogramSum[i] += periodogramTmp[i];
				}
			}
			summed += 1;
			segment.clear();
			periodogramTmp.clear();
			fourierTransformed.clear();
			segmentComplex.clear();
			start += segmentSize;
			finish += segmentSize - overlapCount;
		}

		vector<double> signalPsd;
		vector<double> noisePsd;
		
		// Divide by number of segments used, and separate the signal frequencies from the rest
		for (unsigned int i = 0; i < periodogramTmp.size(); i++) {
			periodogramSum[i] = periodogramSum[i] / summed;
			if ((frequencies[i] < 1 / symbolPeriod) && (frequencies[i] > -1 / symbolPeriod)) {
				signalPsd.insert(signalPsd.end(), periodogramSum[i]);
			} else {
				noisePsd.insert(noisePsd.end(), periodogramSum[i]);
			}
		}

		// Calculate overall noise and signal power
		// Assume that noise is uniformly distributed and that there is noise all over the spectrum
		for (unsigned int i = 1; i < noisePsd.size(); i++) {
			noisePower += (noisePsd[i - 1] + noisePsd[i]) / 2;
		}

		for (unsigned int i = 1; i < signalPsd.size(); i++) {
			signalPower += (signalPsd[i - 1] + signalPsd[i]) / 2;
		}
		signalPower = signalPower - noisePower * signalPsd.size() / noisePsd.size();
		noisePower = noisePower * periodogramSum.size() / noisePsd.size();
		if (signalPower <= 0) {
			cout << "ERROR: SNR too low, cannot identify the signal within the noise" << "\n";
		} else {
			allSNR.insert(allSNR.end(), signalPower / noisePower);
			cout << "SNR: " << 10 * log10(signalPower / noisePower) << "\n";
		}
		measuredInterval.clear();
		noisePower = 0;
		signalPower = 0;
	}
	
	
	return true;
}


vector<double> SNREstimator::getWindow(WindowType windowType, int windowSize) {
	vector<double> wn(windowSize);
	switch (windowType)
	{
		case Hamming:
			for (int x = 0; x < windowSize; x++) {
				wn[x] = 0.54 - 0.46*cos(2 * PI*x / (windowSize - 1));
			}
			return wn;

		case Hanning:
			for (int x = 0; x < windowSize; x++) {
				wn[x] = 0.5 *(1 - cos(2 * PI*x / (windowSize - 1)));
			}
	}
}



// NOT YET INCLUDED IN THE FFT ".CPP". REMOVE WHEN IT ARRIVES
vector<complex<double>> SNREstimator::fftshift(vector<complex<double>> &vec)
{
	unsigned long long N = vec.size();
	vector<complex<double>> output;

	if (N % 2 == 0)
	{
		for (unsigned long long i = N / 2; i < vec.size(); i++)
		{
			output.push_back(vec[i]);
		}

		for (unsigned long long i = 0; i < N / 2; i++)
		{
			output.push_back(vec[i]);
		}
	}
	else
	{
		N = N + 1;
		for (unsigned long long i = N / 2; i < vec.size(); i++)
		{
			output.push_back(vec[i]);
		}

		for (unsigned long long i = 0; i < N / 2; i++)
		{
			output.push_back(vec[i]);
		}

	}

	return output;

}