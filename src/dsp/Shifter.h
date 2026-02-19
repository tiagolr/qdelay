// Frequency shifter port of Saike's freq shifter library
// Copyright Tilr 2026

#pragma once
#include <JuceHeader.h>
#include "./Utils.h"
#include "../Globals.h"

using namespace globals;
class QDelayAudioProcessor;

class Ellip
{
public:
	double quick_ellip(double x)
	{
		// Elliptical filter at 0.25
	  	y = 0.03974403712835188 * x + s1;
	  	s1 = 0.11443117839583584 * x - -1.2209793606380654 * y + s2;
	  	s2 = 0.4102732984609602 * x - 6.918940386446262 * y + s3;
	  	s3 = 0.8255281436307241 * x - -7.438409047076798 * y + s4;
	  	s4 = 1.6689828207164152 * x - 20.47654014058037 * y + s5;
	  	s5 = 2.5256753272317622 * x - -19.21733444638215 * y + s6;
	  	s6 = 3.6193770241123127 * x - 33.69411950162771 * y + s7;
	  	s7 = 4.250403515943048 * x - -27.235417392156258 * y + s8;
	  	s8 = 4.641846929462009 * x - 33.46680351213294 * y + s9;
	  	s9 = 4.25040351594302 * x - -22.8021725145997 * y + s10;
	  	s10 = 3.6193770241123016 * x - 20.29444701618275 * y + s11;
	  	s11 = 2.525675327231766 * x - -11.231790923026374 * y + s12;
	  	s12 = 1.6689828207164181 * x - 7.173357397659418 * y + s13;
	  	s13 = 0.8255281436307251 * x - -2.9956603900306376 * y + s14;
	  	s14 = 0.41027329846095995 * x - 1.2866484319363045 * y + s15;
	  	s15 = 0.11443117839583594 * x - -0.3305293493933626 * y + s16;
	  	s16 = 0.0397440371283519 * x - 0.07745428581611816 * y;

	  	return y;
	}

	void clear()
	{
		s1 = s2 = s3 = s4 = s5 = s6 = s7 = s8 = s9 = s10 = s11 = s12 = s13 = s14 = s15 = s16 = 0.f;
	}

private:
	double y = 0.;
	double s1 = 0.;
	double s2 = 0.;
	double s3 = 0.;
	double s4 = 0.;
	double s5 = 0.;
	double s6 = 0.;
	double s7 = 0.;
	double s8 = 0.;
	double s9 = 0.;
	double s10 = 0.;
	double s11 = 0.;
	double s12 = 0.;
	double s13 = 0.;
	double s14 = 0.;
	double s15 = 0.;
	double s16 = 0.;
};

class Shifter
{
public:
	bool isOn = false;

	Shifter(QDelayAudioProcessor& p);
	~Shifter() {}

	void prepare(float srate);
	void onSlider();
	void clear();
	void process(float& left, float& right);

private:
	float srate = 44100.f;
	QDelayAudioProcessor& audioProcessor;
	float drymix = 0.f;
    float wetmix = 0.f;
	float shift = 0.f;
	Ellip l1{};
	Ellip l2{};
	Ellip r1{};
	Ellip r2{};

	// freq shifter
	double osc_coeff_t1 = 0.;
	double osc_coeff_t2 = 0.;
	double cos_t1_1 = 0.;
	double cos_t1_2 = 0.;
	double sin_t1_1 = 0.;
	double sin_t1_2 = 0.;
	double cos_t2_1 = 0.;
	double cos_t2_2 = 0.;
	double sin_t2_1 = 0.;
	double sin_t2_2 = 0.;
	double t1 = 0.;
	double t2 = 0.;
	double dt1 = 0.;
	double dt2 = 0.;

	float quick_ellip(float x);
	void init_freq_shifter();
};