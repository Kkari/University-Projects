#pragma once

#include <cglib/rt/texture.h>

#include <vector>
#include <memory>

class Camera;
class Light;
class AreaLight;
class Object;
class RaytracingParameters;
class TriangleSoup;

/*
 * Scene.
 *
 * This is the all the stuff that will be raytraced (camera, objects, materials, ...)
 * -- Use init_context to initially create objects and load textures. 
 *    This function will be called on startup and when parameters changed
 * -- Use init_camera to set up the camera.
 *
 */
class Scene
{
public:
	std::shared_ptr<Camera> camera;
	std::vector<std::unique_ptr<Light>> lights;
	std::vector<std::unique_ptr<Object>> objects;
	TextureContainer textures;
	ImageTexture* env_map = nullptr;
	std::vector<std::shared_ptr<TriangleSoup>> soups;

    virtual ~Scene();

	virtual void init_scene(RaytracingParameters const& params) = 0;
    virtual void refresh_scene(RaytracingParameters const& params) = 0;
	virtual void init_camera(RaytracingParameters& params) = 0;
	virtual void set_active_camera();
};


class TextureScene : public Scene
{
public:
    TextureScene(RaytracingParameters& params);

	void init_scene(RaytracingParameters const& params);
    void refresh_scene(RaytracingParameters const& params);
	void init_camera(RaytracingParameters& params);
};

class TexturedSphereScene : public Scene
{
public:
    TexturedSphereScene(RaytracingParameters& params);

	void init_scene(RaytracingParameters const& params);
    void refresh_scene(RaytracingParameters const& params);
	void init_camera(RaytracingParameters& params);
};

class AliasingPlaneScene : public Scene
{
public:
    AliasingPlaneScene(RaytracingParameters& params);

	void init_scene(RaytracingParameters const& params);
    void refresh_scene(RaytracingParameters const& params);
	void init_camera(RaytracingParameters& params);
};

class PoolTableScene : public Scene
{
public:
	PoolTableScene(RaytracingParameters& params);

	void init_scene(RaytracingParameters const& params);
    void refresh_scene(RaytracingParameters const& params);
	void init_camera(RaytracingParameters& params);
};

class GoBoardScene : public Scene
{
public:
	GoBoardScene(RaytracingParameters& params);

	void init_scene(RaytracingParameters const& params);
    void refresh_scene(RaytracingParameters const& params);
	void init_camera(RaytracingParameters& params);
};

class MonkeyScene : public Scene
{
public:
    MonkeyScene(RaytracingParameters& params);

	void init_scene(RaytracingParameters const& params);
    void refresh_scene(RaytracingParameters const& params);
	void init_camera(RaytracingParameters& params);
};

class SponzaScene : public Scene
{
public:
    SponzaScene(RaytracingParameters& params);

	void init_scene(RaytracingParameters const& params);
    void refresh_scene(RaytracingParameters const& params);
	void init_camera(RaytracingParameters& params);
};

class TriangleScene : public Scene
{
public:
    TriangleScene(RaytracingParameters& params);

	void init_scene(RaytracingParameters const& params);
    void refresh_scene(RaytracingParameters const& params);
	void init_camera(RaytracingParameters& params);
};

