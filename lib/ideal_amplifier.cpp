#include <algorithm>  // min()
#include <complex>

#include "netxpto.h"
#include "ideal_amplifier.h"


void IdealAmplifier::initialize(void){

	outputSignals[0]->setSymbolPeriod(inputSignals[0]->getSymbolPeriod());
	outputSignals[0]->setSamplingPeriod(inputSignals[0]->getSamplingPeriod());
	outputSignals[0]->setFirstValueToBeSaved(inputSignals[0]->getFirstValueToBeSaved());

}


bool IdealAmplifier::runBlock(void){

	int ready = inputSignals[0]->ready();
	int space = outputSignals[0]->space();

	int process = min(ready, space);

	if (process == 0) return false;
	
	signal_value_type sType = inputSignals[0]->getValueType();

	switch (sType) {
		case RealValue:
			t_real inReal;
			for (int k = 0; k < process; k++) {
				inputSignals[0]->bufferGet(&inReal);
				outputSignals[0]->bufferPut((t_real)gain*inReal);
			}
			break;
		case ComplexValue:
			t_complex inComplexValue;
			for (int k = 0; k < process; k++) {
				inputSignals[0]->bufferGet(&inComplexValue);
				outputSignals[0]->bufferPut((t_complex)gain*inComplexValue);
			}
			break;
		case ComplexValueXY:
			t_complex_xy inComplexValueXY;
			for (int k = 0; k < process; k++) {
				inputSignals[0]->bufferGet(&inComplexValueXY);
				outputSignals[0]->bufferPut((t_complex_xy) { gain*inComplexValueXY.x, gain*inComplexValueXY.y });
				break;
			}
			break;
	}
	return true;
}