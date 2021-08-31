#include "AudioEngine.h"
#include <miniaudio/miniaudio.h>
#include <util/Logger.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>
#include "AudioSource.h"

AudioEngine::AudioEngine(const cpptoml::table &settings)
{

	if(ma_context_init(nullptr, 0, nullptr, &context) != MA_SUCCESS)
	{
		logger->fatal("Could not initialize miniaudio context");
	}

	hdr_gain = 1.0f;
	is_inside = false;
	// Set default values
	for(size_t i = 0; i < channels.size(); i++)
	{
		channels[i].use_hdr = i != 0;
		channels[i].gain = 1.0f;
		std::string ch = "audio_engine.channel_";
		ch += std::to_string(i);
		channels[i].external_gain = settings.get_qualified_as<double>(ch + "_ext_gain").value_or(1.0f);
		channels[i].internal_gain = settings.get_qualified_as<double>(ch + "_int_gain").value_or(1.0f);
	}
	master_gain = settings.get_qualified_as<double>("audio_engine.gain").value_or(1.0f);
	simple_panning = settings.get_qualified_as<bool>("audio_engine.simple_panning").value_or(false);

	// We retrieve all devices to allow configuration to use the device by string. We store this for
	// the settings interface
	// This pointers are deleted when we end the context, they are not used again!
	ma_device_info* infos;
	ma_uint32 playback_count;
	ma_context_get_devices(&context, &infos, &playback_count, nullptr, nullptr);

	logger->info("Audio device enumeration:");
	ma_int32 chosen_i = -1;
	std::string wanted_device = settings.get_qualified_as<std::string>("audio_engine.device").value_or("");
	for(ma_uint32 i = 0; i < playback_count; i++)
	{
		audio_devices.emplace_back((char*)&infos[i].name[0]);

		logger->info("- {}", infos[i].name);
		if(!wanted_device.empty())
		{
			if(strcmp(infos[i].name, wanted_device.c_str()) == 0)
			{
				logger->info("Found wanted audio device '{}'", infos[i].name);
				chosen_i = (ma_int32)i;
			}
		}
	}

	if(chosen_i == -1)
	{
		chosen_i = 0;
		if(!wanted_device.empty())
		{
			logger->warn("Could not find audio device '{}', using default", wanted_device);
		}
	}

	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.pDeviceID = &infos[chosen_i].id;
	config.playback.format = ma_format_f32;
	config.playback.channels = 2;
	// We use the native samplerate. Audio sources will all output in this samplerate
	config.sampleRate = 0;
	config.dataCallback = data_callback;
	config.pUserData = (void*)this;

	if(ma_device_init(nullptr, &config, &device) != MA_SUCCESS)
	{
		logger->fatal("Could not initialize miniaudio device");
	}

	this->sample_rate = device.sampleRate;

	// This may be an unnecesarly HUGE buffer. We could reduce it if perfomance is affected
	mix_buffer_size = sample_rate;

	mix_buffer = (float*)calloc(mix_buffer_size, sizeof(float) * 2);
	chmix_buffer = (float*)calloc(mix_buffer_size, sizeof(float) * 2);

	// From here on, the thread is running
	ma_device_start(&device);
}

AudioEngine::~AudioEngine()
{
	ma_device_uninit(&device);
	ma_context_uninit(&context);
}

std::pair<float, float> AudioEngine::get_panning(glm::dvec3 pos)
{
	float left = 1.0f, right = 1.0f;
	// We project the sound source through up into the plane of the camera, and obtain
	// the angle. Then we either do simple panning or our custom, better sounding algorithm for headphones
	glm::dvec3 from_to = pos - listener_pos;
	double dist_along_normal = glm::dot(from_to, listener_up);
	glm::dvec3 proy_point = pos - listener_up * dist_along_normal;
	glm::dvec3 from_to_proy = glm::normalize(proy_point - listener_pos);
	// Right handed coordinate system!

	// We now find the angle between the proyected point and forward
	// (1 means it's forward, 0.0 means it's 90º right / left, -1.0 means it's behind)
	// (Left or right is determined by cangle_right)
	double cangle = glm::dot(listener_fwd, from_to_proy) * glm::half_pi<float>();
	bool cangle_right = glm::dot(from_to_proy, listener_right) > 0.0;

	if(simple_panning)
	{
		right = 1.0f;
		left = glm::sin(glm::abs(cangle));
	}
	else
	{
		// here we use a more advanced formula that doesnt zero out one ear
		// on full stereo and that slightly lowers the volume when the sound is behind
		// the head, attempting to "simulate" a head transfer function, but with flat frequency response
		// (The numbers were chosen to satisfy tatjam's ears, and may not be appropiate for everybody!)
		float bx = glm::abs(glm::sin(cangle));
		float ox = -glm::sin(glm::min(cangle, 0.0)) * 0.3;
		left = glm::max(bx - ox, 0.15f);
		right = 1.0f - ox;
	}

	if(!cangle_right)
	{
		std::swap(right, left);
	}


	logger->info("Right: {}, Left: {}", right, left);

	return std::make_pair(left, right);
}

