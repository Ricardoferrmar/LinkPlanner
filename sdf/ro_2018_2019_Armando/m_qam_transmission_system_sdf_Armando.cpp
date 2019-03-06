# include "../../include/netxpto_20180815.h"

# include "../../include/binary_source_20180815.h"
# include "../../include/discrete_to_continuous_time_20180815.h"
# include "../../include/iq_modulator_20180815.h"
# include "../../include/laser_20180815.h"
# include "../../include/m_qam_mapper_20180815.h"
# include "../../include/pulse_shaper_20180815.h"
# include "../../include/sink_20180815.h"


// #####################################################################################################
// ################################### System Input Parameters #########################################
// #####################################################################################################

double symbolPeriod{ 2e-11 };
int samplesPerSymbol{ 8 };

int main() {

	// #####################################################################################################
	// ########################### Signals Declaration and Inicialization ##################################
	// #####################################################################################################

	Binary S0{ "S0.sgn" };

	Binary S1{ "S1.sgn" };
	
	TimeDiscreteAmplitudeContinuousReal S2{ "S2.sgn" };
	
	TimeDiscreteAmplitudeContinuousReal S3{ "S3.sgn" };
	
	TimeContinuousAmplitudeContinuousReal S4{ "S4.sgn" };
	
	TimeContinuousAmplitudeContinuousReal S5{ "S5.sgn" };
	
	TimeContinuousAmplitudeContinuousReal S6{ "S6.sgn" };
	
	TimeContinuousAmplitudeContinuousReal S7{ "S7.sgn" };
	
	OpticalSignal S8{ "S8.sgn" };

	OpticalSignal S9{ "S9.sgn" };
	
	
	
	// #####################################################################################################
	// ########################### Blocks Declaration and Inicialization ###################################
	// #####################################################################################################


	BinarySource B1{ {}, { &S1, &S0 } };

	Sink B_1{ { &S0 },{} };

	MQamMapper B2{ { &S1 }, { &S2, &S3 } };
	B2.setIqAmplitudes({ {1,1},{-1,1},{1,-1},{-1,-1} });

	DiscreteToContinuousTime B3{ { &S2 }, { &S4 } };

	DiscreteToContinuousTime B4{ { &S3 }, { &S5 } };

	PulseShaper B5{ { &S4 }, { &S6 } };

	PulseShaper B6{ { &S5 }, { &S7 } };

	Laser B7{ {}, { &S8 } };
	B7.setSymbolPeriod(symbolPeriod);
	B7.setSamplingPeriod(symbolPeriod / samplesPerSymbol);


	IqModulator B8{ { &S6, &S7, &S8 }, { &S9 } };

	Sink B_8{ { &S9 }, {} };

	// #####################################################################################################
	// ########################### System Declaration and Inicialization ###################################
	// #####################################################################################################

	System MainSystem{ { &B1, &B_1, &B2, &B3, &B4, &B5, &B6, &B7, &B8, &B_8 } };

	// #####################################################################################################
	// #################################### System Run #####################################################
	// #####################################################################################################

	MainSystem.run();
	MainSystem.terminate();

	system("pause");
	return 0;

}