#pragma once
#include <cpptoml.h>
#include <miniaudio/miniaudio.h>
#include <mutex>
#include <glm/glm.hpp>

class AudioSource;

struct AudioChannel
{
	// Gain used for mixing
	float gain;

	// Gain of this channel while in external view, set in the config
	float external_gain;
	// Gain of this channel while in cockpit view, set in the config
	float internal_gain;
	// Should this audio channel affect the HDR simulation?
	bool use_hdr;

	// We handle the lifetime of the sources, which must be created by the engine
	std::vector<std::shared_ptr<AudioSource>> sources;

};

// TODO: The AudioEngine should allow disabling audio, and then it will simulate playback
// We always use f32 audio, as it seems to be convenient, if this supposes a big perfomance hit
// it could be changed to s16, which is the typical internal format, at the cost of mixing precision!
// We always use the native sample rate and resample loaded audio
// We have a fixed number of channels, each with the possibility of applying effects
// This is more or less fixed, as having arbitrary channels feels unnecesary. We clearly separate the
// UI from real sounds, which are affected by HDR and effects.
// TODO: Maybe AudioEngine should be lower level and care less about the game? Probably unnecesary
// TODO: We could implement HRTF as this game could easily benefit from the 3D effects
class AudioEngine
{
private:

	// We use two mixing buffers to allow stereo sounds and effects
	float* mix_buffer;
	float* chmix_buffer;
	uint32_t mix_buffer_size;

	friend class AudioSource;

	glm::dvec3 listener_pos;
	glm::dvec3 listener_fwd;
	// Precomputed
	glm::dvec3 listener_right;
	glm::dvec3 listener_up;

	ma_context context;
	ma_device device;
	size_t sample_rate;

	// Are we inside the cockpit?
	bool is_inside;

	// For the settings interface, and to store in the config. We compare
	// the raw strings, and if missing, use the default device
	std::vector<std::string> audio_devices;

	// Actual gain for hdr sounds, shared between all HDR channels
	float hdr_gain;

	// Gain applied after mixing
	float master_gain;

	// Channel layout:
	// Channel 0: UI, Music, etc... (No effects, no HDR)
	// Channel 1: Ambient sounds (Shared HDR, external effects)
	// Channel 2: External sounds (Shared HDR, external effects)
	// Channel 3: Cockpit sounds (Shared HDR, internal effects)
	std::array<AudioChannel, 4> channels;


	// Remember that audio is in a different thread!
	std::mutex mtx;

	bool simple_panning;

public:

	// Returns left, right pair.
	// TODO: HRTF filter? Could be CPU expensive but sounds awesome
	std::pair<float, float> get_panning(glm::dvec3 pos);

	size_t get_sample_rate() const { return sample_rate; }

	bool get_is_inside() const { return is_inside; }
	void set_is_inside(bool val) {mtx.lock(); is_inside = val; mtx.unlock(); }

	float get_master_gain() const { return master_gain; }
	void set_master_gain(float val) {mtx.lock(); master_gain = val; mtx.unlock(); }

	float get_channel_gain(int channel) const { return channels[channel].gain; }
	void set_channel_gain(int channel, float val) { mtx.lock(); channels[channel].gain = val; mtx.unlock(); }

	void set_listener(glm::dvec3 pos, glm::dvec3 fwd, glm::dvec3 up);
	glm::dvec3 get_listener_pos() const { return listener_pos; }
	glm::dvec3 get_listener_fwd() const { return listener_fwd; }
	glm::dvec3 get_listener_up() const { return listener_up; }

	std::weak_ptr<AudioSource> create_audio_source(uint32_t in_channel);

	static void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frames);

	explicit AudioEngine(const cpptoml::table& settings);
	~AudioEngine();
};
