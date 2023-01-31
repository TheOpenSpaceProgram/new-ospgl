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
	line_shader = AssetHandle<Shader>("core:shaders/lines/lines.vs");
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
			// We have to use vec4 for memory alignment reasons
			std::vector<glm::vec4> final_points;
			final_points.resize(pred_points->size());
			for(size_t i = 0; i < pred_points->size(); i++)
			{
				glm::dvec3 point = (*pred_points)[i];
				if (elem_points)
				{
					// TODO: Transform points with ref and elem_points
				}
				glm::vec3 vec = (glm::vec3)(point * scale);
				final_points[i] = glm::vec4(vec, 1.0f);
			}

			// Send to the GPU
			size_t size = final_points.size();
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, put_points * sizeof(float) * 4, size * sizeof(float) * 4, (float*)final_points.data());

			put_points += size;
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		points_in_ssbo = put_points;
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
	glBufferData(GL_SHADER_STORAGE_BUFFER, points * sizeof(float) * 4, nullptr, GL_DYNAMIC_DRAW);
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

void PredictionDrawer::forward_pass(CameraUniforms &cu)
{
	if(points_in_ssbo < 3)
		return;

	auto frame = pr->get_ref();
	glm::dmat4 center_body_tform = frame.get_tform_matrix();
	glm::dmat4 premult = cu.tform * center_body_tform;

	line_shader->use();
	line_shader->setMat4("tform", premult);
	line_shader->setVec2("screen", cu.screen_size);
	line_shader->setFloat("f_coef", 2.0f / glm::log2(cu.far_plane + 1.0f));
	line_shader->setFloat("thickness", 3.0f);
	line_shader->setVec4("color", glm::vec4(1.0, 0.0, 1.0, 1.0));
	line_shader->setFloat("inv_scale", (float)(1.0 / scale));

	// Bind the buffers and draw
	glBindVertexArray(vao);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	size_t n = points_in_ssbo - 2;
	glDrawArrays(GL_TRIANGLES, 0, 6 * (n - 1));
}
