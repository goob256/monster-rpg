#include <vector>

#include "monster.h"

class Slow1Affect : public BattleAffect {
public:
	static const int DURATION = 30;
	static const int AMOUNT = 30;

	bool update(int step) {
		count += step;
		if (count >= DURATION) {
			return true;
		}
		return false;
	}

	void apply(BattleStats* target) {
		target->latency += AMOUNT;
	}

	Slow1Affect() : BattleAffect("Slow1") {
		count = 0;
		good = false;
	}

private:
	int count;
};

BattleAffect* createBattleAffect(char* name)
{
	if (!strcmp(name, "Slow1")) {
		return new Slow1Affect();
	}
	return 0;
}
