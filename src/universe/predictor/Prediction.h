#pragma once
#include "FrameOfReference.h"

class Prediction
{
private:
	// If positive, the prediction needs to be sent to the GPU starting
	// at index dirty in returned ptr from get_prediction_point
	int dirty;
public:
	std::mutex lock;

	virtual FrameOfReference get_ref() = 0;
	// Return true as long as there are more points to draw, set *ptr = &points
	// Will always be called to completion
	virtual bool get_prediction_points(std::vector<glm::dvec3>** ptr) = 0;
	// Return the TOTAL number of points in the prediction. May be bigger than the
	// actual value of points to pre-allocate space if you know num of points is going to grow
	virtual size_t get_num_points() = 0;
	// Return nullptr if points are already transformed into FrameOfReference
	virtual std::vector<CartesianState>* get_element_points() = 0;
	bool is_dirty(){ return dirty >= 0; }
	int get_dirty_level() {return dirty; }
	void unset_dirty() { dirty = -1; }
	void set_dirty(int depth){ dirty = depth;}

	Prediction()
	{
		dirty = -1;
	}
};
