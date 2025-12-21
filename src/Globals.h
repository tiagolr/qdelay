#pragma once

namespace globals {
	constexpr float DB2LOG = 0.11512925464970228420089957273422f;

	// filter consts
	inline const float F_MIN_FREQ = 20.0f;
	inline const float F_MAX_FREQ = 20000.0f;

	inline unsigned int COLOR_BACKGROUND = 0xff181818;
	inline unsigned int COLOR_ACTIVE = 0xff469DDA;
	inline unsigned int COLOR_NEUTRAL = 0xff666666;
	inline unsigned int COLOR_KNOB = 0xff272727;
	inline unsigned int COLOR_BEVEL = 0x00000000;

	inline const int EQ_BANDS = 4;
	inline const float EQ_MAX_GAIN = 24.f;
	inline const int EQ_FFT_ORDER = 12;

	// view consts
	inline const int PLUG_WIDTH = 690;
	inline const int PLUG_HEIGHT = 650;
	inline const int MAX_PLUG_WIDTH = 640 * 3;
	inline const int MAX_PLUG_HEIGHT = 650 * 2;
	inline const int PLUG_PADDING = 15;
};