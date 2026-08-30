#include "FlyCamera.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>

void 
FlyCamera::update(bool moveForward, bool moveBackward,
                  bool moveLeft, bool moveRight, float deltaTime,
                  float mouseDeltaX, float mouseDeltaY)
{
	mYaw   = std::fmod(mYaw + mouseDeltaX * glm::two_pi<float>(), glm::two_pi<float>());
	mPitch = std::clamp(mPitch + mouseDeltaY * glm::pi<float>(), -glm::pi<float>() * 0.9f, glm::pi<float>() * 0.9f);

	auto rotation = glm::quat(glm::vec3(-mPitch, -mYaw, 0.f));
    auto forward  = glm::normalize(rotation * glm::vec3(0.f, 0.f, -1.f));
    auto right    = glm::normalize(glm::cross(forward, mWorldUp));

    // Keyboard movement
    float velocity = mMovementSpeed * deltaTime;

    if (moveForward)
        mPosition += forward * velocity;
    if (moveBackward)
        mPosition -= forward * velocity;
    if (moveLeft)
        mPosition -= right * velocity;
    if (moveRight)
        mPosition += right * velocity;

    auto T = glm::translate(mPosition);
    auto R = glm::toMat4(rotation);
    auto S = glm::mat4(1.f);

    mLocalToWorldMatrix = T * R * S;
}

glm::mat4
FlyCamera::viewMatrix() const
{
    return glm::inverse(mLocalToWorldMatrix);
}
