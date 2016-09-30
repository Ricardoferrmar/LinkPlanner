# ifndef PROGRAM_INCLUDE_TIAMPLIFIER_H_
# define PROGRAM_INCLUDE_TIAMPLIFIER_H_

# include "netxpto.h"
# include <vector>

// Simulates a Transimpedance Amplifier
class TIAmplifier : public Block {

	bool firsTime{ true };

public:

	TIAmplifier(vector<Signal *> &InputSig, vector<Signal *> &OutputSig) :Block(InputSig, OutputSig){};
	
	void initialize(void);
	bool runBlock(void);
private:
};


#endif // !PROGRAM_INCLUDE_TIAMPLIFIER_H_
