// Based of TARONS MiniVerb
#pragma once
#include <array>
#include "Utils.h"

class Diffusor
{
public:
	static constexpr int NUM_ALLPASS = 8;

	struct AllPass {
		float rawsize = -1.f;
		float srate = 0.f;
		int size = 0;
		float dist = 0.f;
		std::vector<float> buf{};
		RCFilter offsetSmooth{};
		RCFilter attenuationSmooth{};
		float offset = 0.f;
		float attenuation = 1.f;
		float offset_targ = 0.f;
		float attenuation_targ = 0.f;
		int pos = 0;

		void init(float _srate, float apdist, float distance) {
			srate = _srate;
			buf.clear();
			dist = apdist;
			size = (int)(apdist * distance);
			buf.resize(size, 0.f);
			offsetSmooth.setup(0.1f, srate);
			attenuationSmooth.setup(0.1f, srate);
		}

		void setSizeOffsets(float _size) {
			if (rawsize != _size) {
				auto mult = _size * 1.11111f;
				attenuation_targ = std::pow(10.f, -(0.2f * dist * mult) * 0.05f);
				offset_targ = (int)buf.size() * _size;
			}
			rawsize = _size;
		}

		inline float allPass(float in, float feedback) {
			auto fp = pos+offset;
			auto ip = (int)floor(fp);
			auto frc = fp-ip;
			ip = ip % size;

			auto out = (buf[ip] + (buf[(ip+1) % size]-buf[ip]) * frc) - in * feedback;
			buf[pos] = in + out * feedback;
			pos = (pos+1)%size;

			attenuation = attenuationSmooth.process(attenuation_targ);
			offset = offsetSmooth.process(offset_targ);
			return out * attenuation;
		}

		void clear() {
			std::fill(buf.begin(), buf.end(), 0.f);
		}
	};

	Diffusor() {}
	~Diffusor() {}

	void prepare(float _srate);
	void setSize(float size);
	void process(float& left, float& right, float drymix, float wetmix);
	void clear();

private:
	std::array<AllPass, NUM_ALLPASS> allpassL;
	std::array<AllPass, NUM_ALLPASS> allpassR;

	float mps = 0.007f; // meters per second
	float distance = 0.5; // distance in meters
	float srate = 44100.f;
	float smear = 0.75f;
};