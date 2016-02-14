#include <cglib/core/camera.h>
#include <cglib/core/glheaders.h>
#include <cglib/core/glmstream.h>
#include <cglib/core/glheaders.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <algorithm>
#include <cmath>

Camera *Camera::active_camera = nullptr;

Camera *Camera::get_active()
{
	return active_camera;
}

void Camera::set_active()
{
	active_camera = this;
}

void Camera::updateStereoMatrices()
{
	// Using view direction and up vector, derive the direction of eye separation.
	glm::vec3 const eyesepDir = glm::normalize(glm::cross(m_viewDirection, m_up));

	// Left and right center.
	m_position[1] = m_position[0] - 0.5f * m_eyeSeparation * eyesepDir;
	m_position[2] = m_position[0] + 0.5f * m_eyeSeparation * eyesepDir;

	for (int i = 1; i < 3; ++i)
	{
		m_viewMatrix[i]    = glm::lookAt(m_position[i], m_position[i] + m_viewDirection, m_up);
		m_invViewMatrix[i] = glm::affineInverse(m_viewMatrix[i]);
	}

	m_requiresRestart  = true;
}

LookAroundCamera::LookAroundCamera()
	: LookAroundCamera(
		glm::vec3(0.f, 0.f, 1.f),
		glm::vec3(0.f, 0.f, 0.f),
		0.1f)
{
}

LookAroundCamera::LookAroundCamera(
	glm::vec3 const& position, 
	glm::vec3 const& center, 
	float eye_separation)
{
	m_eyeSeparation = eye_separation;
	m_speed = 1.f;
	m_position[0] = position;
	m_center = center;
	m_viewDirection = glm::normalize(m_center - m_position[0]);

	m_r = glm::length(m_position[0] - m_center);
	const glm::vec3 d = glm::normalize(m_position[0] - m_center);
	m_theta = std::acos(d.y);
	m_phi = std::atan(d.z / d.x);

	update();
}

void LookAroundCamera::update()
{
	m_position[0] = m_center + glm::vec3(
		m_r * std::sin(m_theta) * std::cos(m_phi),
		m_r * std::cos(m_theta),
		m_r * std::sin(m_theta) * std::sin(m_phi));
	m_viewDirection = glm::normalize(m_center-m_position[0]);
	m_viewMatrix[0]    = glm::lookAt(m_position[0], m_position[0] + m_viewDirection, m_up);
	m_invViewMatrix[0] = glm::affineInverse(m_viewMatrix[0]);
	updateStereoMatrices();
}

bool LookAroundCamera::handle_key_event(int key, int action)
{
	if (key == GLFW_KEY_P) {
		std::cout << "camera position " << m_position[0] << std::endl;
		return true;
	}

	int forward = int(key == GLFW_KEY_W || key == GLFW_KEY_UP);
	int backward = int(key == GLFW_KEY_S || key == GLFW_KEY_DOWN);
	if (forward || backward) {
		m_r = std::max(0.0001f, m_r - 0.5f * float(forward - backward));
		update();
		return true;
	}

	return false;
}

bool LookAroundCamera::handle_mouse_button_event(int button, int action)
{
	return false;
}

bool LookAroundCamera::handle_mouse_motion_event(int x, int y)
{
	return false;
}

bool LookAroundCamera::handle_mouse_wheel_event(float x, float y) 
{
	m_r = std::max(0.0001f, m_r - 0.5f * y);
	update();
	return true;
}

bool LookAroundCamera::handle_mouse_drag_event(float x, float y) 
{
	m_theta += y * 0.002f;
	m_phi -= x * 0.001f;

	while (m_phi >= 180.f) {
		m_phi -= 360.f;
	}
	while (m_phi < -180.f)
	{
		m_phi += 360.f;
	}
	while (m_theta >= 90.f) {
		m_theta -= 180.f;
	}
	while (m_theta < -90.f)
	{
		m_theta += 180.f;
	}

	update();
	return true;
}

FreeFlightCamera::FreeFlightCamera()
{
	FreeFlightCamera(
		glm::vec3(0.f, 0.f, 2.f),
		glm::vec3(0.f, 0.f, -1.f),
		0.f);
}

FreeFlightCamera::FreeFlightCamera(
	glm::vec3 const& position, 
	glm::vec3 const& view, 
	float eye_separation) :
	m_yaw(0.f),
	m_pitch(0.f)
{

	m_eyeSeparation = eye_separation;
	m_speed = 1.f;
	m_position[0] = position;
	m_viewDirection = glm::normalize(view);

	m_pitch = std::asin(view.y);
	m_yaw = std::atan2(-view.z, view.x) - static_cast<float>(M_PI)/2.f;

	update();
}

