#pragma once

class OSP;

// Scenes handle user interaction with the universe, either via seeing
// and controlling vehicles or other task (creating vehicles, discovering technology...)
class Scene
{
private:

	OSP* osp;

public:


	// Called the frame the scene is loaded
	virtual void load() = 0;
	virtual void update() = 0;
	virtual void render() = 0;
	// Called the frame the scene is unloaded
	virtual void unload() = 0;

	

	OSP* get_osp()
	{
		return osp;
	}

	void setup(OSP* osp)
	{
		this->osp = osp;
	}

	virtual ~Scene() {}	
};
