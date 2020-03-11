#include <cmath>

#include "Navball.h"

#include "../../util/DebugDrawer.h"

void Navball::draw_to_texture(const Vessel& vessel, const ReferenceFrame& frame)
{
	// Calculate the FOV from view distance
	if (view_distance <= 1.0f)
	{
		view_distance = 1.0f;
	}

	glm::dvec3 rel_vel = vessel.state.vel - frame.get_velocity(vessel.state.pos);


	glm::quat rot = vessel.rotation;
	glm::quat prog = MathUtil::quat_look_at(glm::dvec3(0.0, 0.0, 0.0), rel_vel);

	glm::dquat rot0 = glm::conjugate(glm::toQuat(frame.get_rotation_matrix())); 
	rot = (glm::quat)rot0 * rot;

	glm::dvec3 prograde = glm::normalize(rel_vel);
	glm::dvec3 normal = glm::normalize(glm::cross(rel_vel, frame.center.get_up_now()));
	glm::dvec3 radialin = glm::cross(prograde, normal);

	debug_drawer->add_line(vessel.state.pos, vessel.state.pos + glm::normalize(rel_vel) * 20.0, glm::vec3(1.0, 1.0, 1.0));

	// Simple trigonometry leads to this
	// Given the tangent point Q, the view point P and the 
	// circle center C, the triangle PQC is rectangle (as Q is the tangent point)
	// we know the sine as we know the opposite to the angle (1.0f) and the
	// hypothenuse (view_distance)
	float angle_sin = 1.0f / view_distance;
	float fov = std::asin(angle_sin) * 2.0f;

	glm::mat4 view, proj, model;
	// 60 degrees as we see the sphere from twice its radius
	// (Hence we form an equilateral triangle)
	proj = glm::perspective(fov, 1.0f, 0.01f, 255.0f);
	view = glm::lookAt(glm::vec3(view_distance, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -voffset * scale, 0.0f));
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	model *= glm::toMat4(glm::normalize(glm::conjugate(rot)));

	fbuffer.bind();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	navball_shader->use();


	navball_shader->setMat4("view", view);
	navball_shader->setMat4("proj", proj);
	navball_shader->setMat4("model", model);
	navball_shader->setInt("tex", 0);
	navball_shader->setInt("progradeTex", 1);
	navball_shader->setInt("retrogradeTex", 2);
	navball_shader->setInt("normalTex", 3);
	navball_shader->setInt("antinormalTex", 4);
	navball_shader->setInt("radialInTex", 5);
	navball_shader->setInt("radialOutTex", 6);

	navball_shader->setFloat("iconSize", icon_scale);


	glm::vec3 mapped_prograde = glm::normalize(rot0 * prograde);
	glm::vec3 mapped_retrograde = glm::normalize(rot0 * -prograde);
	glm::vec3 mapped_normal = glm::normalize(rot0 * normal);
	glm::vec3 mapped_antinormal = glm::normalize(rot0 * -normal);
	glm::vec3 mapped_radialin = glm::normalize(rot0 * radialin);
	glm::vec3 mapped_radialout = glm::normalize(rot0 * -radialin);
	navball_shader->setVec4("progradePos", glm::vec4(mapped_prograde, 0.0f));
	navball_shader->setVec4("retrogradePos", glm::vec4(mapped_retrograde, 0.0f));
	navball_shader->setVec4("normalPos", glm::vec4(mapped_normal, 0.0f));
	navball_shader->setVec4("antinormalPos", glm::vec4(mapped_antinormal, 0.0f));
	navball_shader->setVec4("radialInPos", glm::vec4(mapped_radialin, 0.0f));
	navball_shader->setVec4("radialOutPos", glm::vec4(mapped_radialout, 0.0f));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, prograde_tex->id);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, retrograde_tex->id);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, normal_tex->id);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, antinormal_tex->id);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, radialin_tex->id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, radialout_tex->id);

	glBindVertexArray(ball_vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)ball_index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	texture_drawer->draw(frame_tex->id, glm::vec2(0.0, 0.0),
		//glm::vec2(fbuffer.get_size().x, -fbuffer.get_size().y), fbuffer.get_size());
		fbuffer.get_size(), fbuffer.get_size());

	// Draw speed
	glm::vec2 speed_text_pos = glm::vec2(180, 114) / (glm::vec2)frame_tex->get_size() * (glm::vec2)fbuffer.get_size();
	speed_text_pos.y = (float)fbuffer.get_size().y - speed_text_pos.y;
	
	std::string speed_text = std::to_string((int)glm::length(rel_vel));

	text_drawer->draw_text_aligned(
		speed_text, speed_font, speed_text_pos, TextDrawer::LEFT,
		fbuffer.get_size(), glm::vec4(0.196, 0.706, 0.2, 1.0), 
		glm::vec2(final_scale * 0.5f, -final_scale * 0.5f));

	fbuffer.unbind();
}

void Navball::draw_to_screen(glm::ivec2 screen_size)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, screen_size.x, screen_size.y);

	float x_size = fbuffer.get_size().x * final_scale;
	float y_size = fbuffer.get_size().y * final_scale;

	glDisable(GL_DEPTH_TEST);

	texture_drawer->draw(fbuffer.tex_color_buffer, 
		glm::vec2(screen_size.x * xoffset - x_size / 2.0f, screen_size.y - y_size), 
		glm::vec2(x_size, y_size), screen_size, false);


	glEnable(GL_DEPTH_TEST);
}


Navball::Navball() : fbuffer(256, 256)
{
	navball_shader = assets->get<Shader>("navball", "navball.vs");
	texture = assets->get<Image>("navball", "navball.png");
	prograde_tex = assets->get<Image>("navball", "prograde.png");
	retrograde_tex = assets->get<Image>("navball", "retrograde.png");
	normal_tex = assets->get<Image>("navball", "normal.png");
	antinormal_tex = assets->get<Image>("navball", "antinormal.png");
	radialin_tex = assets->get<Image>("navball", "radialin.png");
	radialout_tex = assets->get<Image>("navball", "radialout.png");
	frame_tex = assets->get<Image>("navball", "frame.png");

	SphereGeometry::generate_and_upload(&ball_vao, &ball_vbo, &ball_ebo, &ball_index_count, 64);
}

Navball::~Navball()
{
}
