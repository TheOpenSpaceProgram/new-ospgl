#pragma once
#include "renderer/Drawable.h"
#include "Prediction.h"

// Draws orbits around their origin
// TODO: Support OpenGL 3.3 by simply drawing non-thick lines instead of using the SSBO
// 		 we may also use a texture or similar to pass the data to the shader?
class PredictionDrawer : public Drawable
{
private:
	AssetHandle<Shader> line_shader;

	Prediction* pr;
	// We use a SSBO to store the points, and a dummy VAO to draw them
	// This means we only need to do partial updates which is fast
	// Note that an overfull SSBO is no problem, and for performance we don't
	// downsize. A downsize can be trigged by manually calling resize_buffers
	// Finally, we use a scale to store the points as floats, they are later
	// upscaled in the shader. This means looking at orbits from up-close will look weird
	size_t points_in_ssbo;
	size_t size_of_ssbo;
	GLuint ssbo;
	GLuint vao;

	bool rebuild;


	void create_buffers();

public:
	// Appropiate value depends on the orbit plotting frame and size
	double scale;
	void resize_buffers();

	// Checks prediction for changes, and updates GPU model if needed
	// Should be very cheap to call as it checks dirty flag!
	void update();

	// Sets scale for next update, EXPENSIVE on next update as it rebuilds the whole orbit
	void set_scale(double nval);

	explicit PredictionDrawer(Prediction* pred, double scale);
	~PredictionDrawer();
};

