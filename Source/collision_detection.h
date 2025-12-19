#ifndef COLLISION_DETECTION_H_
#define COLLISION_DETECTION_H_

#include <DirectXMath.h>

// ëOï˚êÈåæ
class SphereCollider;
class BoxCollider;
class AABBCollider;
class CylinderCollider;

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
bool CheckSphereVsAABBSimple(
    const SphereCollider* sphere,
    const AABBCollider* aabb);

bool CheckSphereVsAABBRigidbody(
    const SphereCollider* sphere,
    const AABBCollider* aabb,
    DirectX::XMFLOAT3& out_correction);

// Sphere vs Cylinder (Cylinder)
bool CheckSphereVsCylinderSimple(
    const SphereCollider* sphere,
    const CylinderCollider* cylinder);

bool CheckSphereVsCylinderRigidbody(
    const SphereCollider* sphere,
    const CylinderCollider* cylinder,
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
bool CheckBoxVsAABBSimple(
    const BoxCollider* box,
    const AABBCollider* aabb);

bool CheckBoxVsAABBRigidbody(
    const BoxCollider* box,
    const AABBCollider* aabb,
    DirectX::XMFLOAT3& out_correction);

// Box (OBB) vs Cylinder (Cylinder)
bool CheckBoxVsCylinderSimple(
    const BoxCollider* box,
    const CylinderCollider* cylinder);

bool CheckBoxVsCylinderRigidbody(
    const BoxCollider* box,
    const CylinderCollider* cylinder,
    DirectX::XMFLOAT3& out_correction);

// AABB vs AABB
bool CheckAABBVsAABBSimple(
    const AABBCollider* aabb_a,
    const AABBCollider* aabb_b);

bool CheckAABBVsAABBRigidbody(
    const AABBCollider* aabb_a,
    const AABBCollider* aabb_b,
    DirectX::XMFLOAT3& out_correction);

// AABB vs Cylinder (Cylinder)
bool CheckAABBVsCylinderSimple(
    const AABBCollider* aabb,
    const CylinderCollider* cylinder);

bool CheckAABBVsCylinderRigidbody(
    const AABBCollider* aabb,
    const CylinderCollider* cylinder,
    DirectX::XMFLOAT3& out_correction);

// Cylinder (Cylinder) vs Cylinder (Cylinder)
bool CheckCylinderVsCylinderSimple(
    const CylinderCollider* cylinder_a,
    const CylinderCollider* cylinder_b);

bool CheckCylinderVsCylinderRigidbody(
    const CylinderCollider* cylinder_a,
    const CylinderCollider* cylinder_b,
    DirectX::XMFLOAT3& out_correction);

}  // namespace CollisionDetection

#endif  // COLLISION_DETECTION_H_
