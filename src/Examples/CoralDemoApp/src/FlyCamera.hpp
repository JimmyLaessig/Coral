#ifndef FLYCAMERA_HPP
#define FLYCAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class FlyCamera
{
public:

    void update(bool moveForward, bool moveBackward, 
                bool moveLeft, bool moveRight, float deltaTime, 
                float mouseDeltaX, float mouseDeltaY);

    glm::mat4 viewMatrix() const;

    glm::mat4 projectionMatrix() const;

    void setSize(uint32_t width, uint32_t height);

private:

    glm::vec3 mVelocity{ 0.f, 0.f, 0.f };

    glm::vec3 mPosition{ 0.f, 2.f, 2.f };
    
    glm::mat4 mLocalToWorldMatrix{ 1.f };

    float mYaw{ 0.f };
    float mPitch{ 0.f};

    float mMovementSpeed{ 1.f };
    float mMouseSensitivity{ 1.f };

    const glm::vec3 mWorldUp{ 0.f, 1.f, 0.f };

    float mFov       = glm::radians(65.f);
    float mNearPlane = 0.01f;
    float mFarPlane  = 1000.f;

    uint32_t mWidth  = 1024;
    uint32_t mHeight = 768;

}; // class FlyCamera

#endif FLYCAMERA_HPP