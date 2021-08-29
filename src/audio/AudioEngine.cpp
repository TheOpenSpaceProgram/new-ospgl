#include "AudioEngine.h"
#include <miniaudio/miniaudio.h>
#include <util/Logger.h>

void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frames)
{

}

AudioEngine::AudioEngine(const cpptoml::table &settings)
{
	if(ma_context_init(nullptr, 0, nullptr, &context) != MA_SUCCESS)
	{
		logger->fatal("Could not initialize miniaudio context");
	}

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


	ma_device_start(&device);
}

AudioEngine::~AudioEngine()
{
	ma_device_uninit(&device);
	ma_context_uninit(&context);
}

