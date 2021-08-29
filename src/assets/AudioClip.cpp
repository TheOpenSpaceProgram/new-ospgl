#include "AudioClip.h"
#include <miniaudio/miniaudio.h>
#include <OSP.h>
#include <audio/AudioEngine.h>

AudioClip* load_audio_clip(ASSET_INFO, const cpptoml::table &cfg)
{
	ma_decoder decoder;
	ma_result result = ma_decoder_init_file(path.c_str(), NULL, &decoder);

	if(result != MA_SUCCESS)
	{
		logger->error("Miniaudio failed to decode", path);
		return nullptr;
	}

	// We load the frames in arbitrary chunks
	constexpr size_t CHUNK_SIZE = 100000;

	size_t output_channels = std::min(decoder.outputChannels, (ma_uint32)2);
	size_t input_frame_size = ma_get_bytes_per_frame(decoder.outputFormat, decoder.outputChannels);
	size_t output_frame_size = ma_get_bytes_per_frame(ma_format_f32, output_channels);

	// frames_buffer stores the data as read from the file IN CHUNKS
	void* frames_buffer = malloc(CHUNK_SIZE * input_frame_size);
	// total_buffer stores the full data that is converted to the proper format
	void* total_buffer = nullptr;

	// The converter we use to unify all sample types
	ma_data_converter_config converter_cfg = ma_data_converter_config_init_default();
	converter_cfg.formatIn = decoder.outputFormat;
	converter_cfg.formatOut = ma_format_f32;
	converter_cfg.channelsIn = decoder.outputChannels;
	converter_cfg.channelsOut = output_channels;
	converter_cfg.sampleRateIn = decoder.outputSampleRate;
	converter_cfg.sampleRateOut = osp->audio_engine->get_sample_rate();

	ma_data_converter converter;
	result = ma_data_converter_init(&converter_cfg, &converter);
	if(result != MA_SUCCESS)
	{
		logger->error("Miniaudio failed to initialize the converter", path);
		return nullptr;
	}

	ma_uint64 read;
	size_t total_written = 0;
	size_t last_total_written = 0;
	do
	{
		read = ma_decoder_read_pcm_frames(&decoder, frames_buffer, CHUNK_SIZE);
		// Convert the frames using the supplied function
		ma_uint64 frames_in = read;
		ma_uint64 frames_out = ma_data_converter_get_expected_output_frame_count(&converter, read);
		total_buffer = realloc(total_buffer, (total_written + frames_out) * output_frame_size);
		// Offset points to the freely allocated space
		size_t offset = (last_total_written) * output_frame_size;
		// TODO: What happens when it skips a frame? Not sure if this has to be handled
		ma_data_converter_process_pcm_frames(&converter, frames_buffer, &frames_in, (char*)total_buffer + offset, &frames_out);
		last_total_written = total_written;
		total_written += frames_out;
	}
	while(read == CHUNK_SIZE);


	free(frames_buffer);
	ma_decoder_uninit(&decoder);
	ma_data_converter_uninit(&converter);

	return new AudioClip(ASSET_INFO_P, total_buffer, total_written, output_channels);
}

AudioClip::AudioClip(ASSET_INFO, void *samples, size_t sample_count, size_t channel_count) : Asset(ASSET_INFO_P)
{
	this->samples = samples;
	this->sample_count = sample_count;
	this->channel_count = channel_count;
}

AudioClip::~AudioClip()
{
	free(samples);
}
