#include <cglib/rt/scene.h>

#include <cglib/rt/epsilon.h>
#include <cglib/rt/light.h>
#include <cglib/rt/object.h>
#include <cglib/rt/raytracing_parameters.h>
#include <cglib/rt/texture.h>

#include <cglib/rt/transform.h>

#include <cglib/rt/bvh.h>
#include <cglib/rt/triangle_soup.h>

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

TriangleScene::TriangleScene(RaytracingParameters& params)
{
    init_camera(params);
    init_scene(params);
}

std::shared_ptr<TriangleSoup> createTriangleSoup(int num_triangles)
{
	std::vector<glm::vec3> positions;
	for (int i = 0; i < num_triangles; ++i)
	{
		positions.push_back(glm::vec3(
			(i+1)*0.1*std::sin(float(2*i)/num_triangles*2*M_PI)-0.5, 
			(i+1)*0.1*std::sin(float(4*i)/num_triangles*2*M_PI)-0.5, 
			1.0-i*0.2));
		positions.push_back(glm::vec3(
			(i+1)*0.1*std::sin(float(2*i)/num_triangles*2*M_PI)+0.5,
			(i+1)*0.1*std::sin(float(4*i)/num_triangles*2*M_PI)-0.5,
			1.0-i*0.2));
		positions.push_back(glm::vec3(
			(i+1)*0.1*std::sin(float(2*i)/num_triangles*2*M_PI),
			(i+1)*0.1*std::sin(float(4*i)/num_triangles*2*M_PI)+0.5,
			1.0-i*0.2));
	}
	std::vector<glm::vec2> tex_coords;
	for (int i = 0; i < num_triangles; ++i) {
		tex_coords.push_back(glm::vec2(0, 0)); 
		tex_coords.push_back(glm::vec2(1, 1));
		tex_coords.push_back(glm::vec2(1, 0));
	}
	std::vector<glm::vec3> normals(num_triangles*3, glm::vec3(0, 0, 1));

	std::vector<Material> materials(7);
	materials[0].k_d = std::make_shared<ConstTexture>(glm::vec3(0.9f, 0.9f, 0.9f));
	materials[1].k_d = std::make_shared<ConstTexture>(glm::vec3(0.1f, 0.1f, 0.9f));
	materials[2].k_d = std::make_shared<ConstTexture>(glm::vec3(0.1f, 0.9f, 0.1f));
	materials[3].k_d = std::make_shared<ConstTexture>(glm::vec3(0.1f, 0.9f, 0.9f));
	materials[4].k_d = std::make_shared<ConstTexture>(glm::vec3(0.9f, 0.1f, 0.1f));
	materials[5].k_d = std::make_shared<ConstTexture>(glm::vec3(0.9f, 0.1f, 0.9f));
	materials[6].k_d = std::make_shared<ConstTexture>(glm::vec3(0.9f, 0.9f, 0.1f));
	std::vector<int> material_ids;
	for (int i = 0; i < num_triangles; ++i) {
		material_ids.push_back(i%(materials.size()-1));
	}

	return std::make_shared<TriangleSoup>(
		std::move(positions),
		std::move(normals),
		std::move(tex_coords),
		std::move(material_ids),
		std::move(materials));
}

void TriangleScene::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();
    textures.clear();
    soups.clear();

	soups.emplace_back(createTriangleSoup(params.num_triangles));
    objects.emplace_back(new BVH(*soups.back()));
    lights.emplace_back(new PointLight(glm::vec3(0.f, 200.f, 400.f), glm::vec3(15000.f)));
}

void TriangleScene::refresh_scene(RaytracingParameters const& params)
{
    soups.clear();
    objects.clear();
    
	soups.emplace_back(createTriangleSoup(params.num_triangles));
	objects.emplace_back(new BVH(*soups.back()));
}

void TriangleScene::init_camera(RaytracingParameters& params)
{
    camera = std::make_shared<LookAroundCamera>(
        glm::vec3(0.f, 0.f, 3.f),
        glm::vec3(0.f, 0.f, 0.5f),
        params.eye_separation);
}

