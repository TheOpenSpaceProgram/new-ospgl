#pragma once
#include "SampleSource.h"
#include "assets/AssetManager.h"
#include "assets/AudioClip.h"

class AudioEngine;
// The audio source is the multi-purpose class used for all audio sources
// It allows both 2D and 3D audio
// Their lifetime is handled by AudioEngine, std::weak_ptr is recommended
class AudioSource
{
private:

	// Asset references, we actually use the sample_source pointer
	AssetHandle<AudioClip> audio_clip_src;

	SampleSource* sample_source;

	float gain;
	// Pitch basically means relative play speed.
	// Values other than 1 involve resampling as all audio is at the same frequency by default
	float pitch;

	AudioEngine* engine;
	uint32_t in_channel;
	bool source_3d;
	glm::dvec3 pos;
	bool playing;
	bool loops;

	uint32_t cur_sample;

public:


	void set_playing(bool value);
	bool is_playing() const {return playing;}

	// Return true if anything was played
	bool mix_samples(void* target, size_t count);

	bool is_3d_source() const { return source_3d; }
	void set_3d_source(bool value);

	// Only on 3d sources
	glm::dvec3 get_position() { return pos; }
	void set_position(glm::dvec3 pos);

	float get_gain() const { return gain; }
	void set_gain(float val);

	float get_pitch() const { return pitch; }
	void set_pitch(float val);

	bool is_looping() const { return loops; }
	void set_looping(bool val);

	// We duplicate the asset
	void set_source_clip(const AssetHandle<AudioClip>& ast);

	// Be aware, all other pointers to this will be invalidated
	void destroy();
	AudioSource(AudioEngine* eng, uint32_t channel);
};

