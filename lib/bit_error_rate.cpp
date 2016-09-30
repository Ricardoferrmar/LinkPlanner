#include <algorithm>
#include <complex>

#include "netxpto.h"
#include "bit_error_rate.h"

void BitErrorRate::initialize(void){
	firstTime = false;

	outputSignals[0]->setSymbolPeriod(inputSignals[0]->getSymbolPeriod());
	outputSignals[0]->setSamplingPeriod(inputSignals[0]->getSamplingPeriod());
	outputSignals[0]->setFirstValueToBeSaved(inputSignals[0]->getFirstValueToBeSaved());
}


bool BitErrorRate::runBlock(void){
	int ready = inputSignals[0]->ready();
	int space = outputSignals[0]->space();

	int process = min(ready, space);

	float NumberOfBits = recievedbits;
	float Coincidences = coincidences;

	float BER;
	BER = (NumberOfBits - Coincidences) / NumberOfBits;

	if (process == 0)
	{
		t_real z = 1.96;
		t_real UpperBound = BER + 1 / sqrt(NumberOfBits) * z  * sqrt(BER*(1 - BER)) + 1 / (3 * NumberOfBits)*(2 * z * z * (1 / 2 - BER) + (2 - BER));
		t_real LowerBound = BER - 1 / sqrt(NumberOfBits) * z  * sqrt(BER*(1 - BER)) + 1 / (3 * NumberOfBits)*(2 * z * z * (1 / 2 - BER) - (1 + BER));

		/* Outputting a .txt report*/
		ofstream myfile;
		myfile.open("BER.txt");
		myfile << "BER=" << BER << "\n";
		myfile << "Upper and Lower bounds for 95\% confidence interval \n";
		myfile << "Upper Bound=" << UpperBound << "\n";
		myfile << "Lower Bound=" << LowerBound << "\n";
		myfile << "Number of recieved bits =" << NumberOfBits << "\n";
		myfile.close();
		return false;
	}



	for (int i = 0; i < process; i++) {

		t_binary signalValue;
		inputSignals[0]->bufferGet(&signalValue);
		t_binary SignalValue;
		inputSignals[1]->bufferGet(&SignalValue);

		recievedbits++;

		if (signalValue == SignalValue)
		{
			coincidences++;
			outputSignals[0]->bufferPut(1);
		}
		else
		{
			outputSignals[0]->bufferPut(0);
		}




	}
	return true;

}