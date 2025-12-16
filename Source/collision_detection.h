#ifndef COLLISION_DETECTION_H_
#define COLLISION_DETECTION_H_

#include <DirectXMath.h>

// ëOï˚êÈåæ
class SphereCollider;
class BoxCollider;
class AABBCollider;
class CapsuleCollider;

// ============================================================================
// Collision Detection Helper Functions
// ============================================================================
namespace CollisionDetection {

static bool TestSeparatingAxis(const DirectX::XMVECTOR& axis, const DirectX::XMFLOAT3 vertices_a[8], const DirectX::XMFLOAT3 vertices_b[8]);

// Sphere vs Sphere
bool CheckSphereVsSphereSimple(
    const SphereCollider* sphere_a,
    const SphereCollider* sphere_b);

bool CheckSphereVsSphereRigidbody(
    const SphereCollider* sphere_a,
    const SphereCollider* sphere_b,
    DirectX::XMFLOAT3& out_correction);

// Sphere vs Box (OBB)
bool CheckSphereVsBoxSimple(
    const SphereCollider* sphere,
    const BoxCollider* box);

bool CheckSphereVsBoxRigidbody(
    const SphereCollider* sphere,
    const BoxCollider* box,
    DirectX::XMFLOAT3& out_correction);

// Sphere vs AABB
bool CheckSphereVsAabbSimple(
    const SphereCollider* sphere,
    const AABBCollider* aabb);

bool CheckSphereVsAabbRigidbody(
    const SphereCollider* sphere,
    const AABBCollider* aabb,
    DirectX::XMFLOAT3& out_correction);

// Sphere vs Cylinder (Capsule)
bool CheckSphereVsCylinderSimple(
    const SphereCollider* sphere,
    const CapsuleCollider* cylinder);

bool CheckSphereVsCylinderRigidbody(
    const SphereCollider* sphere,
    const CapsuleCollider* cylinder,
    DirectX::XMFLOAT3& out_correction);


// Box (OBB) vs Box (OBB)
bool CheckBoxVsBoxSimple(
    const BoxCollider* box_a,
    const BoxCollider* box_b);

bool CheckBoxVsBoxRigidbody(
    const BoxCollider* box_a,
    const BoxCollider* box_b,
    DirectX::XMFLOAT3& out_correction);

// Box (OBB) vs AABB
bool CheckBoxVsAabbSimple(
    const BoxCollider* box,
    const AABBCollider* aabb);

bool CheckBoxVsAabbRigidbody(
    const BoxCollider* box,
    const AABBCollider* aabb,
    DirectX::XMFLOAT3& out_correction);

// Box (OBB) vs Cylinder (Capsule)
bool CheckBoxVsCylinderSimple(
    const BoxCollider* box,
    const CapsuleCollider* cylinder);

bool CheckBoxVsCylinderRigidbody(
    const BoxCollider* box,
    const CapsuleCollider* cylinder,
    DirectX::XMFLOAT3& out_correction);

// AABB vs AABB
bool CheckAabbVsAabbSimple(
    const AABBCollider* aabb_a,
    const AABBCollider* aabb_b);

bool CheckAabbVsAabbRigidbody(
    const AABBCollider* aabb_a,
    const AABBCollider* aabb_b,
    DirectX::XMFLOAT3& out_correction);

// AABB vs Cylinder (Capsule)
bool CheckAabbVsCylinderSimple(
    const AABBCollider* aabb,
    const CapsuleCollider* cylinder);

bool CheckAabbVsCylinderRigidbody(
    const AABBCollider* aabb,
    const CapsuleCollider* cylinder,
    DirectX::XMFLOAT3& out_correction);

// Cylinder (Capsule) vs Cylinder (Capsule)
bool CheckCylinderVsCylinderSimple(
    const CapsuleCollider* cylinder_a,
    const CapsuleCollider* cylinder_b);

bool CheckCylinderVsCylinderRigidbody(
    const CapsuleCollider* cylinder_a,
    const CapsuleCollider* cylinder_b,
    DirectX::XMFLOAT3& out_correction);

}  // namespace CollisionDetection

#endif  // COLLISION_DETECTION_H_
