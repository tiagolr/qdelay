// Copyright 2025 tilr
// Envelope Follower
#pragma once
#include "JuceHeader.h"
#include "../Globals.h"

using namespace globals;

class Follower
{
public:
	Follower() {};
	~Follower() {};

	void prepare(float srate, float thresh_, bool autorel_, float attack_, float hold, float release, bool detectorOnly);
	float process(float lsamp, float rsamp);
	void clear();

	float outl = 0.0f;
	float outr = 0.0f;

private:
	int rmswindow = 100;
	float thresh = 0.0f;
	bool autorel = false;
	float attack = 1.0f; // s
	float hold = 0.0f; // s
	float holdCounter = 0.0f;
	float release = 1.0f; // s
	float attackcoeff = .1f;
	float releasecoeff = .1f;
	float minreleasecoeff = .1f;
	float envelope = 0.0f;
	float amount = 0.f;
	bool detectorOnly = false;
};