MonkeyScene::MonkeyScene(RaytracingParameters& params)
{
    init_camera(params);
    init_scene(params);
}

void MonkeyScene::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();
    textures.clear();
    soups.clear();

    textures.insert({"floor", std::make_shared<ImageTexture>(
		"assets/checker.tga", params.tex_filter_mode, 
		params.tex_wrap_mode, 2.2f)});

	std::shared_ptr<Image> appartment = std::make_shared<Image>();
	appartment->load("assets/appartment.jpg", 1.f);
    textures.insert({"appartment_env",          
		std::make_shared<ImageTexture>(*appartment,
		BILINEAR, REPEAT)});
	env_map = textures["appartment_env"].get();

    soups.push_back(std::make_shared<TriangleSoup>(
		"assets/suzanne.obj", &this->textures));
    objects.emplace_back(new BVH(*soups.back()));
	objects.back()->set_transform_object_to_world(
		glm::translate(glm::vec3(0.f, 2.f, 0.f)) * 
		glm::scale(glm::vec3(3.f, 3.f, 3.f)));
    objects.emplace_back((create_plane(
        glm::vec3(0.f, -2.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, -1.f),
        glm::vec2(4.f))));
    objects.back()->material->k_d = textures["floor"];

    lights.emplace_back(new PointLight(
		glm::vec3(0.f, 6.f, 12.f), glm::vec3(3.f)));
    lights.emplace_back(new PointLight(
		glm::vec3(0.f, 12.f, 6.f), glm::vec3(3.f)));

}

void MonkeyScene::refresh_scene(RaytracingParameters const& params)
{
	if (params.filtered_envmap) {
		auto fem = textures["appartment_env_filtered"].get();
		// filter on demand
		if (!fem) {
			auto appartment_filtered = textures["appartment_env"]->get_mip_levels()[0]->filter_gaussian_separable(10, 41, Image::REPEAT);
			auto appartment_filtered_tex = std::make_shared<ImageTexture>(*appartment_filtered, BILINEAR, REPEAT);
			textures["appartment_env_filtered"] = appartment_filtered_tex;
			fem = appartment_filtered_tex.get();
		}
		env_map = fem;
	}
	else {
		env_map = textures["appartment_env"].get();
	}
}

void MonkeyScene::init_camera(RaytracingParameters& params)
{
    camera = std::make_shared<LookAroundCamera>(
        glm::vec3(0.f, 2.f, 10.f),
        glm::vec3(0.f, 2.f, -1.f),
        params.eye_separation);
}

SponzaScene::SponzaScene(RaytracingParameters& params)
{
	init_camera(params);
	init_scene(params);
}

void SponzaScene::init_scene(RaytracingParameters const& params)
{
	objects.clear();
	lights.clear();
	textures.clear();
	soups.clear();

	for (int i = 0; i < 4; ++i) {
		objects.emplace_back(create_sphere(glm::vec3(-3.f*i, 0.6f, -0.3f), 0.5f));
		objects.back()->material->k_d = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.0f)));
		objects.back()->material->k_r = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.4f)));
	}

	auto objTriangles = std::make_shared<TriangleSoup>("assets/crytek-sponza/sponza_subdiv3.obj", &this->textures);
	soups.push_back(objTriangles);
	objects.emplace_back(new BVH(*objTriangles));
	objects.back()->set_transform_object_to_world(
		glm::scale(glm::vec3(0.01f)));

	for (int i = 0; i < 2; ++i) {
		lights.emplace_back(new PointLight(glm::vec3(-2.5f+5.f*i, 6.f, 0.f), glm::vec3(10.f)));
	}

}

void SponzaScene::refresh_scene(RaytracingParameters const& params)
{
	for (auto &tex : textures) {
		tex.second->filter_mode = params.tex_filter_mode;
		tex.second->wrap_mode = params.tex_wrap_mode;
	}
}

void SponzaScene::init_camera(RaytracingParameters& params)
{
	camera = std::make_shared<FreeFlightCamera>(
		glm::vec3(4.f, 5.f, -0.3f),
		glm::normalize(glm::vec3(-1.f, -0.5f, 0.f)),
		params.eye_separation);
}

