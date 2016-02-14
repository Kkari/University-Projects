#pragma once

#include <cglib/core/camera.h>
#include <cglib/gl/shader.h>

#include <glm/glm.hpp>

// DO NOT PUT ANY OPENGL CALLS INTO CONSTRUCTOR!!!
// all intialization must be done in initialize!

class DeviceRenderingParameters;
class GLObjModel;
struct FBO;
struct AABB;

/*
 * Call the drawing functor, but first set up the OpenGL context so that
 * all geometry is drawn as a wireframe overlay.
 */
template <typename DrawFunc>
void wireframe_overlay(DrawFunc&& draw)
{
	// Store a copy of the current OpenGL state for all attributes that
	// we are going to modify.
	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT);

	// Disable depth testing and writing (this is an overlay), and draw as lines.
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Slightly thicker lines for
	//glLineWidth(2.f);

	// Draw using the provided functor.
	draw();

	// Restore OpenGL state.
	glPopAttrib();
}

class Renderer
{
public:
	ShaderManager shader_manager;
	std::shared_ptr<Camera> camera;
	Renderer(const MapNameProgramDefinition &program_definitions)
		: shader_manager(program_definitions)
	{
	}
	virtual ~Renderer()       {}
	virtual void draw()       = 0;
	virtual void destroy()    {}
	virtual void activate()   {}
	virtual void deactivate() {}
	virtual int  initialize(const DeviceRenderingParameters &parameters);
	virtual int  update_parameters(const DeviceRenderingParameters &parameters);
	virtual void set_active_camera();
	virtual void update_objects(float time_step);

	glm::mat4 projection;
};

class RGBCubeRenderer : public Renderer
{
public:
	RGBCubeRenderer();

	virtual void draw() override;
	virtual void destroy() override;
	virtual int  initialize(const DeviceRenderingParameters &parameters) override;
	virtual int  update_parameters(const DeviceRenderingParameters &parameters) override;

	std::vector<glm::vec3> cube_vertices;
	std::vector<glm::vec3> cube_wire_colors;

	bool showWireframe = false;

	static const MapNameProgramDefinition program_definitions;
};

class ColorMatchingRenderer : public Renderer
{
public:
	ColorMatchingRenderer();

	virtual void draw() override;
	virtual void destroy() override;
	virtual int  initialize(const DeviceRenderingParameters &parameters) override;
	virtual int  update_parameters(const DeviceRenderingParameters &parameters) override;

	// Triangle strip used for the spectrum visualization.
	std::vector<glm::vec3> spectrum_vertices;
	std::vector<glm::vec3> spectrum_colors;
	std::vector<glm::vec3> blackbody_vertices;
	std::vector<glm::vec3> blackbody_colors;
	std::vector<glm::vec3> blackbody_wire_colors;

	bool showWireframe = false;
	float temperature = 6500.f;
	std::uint32_t tesselation = 16;

	void update_spectrum(
		// Gradient strip resolution.
		std::uint32_t resolution,
		// Current temperature.
		float T);


	static const MapNameProgramDefinition program_definitions;
};

class GravityFieldRenderer : public Renderer
{
public:
	GravityFieldRenderer();

	virtual void draw() override;
	virtual void destroy() override;
	virtual int  initialize(const DeviceRenderingParameters &parameters) override;
	virtual int  update_parameters(const DeviceRenderingParameters &parameters) override;

	// Gravitational potential height field.
	std::vector<glm::vec3>  gravity_field_vertices;
	std::vector<glm::vec3>  gravity_field_colors;
	std::vector<glm::vec3>  gravity_wire_colors;
	std::vector<glm::uvec3> gravity_field_indices;

	bool showWireframe = false;
	float mass0;
	glm::vec3 mass0Center = glm::vec3(0.25f, 0.f, 0.25f);
	float mass1;
	glm::vec3 mass1Center = glm::vec3(0.75f, 0.f, 0.75f);
	std::uint32_t tesselation;

	void update_gravity_field(
		// Grid resolution.
		std::uint32_t resolution,
		// Point mass 1.
		glm::vec3 const& p1,
		float m1,
		// Point mass 2.
		glm::vec3 const& p2,
		float m2);

	static const MapNameProgramDefinition program_definitions;
};

