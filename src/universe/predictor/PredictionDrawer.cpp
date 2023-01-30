#include "PredictionDrawer.h"
#include <vector>
#include "FrameOfReference.h"



PredictionDrawer::PredictionDrawer(Prediction* pred, double nscale)
{
	pr = pred;
	ssbo = 0;
	vao = 0;
	scale = nscale;
	rebuild = false;
}


PredictionDrawer::~PredictionDrawer()
= default;

void PredictionDrawer::update()
{
	// TODO: This lock could be more "localized"
	pr->lock.lock();
	int dirty = pr->get_dirty_level();

	if(dirty >= 0)
	{
		size_t points = pr->get_num_points();
		if(ssbo == 0)
		{
			create_buffers();
			resize_buffers();
		}
		else
		{
			if(points > size_of_ssbo)
			{
				resize_buffers();
			}
		}
		int num = 0;
		size_t put_points = 0;

		std::vector<glm::dvec3>* pred_points;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		while(pr->get_prediction_points(&pred_points))
		{
			num++;
			if(num - 1 < dirty)
			{
				// We "put" all previous points
				put_points += pred_points->size();
				continue;
			}

			FrameOfReference ref = pr->get_ref();
			std::vector<CartesianState>* elem_points = pr->get_element_points();
			std::vector<glm::vec3> final_points;
			final_points.resize(pred_points->size());
			for(size_t i = 0; i < pred_points->size(); i++)
			{
				glm::dvec3 point = (*pred_points)[i];
				if (elem_points)
				{
					// TODO: Transform points with ref and elem_points
				}
				glm::vec3 vec = (glm::vec3)(point * scale);
				final_points.push_back(vec);
			}

			// Send to the GPU
			size_t size = final_points.size();
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, put_points * sizeof(float) * 3, size * sizeof(float) * 3, (float*)final_points.data());

			put_points += size;
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	}

	pr->lock.unlock();
}


void PredictionDrawer::create_buffers()
{
	glGenBuffers(1, &ssbo);
	glGenVertexArrays(1, &vao);
}

void PredictionDrawer::resize_buffers()
{
	size_t points = pr->get_num_points();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, points * sizeof(float) * 3, nullptr, GL_DYNAMIC_DRAW);
	// Later on glBufferSubData is used to fill it up
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	size_of_ssbo = points;
}

void PredictionDrawer::set_scale(double nval)
{
	// Tiny scale changes are ignored for performance
	if(scale / nval < 1.01 || scale / nval > 0.999)
		return;

	scale = nval;
	rebuild = true;
}
