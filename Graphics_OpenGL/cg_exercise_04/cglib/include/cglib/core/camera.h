#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <iostream>

class Camera
{
public:
	enum Mode
	{
		Mono = 0,
		StereoLeft,
		StereoRight
	};

public:
	Camera() : m_up(0.f, 1.f, 0.f) {}

	inline glm::vec3 const& get_position(Mode mode) const
	{
		return m_position[mode];
	}

	inline glm::vec3 const& get_direction() const
	{
		return m_viewDirection;
	}

	inline glm::mat4 const& get_view_matrix(Mode mode) const
	{
		return m_viewMatrix[mode];
	}

	inline glm::mat4 const& get_inverse_view_matrix(Mode mode) const
	{
		return m_invViewMatrix[mode];
	}

	inline bool requires_restart()
	{
        const bool restart = m_requiresRestart;
        m_requiresRestart = false;
		return restart;
	}

	inline void set_requires_restart(bool rr)
	{
		m_requiresRestart = rr;
	}

	inline float get_eye_separation() const
	{
		return m_eyeSeparation;
	}

	inline void set_eye_separation(float eyeSep)
	{
		m_eyeSeparation = eyeSep;
		updateStereoMatrices();
	}
	
	virtual void update() = 0;
	/* update depending on frametime */
	virtual void update_time_dependant(float frame_time) { update(); }
    
	virtual bool handle_key_event(int key, int action) = 0;
	virtual bool handle_mouse_button_event(int button, int action) = 0;
	virtual bool handle_mouse_motion_event(int x, int y) = 0;
	virtual bool handle_mouse_wheel_event(float x, float y) = 0;
    virtual bool handle_mouse_drag_event(float x, float y) = 0;

	static Camera * get_active();
	void set_active();

	float m_speed;

protected:
	void updateStereoMatrices();

protected:
    bool       m_requiresRestart;
	float      m_eyeSeparation;

	// These are triples because we support stereo rendering. Layout
	// is {Mono, Stereo Left, Stereo Right}.
	glm::vec3 m_up;
	glm::vec3 m_viewDirection;
	glm::vec3 m_position[3];
	glm::mat4 m_viewMatrix[3];
	glm::mat4 m_invViewMatrix[3];
	static Camera *active_camera;
};

class LookAroundCamera : public Camera
{
public:
	LookAroundCamera();
	LookAroundCamera(glm::vec3 const& position, glm::vec3 const& center, float eye_separation);

	void update() override;
	
	bool handle_key_event(int key, int action) override;
	bool handle_mouse_button_event(int button, int action) override;
	bool handle_mouse_motion_event(int x, int y) override;
	bool handle_mouse_wheel_event(float x, float y) override;
	bool handle_mouse_drag_event(float x, float y) override;

private:
    glm::vec3 m_center;
    float m_phi, m_theta, m_r;
};

class FreeFlightCamera : public Camera
{
public:
	FreeFlightCamera();
	FreeFlightCamera(glm::vec3 const& position, glm::vec3 const& view, float eye_separation);

	void update() override;

	bool handle_key_event(int key, int action) override;
	bool handle_mouse_button_event(int button, int action) override;
	bool handle_mouse_motion_event(int x, int y) override;
	bool handle_mouse_wheel_event(float x, float y) override;
	bool handle_mouse_drag_event(float x, float y) override;

protected:
	glm::vec3 m_right;
	float m_yaw;
	float m_pitch;

};

class RTFreeFlightCamera : public FreeFlightCamera
{
public:
	RTFreeFlightCamera() : FreeFlightCamera() {}
	RTFreeFlightCamera(glm::vec3 const& position, glm::vec3 const& view, float eye_separation)
		: FreeFlightCamera(position, view, eye_separation) {}
	bool handle_key_event(int key, int action) override;

	void update_time_dependant(float t) override;

protected:
	bool up       = false;
	bool down     = false;
	bool left     = false;
	bool right    = false;
	bool forward  = false;
	bool backward = false;
};