void FreeFlightCamera::update()
{
	glm::mat4 camToWorld = glm::translate(m_position[0]) * glm::rotate(m_yaw, glm::vec3(0.0, 1.0, 0.0)) * glm::rotate(m_pitch, glm::vec3(1.0, 0.0, 0.0));
	m_viewDirection = -glm::vec3(camToWorld[2]);
	
	m_viewMatrix[0]    = glm::affineInverse(camToWorld);
	m_invViewMatrix[0] = camToWorld;
	m_right = glm::vec3(m_viewMatrix[0][0][0], m_viewMatrix[0][1][0], m_viewMatrix[0][2][0]);

	updateStereoMatrices();
}

bool FreeFlightCamera::handle_key_event(int key, int action)
{
	if(action == GLFW_RELEASE)
		return false;

	bool handled = false;
	if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
		m_position[0] += m_speed * m_viewDirection;
		handled = true;
	}
	else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
		m_position[0] -= m_speed * m_viewDirection;
		handled = true;
	}
	else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
		m_position[0] -= m_speed * m_right;
		handled = true;
	}
	else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
		m_position[0] += m_speed * m_right;
		handled = true;
	}
	else if (key == GLFW_KEY_Q) {
		m_position[0] -= m_speed * m_up;
		handled = true;
	}
	else if (key == GLFW_KEY_E) {
		m_position[0] += m_speed * m_up;
		handled = true;
	}

	if (handled) {
		update();
	}

	if (key == GLFW_KEY_P) {
		std::cout << "camera position " << m_position[0] << std::endl;
		std::cout << "camera view direction " << m_viewDirection << std::endl;
		handled = true;
	}

	return handled;
}

bool FreeFlightCamera::handle_mouse_button_event(int button, int action)
{
	return false;
}

bool FreeFlightCamera::handle_mouse_motion_event(int x, int y)
{
	return false;
}

bool FreeFlightCamera::handle_mouse_wheel_event(float x, float y)
{
	if (y < 0.f) m_speed /= 1.1f;
	else m_speed *= 1.1f;
	
	return true;
}

bool FreeFlightCamera::handle_mouse_drag_event(float x, float y)
{
	m_yaw += 1.5e-3f * x;
	m_pitch += 1.5e-3f * y;

	m_pitch = std::min(static_cast<float>(M_PI)/2.f - 1e-2f, std::max(-static_cast<float>(M_PI)/2.f + 1e-2f, m_pitch));

	update();
	return true;
}

void RTFreeFlightCamera::
update_time_dependant(float t)
{
	glm::vec4 m(right - left, up - down, backward - forward, 0.0f);
	if(m[0] != 0.0f || m[1] != 0.0f || m[2] != 0.0f) {
		m_position[0] += glm::vec3(
				m_invViewMatrix[0]
				* glm::normalize(m)
				* t * m_speed);
	}

	FreeFlightCamera::update();
}

bool RTFreeFlightCamera::
handle_key_event(int key, int action)
{
	switch(key) {
	case GLFW_KEY_W:
	case GLFW_KEY_UP:
		if(action == GLFW_PRESS)
			forward = true;
		else if(action == GLFW_RELEASE)
			forward = false;
		return true;
	case GLFW_KEY_A:
	case GLFW_KEY_LEFT:
		if(action == GLFW_PRESS)
			left = true;
		else if(action == GLFW_RELEASE)
			left = false;
		return true;
	case GLFW_KEY_S:
	case GLFW_KEY_DOWN:
		if(action == GLFW_PRESS)
			backward = true;
		else if(action == GLFW_RELEASE)
			backward = false;
		return true;
	case GLFW_KEY_D:
	case GLFW_KEY_RIGHT:
		if(action == GLFW_PRESS)
			right = true;
		else if(action == GLFW_RELEASE)
			right = false;
		return true;
	case GLFW_KEY_Q:
		if(action == GLFW_PRESS)
			down = true;
		else if(action == GLFW_RELEASE)
			down = false;
		return true;
	case GLFW_KEY_E:
		if(action == GLFW_PRESS)
			up = true;
		else if(action == GLFW_RELEASE)
			up = false;
		return true;
	case GLFW_KEY_P:
		if(action == GLFW_PRESS) {
			std::cout << "camera position " << m_position[0] << std::endl;
			std::cout << "camera view direction " << m_viewDirection << std::endl;
		}
		return true;
	default:
		return false;
	}

	return false;
}
