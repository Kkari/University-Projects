#include <cglib/rt/scene.h>

#include <cglib/rt/epsilon.h>
#include <cglib/rt/light.h>
#include <cglib/rt/object.h>
#include <cglib/rt/raytracing_parameters.h>
#include <cglib/rt/texture.h>

#include <cglib/core/camera.h>
#include <cglib/core/image.h>

#include <sstream>
#include <random>

Scene::~Scene()
{
}

void Scene::
set_active_camera()
{
	if(camera)
		camera->set_active();
}

CornellBox::CornellBox(RaytracingParameters & params)
{
    init_camera(params);
    init_scene(params);
}

void CornellBox::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();

	auto diffuse_gray = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.5f)));

	// build checkerboard wall
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			objects.emplace_back(create_quad(
				glm::vec3(-4.5+i, j+0.5f, -5.f),
				glm::vec3(0.f, 0.f, 1.f),
				glm::vec3(1.f, 0.f, 0.f),
				glm::vec3(0.f, 1.f, 0.f)));
			if ((i % 2 == 0 && j % 2 == 0) || (i % 2 == 1 && j % 2 == 1)) {
				objects.back()->material->k_a = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f, 0.0f, 0.02f)));
				objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f, 0.0f, 0.5f)));
			}
			else {
				objects.back()->material->k_d = diffuse_gray;
			}
			objects.back()->material->k_s = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f, 0.4f, 0.4f)));
			objects.back()->material->n = 10.f;
		}
	}

	// floor
    objects.emplace_back(create_quad(
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 10.f),
        glm::vec3(10.f, 0.f, 0.f)));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.2f)));
    objects.back()->material->k_r = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));

	// ciel
    objects.emplace_back(create_quad(
        glm::vec3(0.f, 10.f, 0.f),
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(10.f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, 10.f)));
    objects.back()->material->k_d = diffuse_gray;

	// left
    objects.emplace_back(create_quad(
        glm::vec3(-5.f, 5.f, 0.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 10.f, 0.f),
        glm::vec3(0.f, 0.f, 10.f)));
    objects.back()->material->k_a = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.02f, 0.0f, 0.0f)));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.5f, 0.0f, 0.0f)));
    objects.back()->material->k_s = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.5f, 0.5f, 0.5f)));
    objects.back()->material->n = 5.f;

	// right
    objects.emplace_back(create_quad(
        glm::vec3(5.f, 5.f, 0.f),
        glm::vec3(-1.f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, 10.f),
        glm::vec3(0.f, 10.f, 0.f)));
    objects.back()->material->k_a = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f, 0.02, 0.0f)));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f, 0.5f, 0.0f)));
    objects.back()->material->k_s = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.5f, 0.5f, 0.5f)));
    objects.back()->material->n = 5.f;

	// pillar right

	auto pillar_gray = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.2f)));
	// front
    objects.emplace_back(create_quad(
        glm::vec3(2.5f, 2.5f, -2.0f),
        glm::vec3(0.f,  0.f, 1.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 5.f, 0.f)
        ));
    objects.back()->material->k_d = pillar_gray;
	// back
    objects.emplace_back(create_quad(
        glm::vec3(2.5f, 2.5f, -3.0f),
        glm::vec3(0.f,  0.f, -1.f),
        glm::vec3(0.f, 5.f, 0.f),
        glm::vec3(1.f, 0.f, 0.f)
        ));
    objects.back()->material->k_d = pillar_gray;
	// left
    objects.emplace_back(create_quad(
        glm::vec3(2.f, 2.5f, -2.5f),
        glm::vec3(-1.f,  0.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(0.f, 5.f, 0.f)
        ));
    objects.back()->material->k_d = pillar_gray;
	// right
    objects.emplace_back(create_quad(
        glm::vec3(3.f, 2.5f, -2.5f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 5.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f)
        ));
    objects.back()->material->k_d = pillar_gray;
	// top
    objects.emplace_back(create_quad(
        glm::vec3(2.5f, 5.f, -2.5f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(1.f, 0.f, 0.f)
        ));
    objects.back()->material->k_d = pillar_gray;

	// pillar left

	// front
    objects.emplace_back(create_quad(
        glm::vec3(-2.5f, 2.5f, -2.0f),
        glm::vec3(0.f,  0.f, 1.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 5.f, 0.f)
        ));
    objects.back()->material->k_d = pillar_gray;
	// back
    objects.emplace_back(create_quad(
        glm::vec3(-2.5f, 2.5f, -3.0f),
        glm::vec3(0.f,  0.f, -1.f),
        glm::vec3(0.f, 5.f, 0.f),
        glm::vec3(1.f, 0.f, 0.f)
        ));
    objects.back()->material->k_d = pillar_gray;
	// left
    objects.emplace_back(create_quad(
        glm::vec3(-2.f, 2.5f, -2.5f),
        glm::vec3(1.f,  0.f, 0.f),
        glm::vec3(0.f, 5.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f)
        ));
    objects.back()->material->k_d = pillar_gray;
	// right
    objects.emplace_back(create_quad(
        glm::vec3(-3.f, 2.5f, -2.5f),
        glm::vec3(-1.f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(0.f, 5.f, 0.f)
        ));
    objects.back()->material->k_d = pillar_gray;
	// top
    objects.emplace_back(create_quad(
        glm::vec3(-2.5f, 5.f, -2.5f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(1.f, 0.f, 0.f)
        ));
    objects.back()->material->k_d = pillar_gray;

	// mirror spheres
    objects.emplace_back(create_sphere(
        glm::vec3(2.5f, 6.5f, -2.5f), 1.5f));
    objects.back()->material->k_r = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(1.0f)));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f)));

    objects.emplace_back(create_sphere(
        glm::vec3(-2.5f, 1.51f, 0.5f), 1.5f));
    objects.back()->material->k_r = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(1.0f)));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f)));

	// glass spheres
	objects.emplace_back(create_sphere(
        glm::vec3(2.5f, 1.51f, 0.5f), 1.5f));
    objects.back()->material->k_t = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.9f)));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f)));
    objects.back()->material->eta = glm::vec3(1.f) / glm::vec3(1.03f, 1.06f, 1.09f);

	objects.emplace_back(create_sphere(
        glm::vec3(-2.5f, 6.5f, -2.5f), 1.5f));
    objects.back()->material->k_t = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.9f)));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f)));
    objects.back()->material->eta = glm::vec3(1.f) / glm::vec3(1.03f, 1.06f, 1.09f);

	// phong shading spheres
	for (int i = 0; i < 5; ++i) {
		objects.emplace_back(create_sphere(
			glm::vec3(-4.f+i*2.f, 0.5f, 3.5f), 0.5f));
		objects.back()->material->k_a = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f, 0.02f, 0.02f)));
		objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f, 0.5f, 0.5f)));
		if (i > 0) {
			objects.back()->material->k_s = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f, 0.4f, 0.4f)));
			objects.back()->material->n = std::pow(5.f, i);
		}
	}

	lights.emplace_back(new PointLight(glm::vec3(0.f, 8.f, 0.f), glm::vec3(10.f)));
}