void AudioEngine::set_listener(glm::dvec3 pos, glm::dvec3 fwd, glm::dvec3 up)
{
	mtx.lock();
	listener_pos = pos;
	listener_fwd = fwd;
	listener_up = up;
	listener_right = glm::cross(listener_fwd, listener_up);
	mtx.unlock();
}

// As a little guide, this is expected to be called a few times per frame, although it could greatly
// depend on platform. We are not a realtime audio application so it's no big deal
// For example, on my (tatjam's) linux system it's called at around 350FPS
// TODO: We could use many optimizations, such as SIMD if possible
void AudioEngine::data_callback(ma_device* device, void* output, const void* input, ma_uint32 frames)
{

	auto* engine = (AudioEngine*)device->pUserData;
	float* foutput = (float*)output;
	float* fmix = engine->mix_buffer;
	float* fmixch = engine->chmix_buffer;

	if(frames > engine->mix_buffer_size)
	{
		logger->warn("Could not mix audio, request too big!");
		return;
	}

	engine->mtx.lock();

	// Channels are mixed channel by channel and then master gain applied
	for(size_t ch_i = 0; ch_i < engine->channels.size(); ch_i++)
	{
		AudioChannel* ch = &engine->channels[ch_i];
		float gain = ch->gain *
				(engine->is_inside ? ch->internal_gain : ch->external_gain) *
				(ch->use_hdr ? engine->hdr_gain : 1.0f);

		if(gain == 0.0f)
		{
			continue;
		}


		bool any = false;
		bool first = true;
		for(const auto& source : ch->sources)
		{
			float attenuation = 1.0f, left = 1.0f, right = 1.0f;
			if(source->is_3d_source())
			{
				auto[nleft, nright] = engine->get_panning(source->get_position());
				left = nleft; right = nright;
				float distance = (float)glm::distance(source->get_position(), engine->listener_pos);
			}

			// Source will NOT apply their own gain
			bool written = source->mix_samples(fmix, frames);
			any |= written;

			// Apply directionality and attenuation, mixing into the channel
			if(written)
			{
				float lgain = left * attenuation * source->get_gain();
				float rgain = right * attenuation * source->get_gain();
				for (ma_uint32 i = 0; i < frames; i++)
				{
					if(first)
					{
						// We must overwrite fmixch as it contains data from previous channels / audio requests
						fmixch[i * 2 + 0] = fmix[i * 2 + 0] * lgain;
						fmixch[i * 2 + 1] = fmix[i * 2 + 1] * rgain;
					}
					else
					{
						fmixch[i * 2 + 0] += fmix[i * 2 + 0] * lgain;
						fmixch[i * 2 + 1] += fmix[i * 2 + 1] * rgain;
					}
				}
				first = false;
			}

		}

		if(any)
		{
			// Apply filters to fmixch

			// Apply gain and HDR (*2 because we use stereo), and mix into the output
			for (ma_uint32 i = 0; i < frames; i++)
			{
				foutput[i * 2 + 0] += fmixch[i * 2 + 0] * gain;
				foutput[i * 2 + 1] += fmixch[i * 2 + 1] * gain;
			}
		}

	}

	engine->mtx.unlock();

}

std::weak_ptr<AudioSource> AudioEngine::create_audio_source(uint32_t in_channel)
{
	mtx.lock();
	auto src = std::make_shared<AudioSource>(AudioSource(this, in_channel));
	channels[in_channel].sources.push_back(src);
	mtx.unlock();
	return src;
}

