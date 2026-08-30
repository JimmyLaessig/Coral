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

private:

    glm::vec3 mVelocity{ 0.f, 0.f, 0.f };

    glm::vec3 mPosition{ 0.f, 2.f, 2.f };
    
    glm::mat4 mLocalToWorldMatrix{ 1.f };

    float mYaw{ 0.f };
    float mPitch{ 0.f};

    float mMovementSpeed{ 1.f };
    float mMouseSensitivity{ 1.f };

    const glm::vec3 mWorldUp{ 0.f, 1.f, 0.f };

}; // class FlyCamera

#endif FLYCAMERA_HPP