void CornellBox::refresh_scene(RaytracingParameters const& params)
{
}

void CornellBox::init_camera(RaytracingParameters& params)
{
    //camera = std::make_shared<LookAroundCamera>(
    //    glm::vec3(0.f, 5.0f, 16.f),
    //    glm::vec3(0.f, 5.0f, 0.f),
    //    params.eye_separation);
    camera = std::make_shared<FreeFlightCamera>(
        glm::vec3(0.f, 5.0f, 16.f),
        glm::vec3(0.f, 0.0f, -1.f),
        params.eye_separation);
}

SpherePortrait::SpherePortrait(RaytracingParameters & params)
{
    init_camera(params);
    init_scene(params);
}

void SpherePortrait::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();

	// floor
    objects.emplace_back(create_quad(
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 100.f),
        glm::vec3(100.f, 0.f, 0.f)));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f, 0.3f, 0.3f)));
    objects.back()->material->k_s = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.3f)));
    objects.back()->material->n = 5;

	// sphere
    objects.emplace_back(create_sphere(glm::vec3(0.f, 4.f, 0.f), 2.f));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));
    objects.emplace_back(create_sphere(glm::vec3(0.f, 7.f, 0.f), 1.f));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));
    objects.emplace_back(create_sphere(glm::vec3(3.f, 4.f, 0.f), 1.f));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));
    objects.emplace_back(create_sphere(glm::vec3(-3.f, 4.f, 0.f), 1.f));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));
    objects.emplace_back(create_sphere(glm::vec3(0.f, 4.f, 3.f), 1.f));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));
    objects.emplace_back(create_sphere(glm::vec3(0.f, 4.f, -3.f), 1.f));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));
    objects.emplace_back(create_sphere(glm::vec3(0.f, 0.f, 0.f), 3.f));
    objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));
}

void SpherePortrait::refresh_scene(RaytracingParameters const& params)
{
	lights.clear();

	// rim light
	lights.emplace_back(new SpotLight(
		glm::vec3(0.0f, 10.f, -40.0f),
		glm::vec3(2000.f),
		glm::normalize(glm::vec3(0.f, -10.f, 40.f)),
		params.spot_light_falloff));

	// fill light
	lights.emplace_back(new PointLight(glm::vec3(-10.f, 3.f, 10.f), 3.f*glm::vec3(10.f, 5.f, 5.f)));
	// key light
	lights.emplace_back(new PointLight(glm::vec3( 10.f, 8.f, 15.f), 15.f*glm::vec3(8.f, 8.f, 10.f)));
}

void SpherePortrait::init_camera(RaytracingParameters& params)
{
    camera = std::make_shared<LookAroundCamera>(
        glm::vec3(7.f, 10.0f, 20.f),
        glm::vec3(-3.f, 3.0f, 0.f),
        params.eye_separation);
}

