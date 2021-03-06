#include "CameraControl.hpp"
#include "../headers/SCECore.hpp"

using namespace SCE;
using namespace std;


CameraControl::CameraControl(SCE::SCEHandle<Container> container, SCE::SCEHandle<Transform> target,
                             vec3 distFromTarget)
    : GameObject(container, "CamControl"), mTarget(target), mDistanceFromTarget(distFromTarget),
      mLookAheadTarget(0.0f, 1.0f, 10.0f)
{  
    mAverageOffset = mTarget->Forward()*mLookAheadTarget.z + mTarget->Up()*mLookAheadTarget.y;;
}

void CameraControl::Update()
{
    //move camera
    SCEHandle<Transform> transform = GetContainer()->GetComponent<Transform>();
    glm::vec3 targetPos = mTarget->Forward()*mDistanceFromTarget.z + mTarget->Up()*mDistanceFromTarget.y;
    targetPos += mTarget->GetScenePosition();
    transform->SetScenePosition(targetPos);

    glm::vec3 lookAtTarget = mTarget->Forward()*mLookAheadTarget.z + mTarget->Up()*mLookAheadTarget.y;

    float avgDuration = 0.1f;
    float deltaTime = float(SCE::Time::DeltaTime());
    mAverageOffset = (lookAtTarget*deltaTime + mAverageOffset*avgDuration)
            / (avgDuration + deltaTime);

//    mAverageOffset = lookAtTarget;
    glm::vec3 dirToTarget = mTarget->GetScenePosition() + mAverageOffset - targetPos;
    dirToTarget = normalize(dirToTarget);
    float smoothStrength = (1.0f - (dot(dirToTarget, transform->Forward())));
    transform->SmoothLookAt(mAverageOffset + mTarget->GetScenePosition(), 0.9f + smoothStrength*0.1f);
}

