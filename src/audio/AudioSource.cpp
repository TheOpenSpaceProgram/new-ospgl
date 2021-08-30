#include "AudioSource.h"
#include "AudioEngine.h"

bool AudioSource::mix_samples(void* target, size_t count)
{
	if(!sample_source || !playing || pitch <= 0.0f)
	{
		return false;
	}

	size_t sample_rate = engine->get_sample_rate() * pitch;

	int32_t jump = sample_source->mix_samples((float*)target, count, cur_sample, loops, sample_rate, engine->get_sample_rate());
	if(jump >= 0)
	{
		cur_sample = jump;
	}
	else
	{
		// Audio playback is finished
		playing = false;
		cur_sample = 0;
	}

	return true;
}

void AudioSource::destroy()
{
	engine->mtx.lock();
}

AudioSource::AudioSource(AudioEngine *eng, uint32_t channel)
{
	this->engine = eng;
	this->in_channel = channel;
	// By default we use a 2D audio source
	source_3d = false;
	playing = false;
	gain = 1.0f;
	pitch = 1.0f;
	sample_source = nullptr;
	loops = false;
}

void AudioSource::set_playing(bool value)
{
	engine->mtx.lock();
	playing = value;
	engine->mtx.unlock();
}

void AudioSource::set_3d_source(bool value)
{
	engine->mtx.lock();
	source_3d = value;
	engine->mtx.unlock();
}

void AudioSource::set_position(glm::dvec3 pos)
{
	engine->mtx.lock();
	this->pos = pos;
	engine->mtx.unlock();
}

void AudioSource::set_gain(float val)
{
	engine->mtx.lock();
	gain = val;
	engine->mtx.unlock();
}

void AudioSource::set_pitch(float val)
{
	engine->mtx.lock();
	pitch = val;
	engine->mtx.unlock();
}

void AudioSource::set_source_clip(const AssetHandle<AudioClip>& ast)
{
	// We obtain a new reference
	audio_clip_src = ast.duplicate();
	// We can now safely store a pointer
	engine->mtx.lock();
	sample_source = ast.data;
	engine->mtx.unlock();
}

void AudioSource::set_looping(bool val)
{
	engine->mtx.lock();
	loops = val;
	engine->mtx.unlock();
}
