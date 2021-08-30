#pragma once
#include "Asset.h"
#include <util/SerializeUtil.h>
#include <audio/SampleSource.h>

// Audio clips. We support only mono and stereo sounds, higher channels are ignored, with a warning.
// This loads the full audio file into memory and does no streaming
// Stereo sounds may be played in 3D sources BUT only their first channel (left) will be mixed.
// All audio types are eventually converted to f32 samples
class AudioClip : public Asset, public SampleSource
{
private:

	size_t channel_count;
	void* samples;
	size_t frame_count;

public:

	size_t get_channel_count() const { return channel_count; }
	size_t get_frame_count() const { return frame_count; }
	void* get_samples() const { return samples; }

	int32_t mix_samples(float* target, uint32_t count, uint32_t cur_frame, bool loop, uint32_t sample_rate,
						uint32_t target_sample_rate) override;

	AudioClip(ASSET_INFO, void* samples, size_t frame_count, size_t channel_count);
	~AudioClip();
};


AudioClip* load_audio_clip(ASSET_INFO, const cpptoml::table& cfg);
