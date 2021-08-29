#pragma once

// Generic class for everything that provides audio to an audio source
class SampleSource
{
public:


	virtual bool is_playing() = 0;
	virtual void get_samples() = 0;
};