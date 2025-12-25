#include "Follower.h"

void Follower::prepare(float srate, float thresh_, bool autorel_, float attack_, float hold_, float release_)
{
	thresh = thresh_;
	autorel = autorel_;
	attack = (ENV_MIN_ATTACK + (ENV_MAX_ATTACK - ENV_MIN_ATTACK) * attack_) / 1000.0f;
	(void)hold_;
	hold = 0.f;
	release = (ENV_MIN_RELEASE + (ENV_MAX_RELEASE - ENV_MIN_RELEASE) * release_) / 1000.0f;

	float targetLevel = 0.2f; // -14dB or something slow
	attackcoeff = std::exp(std::log(targetLevel) / (attack * srate));
	releasecoeff = std::exp(std::log(targetLevel) / (release * srate));
	float minReleaseTime = release * 0.2f; // faster release
	minreleasecoeff = std::exp(std::log(targetLevel) / (minReleaseTime * srate));
}

float Follower::process(float lsamp, float rsamp)
{
	float amp = std::max(std::fabs(lsamp), std::fabs(rsamp));
	float in = std::max(0.0f, amp - thresh);

	if (in > envelope) {
		envelope = attackcoeff * envelope + (1.0f - attackcoeff) * in;
		holdCounter = hold;
	}
	else if (holdCounter > 0.0f) {
		holdCounter -= 1.0f; // Decrement hold timer
	}
	else if (autorel) {
		float releaseRatio = (envelope - in) / (envelope + 1e-12f);
		releaseRatio = releaseRatio * releaseRatio;
		releaseRatio = std::clamp(releaseRatio, 0.0f, 1.0f);
		float adaptiveCoeff = releasecoeff + (minreleasecoeff - releasecoeff) * releaseRatio;
		envelope = adaptiveCoeff * envelope + (1.0f - adaptiveCoeff) * in;
	}
	else {
		envelope = releasecoeff * envelope + (1.0f - releasecoeff) * in;
	}

	return envelope;
}

void Follower::clear()
{
	outl = 0.0f;
	outr = 0.0f;
	envelope = 0.0f;
}