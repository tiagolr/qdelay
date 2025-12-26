#pragma once
#include <vector>
#include <cmath>

class DelayLine {
public:
	int size = 0;

	void resize(int newSize)
	{
		newSize = std::max(1, newSize);
		size = newSize;

		if (newSize > (int)buf.size())
		{
			buf.resize(newSize, 0.0f);
		}

		pos %= size;
	}

	void clear()
	{
		std::fill(buf.begin(), buf.end(), 0.f);
	}

	// writes to buffer with an offset without advancing the read position
	// used to transpose the whole delay for Feel offset
	void writeOffset(float s, int offset, bool overdub)
	{
		int writepos = pos + offset;
		offset %= size;
		while (writepos < 0) writepos += size;
		while (writepos >= size) writepos -= size;

		if (overdub) buf[writepos] += s;
		else buf[writepos] = s;
	}

	// override is a hack used for Feel offset
	// the input is written at an offset while the feedback is written at standard write position
	// the most recent write replaces the tape while lagged one adds to it depending on offset pos or neg
  	void write(float s, bool overdub = false)
	{
		if (overdub) buf[pos] += s;
		else buf[pos] = s;

  		pos += 1;
  		if (pos >= size) 
			pos -= size;
	}

	float read(float delay)
	{
	    float _pos = pos - delay;

	    while (_pos < 0) _pos += (float)size;
	    while (_pos >= size) _pos -= (float)size;

	    int i0 = (int)std::floor(_pos);
	    int i1 = i0 + 1;
	    if (i1 >= size) i1 -= size;

	    float frac = _pos - (float)i0;

	    float y0 = buf[i0];
	    float y1 = buf[i1];
	    return y0 + frac * (y1 - y0);
	}

  	float read(int delay)
	{
		int _pos = pos - delay;
		if (_pos < 0) _pos += size;
		if (_pos >= size) _pos -= size;
		return buf[_pos];
	}

  	float read3(float delay)
	{
		float _pos = pos - delay;

		while (_pos < 0) _pos += (float)size;
		while (_pos >= size) _pos -= (float)size;

		int i1 = (int)std::floor(_pos);
		int i0 = (i1 - 1 + size) % size;
		int i2 = (i1 + 1) % size;
		int i3 = (i1 + 2) % size;

		float frac = _pos - (float)i1;

		float y0 = buf[i0];
		float y1 = buf[i1];
		float y2 = buf[i2];
		float y3 = buf[i3];

		float a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
		float a1 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
		float a2 = -0.5f * y0 + 0.5f * y2;
		float a3 = y1;

		return ((a0 * frac + a1) * frac + a2) * frac + a3;
	}

private:
  	std::vector<float> buf;
  	int pos = 0;
};