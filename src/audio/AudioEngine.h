#pragma once
#include <cpptoml.h>
#include <miniaudio/miniaudio.h>

// TODO: The AudioEngine should allow disabling audio, and then it will simulate playback
// We always use f32 audio, as it seems to be convenient, if this supposes a big perfomance hit
// it could be changed to s16, which is the typical internal format, at the cost of mixing precision!
// We always use the native sample rate and resample loaded audio
class AudioEngine
{
private:

	ma_context context;
	ma_device device;
	size_t sample_rate;

	// For the settings interface, and to store in the config. We compare
	// the raw strings, and if missing, use the default device
	std::vector<std::string> audio_devices;

public:

	size_t get_sample_rate() const { return sample_rate; }

	explicit AudioEngine(const cpptoml::table& settings);
	~AudioEngine();
};
