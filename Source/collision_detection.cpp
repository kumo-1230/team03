#include "collision_detection.h"
#include "sphere_collider.h"
#include "box_collider.h"
#include "aabb_collider.h"
#include "capsule_collider.h"
#include "game_object.h"
#include <algorithm>
#include <vector>

namespace CollisionDetection {

    // ============================================================================
    // ƒwƒ‹ƒp[ŠÖ”
    // ============================================================================

    static DirectX::XMVECTOR ClosestPointOnSegment(
        DirectX::FXMVECTOR point,
        DirectX::FXMVECTOR segment_start,
        DirectX::FXMVECTOR segment_end) {
        DirectX::XMVECTOR ab = DirectX::XMVectorSubtract(segment_end, segment_start);
        DirectX::XMVECTOR ap = DirectX::XMVectorSubtract(point, segment_start);

        float t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ap, ab)) /
            DirectX::XMVectorGetX(DirectX::XMVector3Dot(ab, ab));

        t = max(0.0f, min(1.0f, t));

        return DirectX::XMVectorAdd(segment_start, DirectX::XMVectorScale(ab, t));
    }

    // ============================================================================
    // Sphere vs Sphere
    // ============================================================================

    bool CheckSphereVsSphereSimple(
        const SphereCollider* sphere_a,
        const SphereCollider* sphere_b) {
        DirectX::XMFLOAT3 center_a = sphere_a->GetWorldCenter();
        DirectX::XMFLOAT3 center_b = sphere_b->GetWorldCenter();

        DirectX::XMVECTOR position_a = DirectX::XMLoadFloat3(&center_a);
        DirectX::XMVECTOR position_b = DirectX::XMLoadFloat3(&center_b);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(position_b, position_a);

        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));
        float radius_sum = sphere_a->GetRadius() + sphere_b->GetRadius();

        return distance < radius_sum;
    }

    bool CheckSphereVsSphereRigidbody(
        const SphereCollider* sphere_a,
        const SphereCollider* sphere_b,
        DirectX::XMFLOAT3& out_correction) {
        DirectX::XMFLOAT3 center_a = sphere_a->GetWorldCenter();
        DirectX::XMFLOAT3 center_b = sphere_b->GetWorldCenter();

        DirectX::XMVECTOR position_a = DirectX::XMLoadFloat3(&center_a);
        DirectX::XMVECTOR position_b = DirectX::XMLoadFloat3(&center_b);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(position_b, position_a);

        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));
        float radius_sum = sphere_a->GetRadius() + sphere_b->GetRadius();

        if (distance < radius_sum) {
            float penetration_depth = radius_sum - distance;
            if (distance > 0.0001f) {
                DirectX::XMVECTOR collision_normal = DirectX::XMVector3Normalize(difference);
                DirectX::XMVECTOR correction_vector = DirectX::XMVectorScale(collision_normal, penetration_depth);
                DirectX::XMStoreFloat3(&out_correction, correction_vector);
            }
            else {
                out_correction = { 0, penetration_depth, 0 };
            }
            return true;
        }

        return false;
    }

    // ============================================================================
    // Sphere vs AABB
    // ============================================================================

    bool CheckSphereVsAABBSimple(
        const SphereCollider* sphere,
        const AABBCollider* aabb) {
        DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
        DirectX::XMFLOAT3 aabb_min, aabb_max;
        aabb->GetWorldBounds(aabb_min, aabb_max);

        DirectX::XMFLOAT3 closest_point = {
            max(aabb_min.x, min(sphere_center.x, aabb_max.x)),
            max(aabb_min.y, min(sphere_center.y, aabb_max.y)),
            max(aabb_min.z, min(sphere_center.z, aabb_max.z))
        };

        DirectX::XMVECTOR sphere_center_vec = DirectX::XMLoadFloat3(&sphere_center);
        DirectX::XMVECTOR closest_point_vec = DirectX::XMLoadFloat3(&closest_point);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(sphere_center_vec, closest_point_vec);

        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));

        return distance < sphere->GetRadius();
    }

    bool CheckSphereVsAABBRigidbody(
        const SphereCollider* sphere,
        const AABBCollider* aabb,
        DirectX::XMFLOAT3& out_correction) {
        DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
        DirectX::XMFLOAT3 aabb_min, aabb_max;
        aabb->GetWorldBounds(aabb_min, aabb_max);

        DirectX::XMFLOAT3 closest_point = {
            max(aabb_min.x, min(sphere_center.x, aabb_max.x)),
            max(aabb_min.y, min(sphere_center.y, aabb_max.y)),
            max(aabb_min.z, min(sphere_center.z, aabb_max.z))
        };

        DirectX::XMVECTOR sphere_center_vec = DirectX::XMLoadFloat3(&sphere_center);
        DirectX::XMVECTOR closest_point_vec = DirectX::XMLoadFloat3(&closest_point);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(sphere_center_vec, closest_point_vec);

        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));

        if (distance < sphere->GetRadius()) {
            float penetration_depth = sphere->GetRadius() - distance;
            if (distance > 0.0001f) {
                DirectX::XMVECTOR collision_normal = DirectX::XMVector3Normalize(difference);
                DirectX::XMVECTOR correction_vector = DirectX::XMVectorScale(collision_normal, penetration_depth);
                DirectX::XMStoreFloat3(&out_correction, correction_vector);
            }
            else {
                out_correction = { 0, penetration_depth, 0 };
            }
            return true;
        }

        return false;
    }

    // ============================================================================
    // Sphere vs OBB (uses existing Box implementation)
    // ============================================================================

    bool CheckSphereVsBoxSimple(
        const SphereCollider* sphere,
        const BoxCollider* box) {

        DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
        DirectX::XMVECTOR sphere_center_vec = DirectX::XMLoadFloat3(&sphere_center);

        DirectX::XMMATRIX box_world = box->GetOwner()
            ? box->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();

        DirectX::XMFLOAT3 box_offset = box->GetOffset();
        DirectX::XMVECTOR offset_vec = DirectX::XMLoadFloat3(&box_offset);
        DirectX::XMMATRIX offset_matrix = DirectX::XMMatrixTranslationFromVector(offset_vec);
        DirectX::XMMATRIX final_box_transform = offset_matrix * box_world;

        DirectX::XMMATRIX box_inverse = DirectX::XMMatrixInverse(nullptr, final_box_transform);
        DirectX::XMVECTOR local_sphere_center = DirectX::XMVector3TransformCoord(
            sphere_center_vec, box_inverse);

        DirectX::XMFLOAT3 box_size = box->GetSize();
        DirectX::XMFLOAT3 half_extents = {
            box_size.x * 0.5f,
            box_size.y * 0.5f,
            box_size.z * 0.5f
        };

        DirectX::XMFLOAT3 local_center;
        DirectX::XMStoreFloat3(&local_center, local_sphere_center);

        DirectX::XMFLOAT3 closest_point = {
            max(-half_extents.x, min(local_center.x, half_extents.x)),
            max(-half_extents.y, min(local_center.y, half_extents.y)),
            max(-half_extents.z, min(local_center.z, half_extents.z))
        };

        DirectX::XMVECTOR closest_vec = DirectX::XMLoadFloat3(&closest_point);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(local_sphere_center, closest_vec);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));

        return distance < sphere->GetRadius();
    }

    bool CheckSphereVsBoxRigidbody(
        const SphereCollider* sphere,
        const BoxCollider* box,
        DirectX::XMFLOAT3& out_correction) {

        DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
        DirectX::XMVECTOR sphere_center_vec = DirectX::XMLoadFloat3(&sphere_center);

        DirectX::XMMATRIX box_world = box->GetOwner()
            ? box->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();

        DirectX::XMFLOAT3 box_offset = box->GetOffset();
        DirectX::XMVECTOR offset_vec = DirectX::XMLoadFloat3(&box_offset);
        DirectX::XMMATRIX offset_matrix = DirectX::XMMatrixTranslationFromVector(offset_vec);
        DirectX::XMMATRIX final_box_transform = offset_matrix * box_world;

        DirectX::XMMATRIX box_inverse = DirectX::XMMatrixInverse(nullptr, final_box_transform);
        DirectX::XMVECTOR local_sphere_center = DirectX::XMVector3TransformCoord(
            sphere_center_vec, box_inverse);

        DirectX::XMFLOAT3 box_size = box->GetSize();
        DirectX::XMFLOAT3 half_extents = {
            box_size.x * 0.5f,
            box_size.y * 0.5f,
            box_size.z * 0.5f
        };

        DirectX::XMFLOAT3 local_center;
        DirectX::XMStoreFloat3(&local_center, local_sphere_center);

        DirectX::XMFLOAT3 closest_point = {
            max(-half_extents.x, min(local_center.x, half_extents.x)),
            max(-half_extents.y, min(local_center.y, half_extents.y)),
            max(-half_extents.z, min(local_center.z, half_extents.z))
        };

        DirectX::XMVECTOR closest_vec = DirectX::XMLoadFloat3(&closest_point);
        DirectX::XMVECTOR local_difference = DirectX::XMVectorSubtract(local_sphere_center, closest_vec);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(local_difference));

        if (distance < sphere->GetRadius()) {
            float penetration_depth = sphere->GetRadius() - distance;

            if (distance > 0.0001f) {
                DirectX::XMVECTOR local_normal = DirectX::XMVector3Normalize(local_difference);
                DirectX::XMVECTOR world_normal = DirectX::XMVector3TransformNormal(
                    local_normal, final_box_transform);
                world_normal = DirectX::XMVector3Normalize(world_normal);

                DirectX::XMVECTOR correction_vector = DirectX::XMVectorScale(
                    world_normal, penetration_depth);
                DirectX::XMStoreFloat3(&out_correction, correction_vector);
            }
            else {
                DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
                DirectX::XMVECTOR world_up = DirectX::XMVector3TransformNormal(up, final_box_transform);
                world_up = DirectX::XMVector3Normalize(world_up);
                DirectX::XMVECTOR correction_vector = DirectX::XMVectorScale(
                    world_up, penetration_depth);
                DirectX::XMStoreFloat3(&out_correction, correction_vector);
            }
            return true;
        }

        return false;
    }

    // ============================================================================
    // Sphere vs Cylinder (Capsule)
    // ============================================================================

    bool CheckSphereVsCylinderSimple(
        const SphereCollider* sphere,
        const CapsuleCollider* cylinder) {
        DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
        DirectX::XMFLOAT3 cylinder_start, cylinder_end;
        cylinder->GetCylinderSegment(cylinder_start, cylinder_end);

        DirectX::XMVECTOR sphere_position = DirectX::XMLoadFloat3(&sphere_center);
        DirectX::XMVECTOR segment_start = DirectX::XMLoadFloat3(&cylinder_start);
        DirectX::XMVECTOR segment_end = DirectX::XMLoadFloat3(&cylinder_end);

        DirectX::XMVECTOR closest_point = ClosestPointOnSegment(sphere_position, segment_start, segment_end);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(sphere_position, closest_point);

        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));
        float radius_sum = sphere->GetRadius() + cylinder->GetRadius();

        return distance < radius_sum;
    }

    bool CheckSphereVsCylinderRigidbody(
        const SphereCollider* sphere,
        const CapsuleCollider* cylinder,
        DirectX::XMFLOAT3& out_correction) {
        DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
        DirectX::XMFLOAT3 cylinder_start, cylinder_end;
        cylinder->GetCylinderSegment(cylinder_start, cylinder_end);

        DirectX::XMVECTOR sphere_position = DirectX::XMLoadFloat3(&sphere_center);
        DirectX::XMVECTOR segment_start = DirectX::XMLoadFloat3(&cylinder_start);
        DirectX::XMVECTOR segment_end = DirectX::XMLoadFloat3(&cylinder_end);

        DirectX::XMVECTOR closest_point = ClosestPointOnSegment(sphere_position, segment_start, segment_end);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(sphere_position, closest_point);

        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));
        float radius_sum = sphere->GetRadius() + cylinder->GetRadius();

        if (distance < radius_sum) {
            float penetration_depth = radius_sum - distance;
            if (distance > 0.0001f) {
                DirectX::XMVECTOR collision_normal = DirectX::XMVector3Normalize(difference);
                DirectX::XMVECTOR correction_vector = DirectX::XMVectorScale(collision_normal, penetration_depth);
                DirectX::XMStoreFloat3(&out_correction, correction_vector);
            }
            else {
                out_correction = { 0, penetration_depth, 0 };
            }
            return true;
        }

        return false;
    }

    // ============================================================================
    // AABB vs AABB
    // ============================================================================

    bool CheckAABBVsAABBSimple(
        const AABBCollider* aabb_a,
        const AABBCollider* aabb_b) {
        DirectX::XMFLOAT3 min_a, max_a, min_b, max_b;
        aabb_a->GetWorldBounds(min_a, max_a);
        aabb_b->GetWorldBounds(min_b, max_b);

        return (min_a.x <= max_b.x && max_a.x >= min_b.x) &&
            (min_a.y <= max_b.y && max_a.y >= min_b.y) &&
            (min_a.z <= max_b.z && max_a.z >= min_b.z);
    }

    bool CheckAABBVsAABBRigidbody(
        const AABBCollider* aabb_a,
        const AABBCollider* aabb_b,
        DirectX::XMFLOAT3& out_correction) {
        DirectX::XMFLOAT3 min_a, max_a, min_b, max_b;
        aabb_a->GetWorldBounds(min_a, max_a);
        aabb_b->GetWorldBounds(min_b, max_b);

        DirectX::XMFLOAT3 center_a = {
            (min_a.x + max_a.x) * 0.5f,
            (min_a.y + max_a.y) * 0.5f,
            (min_a.z + max_a.z) * 0.5f
        };
        DirectX::XMFLOAT3 center_b = {
            (min_b.x + max_b.x) * 0.5f,
            (min_b.y + max_b.y) * 0.5f,
            (min_b.z + max_b.z) * 0.5f
        };

        DirectX::XMFLOAT3 half_extents_a = {
            (max_a.x - min_a.x) * 0.5f,
            (max_a.y - min_a.y) * 0.5f,
            (max_a.z - min_a.z) * 0.5f
        };
        DirectX::XMFLOAT3 half_extents_b = {
            (max_b.x - min_b.x) * 0.5f,
            (max_b.y - min_b.y) * 0.5f,
            (max_b.z - min_b.z) * 0.5f
        };

        DirectX::XMFLOAT3 difference = {
            center_b.x - center_a.x,
            center_b.y - center_a.y,
            center_b.z - center_a.z
        };

        float overlap_x = (half_extents_a.x + half_extents_b.x) - abs(difference.x);
        float overlap_y = (half_extents_a.y + half_extents_b.y) - abs(difference.y);
        float overlap_z = (half_extents_a.z + half_extents_b.z) - abs(difference.z);

        if (overlap_x > 0 && overlap_y > 0 && overlap_z > 0) {
            if (overlap_x < overlap_y && overlap_x < overlap_z) {
                out_correction = {
                    (difference.x > 0 ? 1.0f : -1.0f) * overlap_x,
                    0,
                    0
                };
            }
            else if (overlap_y < overlap_z) {
                out_correction = {
                    0,
                    (difference.y > 0 ? 1.0f : -1.0f) * overlap_y,
                    0
                };
            }
            else {
                out_correction = {
                    0,
                    0,
                    (difference.z > 0 ? 1.0f : -1.0f) * overlap_z
                };
            }
            return true;
        }

        return false;
    }

    // ============================================================================
    // OBB vs AABB
    // ============================================================================

    bool CheckBoxVsAABBSimple(
        const BoxCollider* box,
        const AABBCollider* aabb) {
        // Convert AABB to OBB for unified testing
        DirectX::XMFLOAT3 aabb_min, aabb_max;
        aabb->GetWorldBounds(aabb_min, aabb_max);

        DirectX::XMFLOAT3 aabb_center = {
            (aabb_min.x + aabb_max.x) * 0.5f,
            (aabb_min.y + aabb_max.y) * 0.5f,
            (aabb_min.z + aabb_max.z) * 0.5f
        };

        DirectX::XMFLOAT3 aabb_size = {
            aabb_max.x - aabb_min.x,
            aabb_max.y - aabb_min.y,
            aabb_max.z - aabb_min.z
        };

        // Get OBB vertices
        std::vector<DirectX::XMFLOAT3> box_vertices_vec = box->GetWorldVertices();
        DirectX::XMFLOAT3 box_vertices[8];
        std::copy(box_vertices_vec.begin(), box_vertices_vec.end(), box_vertices);

        // Create AABB vertices
        DirectX::XMFLOAT3 half_size = {
            aabb_size.x * 0.5f,
            aabb_size.y * 0.5f,
            aabb_size.z * 0.5f
        };

        DirectX::XMFLOAT3 aabb_vertices[8] = {
            {aabb_center.x - half_size.x, aabb_center.y - half_size.y, aabb_center.z - half_size.z},
            {aabb_center.x + half_size.x, aabb_center.y - half_size.y, aabb_center.z - half_size.z},
            {aabb_center.x + half_size.x, aabb_center.y + half_size.y, aabb_center.z - half_size.z},
            {aabb_center.x - half_size.x, aabb_center.y + half_size.y, aabb_center.z - half_size.z},
            {aabb_center.x - half_size.x, aabb_center.y - half_size.y, aabb_center.z + half_size.z},
            {aabb_center.x + half_size.x, aabb_center.y - half_size.y, aabb_center.z + half_size.z},
            {aabb_center.x + half_size.x, aabb_center.y + half_size.y, aabb_center.z + half_size.z},
            {aabb_center.x - half_size.x, aabb_center.y + half_size.y, aabb_center.z + half_size.z}
        };

        // Get OBB axes
        DirectX::XMMATRIX box_transform = box->GetOwner()
            ? box->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();

        DirectX::XMVECTOR box_axes[3] = {
            DirectX::XMVector3Normalize(box_transform.r[0]),
            DirectX::XMVector3Normalize(box_transform.r[1]),
            DirectX::XMVector3Normalize(box_transform.r[2])
        };

        // AABB axes (world axes)
        DirectX::XMVECTOR aabb_axes[3] = {
            DirectX::XMVectorSet(1, 0, 0, 0),
            DirectX::XMVectorSet(0, 1, 0, 0),
            DirectX::XMVectorSet(0, 0, 1, 0)
        };

        // Test OBB face normals
        for (int i = 0; i < 3; ++i) {
            if (!TestSeparatingAxis(box_axes[i], box_vertices, aabb_vertices)) {
                return false;
            }
        }

        // Test AABB face normals
        for (int i = 0; i < 3; ++i) {
            if (!TestSeparatingAxis(aabb_axes[i], box_vertices, aabb_vertices)) {
                return false;
            }
        }

        // Test edge cross products
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                DirectX::XMVECTOR cross_axis = DirectX::XMVector3Cross(box_axes[i], aabb_axes[j]);
                float length_squared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(cross_axis));

                if (length_squared < 0.0001f) continue;

                cross_axis = DirectX::XMVector3Normalize(cross_axis);
                if (!TestSeparatingAxis(cross_axis, box_vertices, aabb_vertices)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool CheckBoxVsAABBRigidbody(
        const BoxCollider* box,
        const AABBCollider* aabb,
        DirectX::XMFLOAT3& out_correction) {
        // Similar to simple version but tracks penetration
        DirectX::XMFLOAT3 aabb_min, aabb_max;
        aabb->GetWorldBounds(aabb_min, aabb_max);

        DirectX::XMFLOAT3 aabb_center = {
            (aabb_min.x + aabb_max.x) * 0.5f,
            (aabb_min.y + aabb_max.y) * 0.5f,
            (aabb_min.z + aabb_max.z) * 0.5f
        };

        DirectX::XMFLOAT3 aabb_size = {
            aabb_max.x - aabb_min.x,
            aabb_max.y - aabb_min.y,
            aabb_max.z - aabb_min.z
        };

        std::vector<DirectX::XMFLOAT3> box_vertices_vec = box->GetWorldVertices();
        DirectX::XMFLOAT3 box_vertices[8];
        std::copy(box_vertices_vec.begin(), box_vertices_vec.end(), box_vertices);

        DirectX::XMFLOAT3 half_size = {
            aabb_size.x * 0.5f,
            aabb_size.y * 0.5f,
            aabb_size.z * 0.5f
        };

        DirectX::XMFLOAT3 aabb_vertices[8] = {
            {aabb_center.x - half_size.x, aabb_center.y - half_size.y, aabb_center.z - half_size.z},
            {aabb_center.x + half_size.x, aabb_center.y - half_size.y, aabb_center.z - half_size.z},
            {aabb_center.x + half_size.x, aabb_center.y + half_size.y, aabb_center.z - half_size.z},
            {aabb_center.x - half_size.x, aabb_center.y + half_size.y, aabb_center.z - half_size.z},
            {aabb_center.x - half_size.x, aabb_center.y - half_size.y, aabb_center.z + half_size.z},
            {aabb_center.x + half_size.x, aabb_center.y - half_size.y, aabb_center.z + half_size.z},
            {aabb_center.x + half_size.x, aabb_center.y + half_size.y, aabb_center.z + half_size.z},
            {aabb_center.x - half_size.x, aabb_center.y + half_size.y, aabb_center.z + half_size.z}
        };

        DirectX::XMMATRIX box_transform = box->GetOwner()
            ? box->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();

        DirectX::XMVECTOR box_axes[3] = {
            DirectX::XMVector3Normalize(box_transform.r[0]),
            DirectX::XMVector3Normalize(box_transform.r[1]),
            DirectX::XMVector3Normalize(box_transform.r[2])
        };

        DirectX::XMVECTOR aabb_axes[3] = {
            DirectX::XMVectorSet(1, 0, 0, 0),
            DirectX::XMVectorSet(0, 1, 0, 0),
            DirectX::XMVectorSet(0, 0, 1, 0)
        };

        float min_penetration_depth = FLT_MAX;
        DirectX::XMVECTOR minimum_translation_axis = DirectX::XMVectorZero();

        auto TestAxisWithPenetration = [&](const DirectX::XMVECTOR& axis) -> bool {
            float min_box = FLT_MAX;
            float max_box = -FLT_MAX;
            float min_aabb = FLT_MAX;
            float max_aabb = -FLT_MAX;

            for (int i = 0; i < 8; ++i) {
                DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&box_vertices[i]);
                float projection = DirectX::XMVectorGetX(DirectX::XMVector3Dot(vertex, axis));
                min_box = min(min_box, projection);
                max_box = max(max_box, projection);
            }

            for (int i = 0; i < 8; ++i) {
                DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&aabb_vertices[i]);
                float projection = DirectX::XMVectorGetX(DirectX::XMVector3Dot(vertex, axis));
                min_aabb = min(min_aabb, projection);
                max_aabb = max(max_aabb, projection);
            }

            if (max_box < min_aabb || max_aabb < min_box) {
                return false;
            }

            float penetration = min(max_box - min_aabb, max_aabb - min_box);
            if (penetration < min_penetration_depth) {
                min_penetration_depth = penetration;

                DirectX::XMFLOAT3 box_center = box->GetWorldCenter();
                DirectX::XMVECTOR center_difference = DirectX::XMVectorSubtract(
                    DirectX::XMLoadFloat3(&aabb_center),
                    DirectX::XMLoadFloat3(&box_center)
                );

                float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(axis, center_difference));
                minimum_translation_axis = (dot > 0) ? axis : DirectX::XMVectorNegate(axis);
            }

            return true;
            };

        for (int i = 0; i < 3; ++i) {
            if (!TestAxisWithPenetration(box_axes[i])) return false;
        }

        for (int i = 0; i < 3; ++i) {
            if (!TestAxisWithPenetration(aabb_axes[i])) return false;
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                DirectX::XMVECTOR cross_axis = DirectX::XMVector3Cross(box_axes[i], aabb_axes[j]);
                float length_squared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(cross_axis));

                if (length_squared < 0.0001f) continue;

                cross_axis = DirectX::XMVector3Normalize(cross_axis);
                if (!TestAxisWithPenetration(cross_axis)) return false;
            }
        }

        DirectX::XMVECTOR correction = DirectX::XMVectorScale(
            minimum_translation_axis,
            min_penetration_depth
        );
        DirectX::XMStoreFloat3(&out_correction, correction);

        return true;
    }

    // ============================================================================
    // OBB vs OBB (SAT - Separating Axis Theorem)
    // ============================================================================

    static bool TestSeparatingAxis(
        const DirectX::XMVECTOR& axis,
        const DirectX::XMFLOAT3 vertices_a[8],
        const DirectX::XMFLOAT3 vertices_b[8]) {

        float min_a = FLT_MAX;
        float max_a = -FLT_MAX;
        float min_b = FLT_MAX;
        float max_b = -FLT_MAX;

        for (int i = 0; i < 8; ++i) {
            DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&vertices_a[i]);
            float projection = DirectX::XMVectorGetX(DirectX::XMVector3Dot(vertex, axis));
            min_a = min(min_a, projection);
            max_a = max(max_a, projection);
        }

        for (int i = 0; i < 8; ++i) {
            DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&vertices_b[i]);
            float projection = DirectX::XMVectorGetX(DirectX::XMVector3Dot(vertex, axis));
            min_b = min(min_b, projection);
            max_b = max(max_b, projection);
        }

        return !(max_a < min_b || max_b < min_a);
    }

    bool CheckBoxVsBoxSimple(
        const BoxCollider* box_a,
        const BoxCollider* box_b) {

        std::vector<DirectX::XMFLOAT3> vertices_a_vec = box_a->GetWorldVertices();
        std::vector<DirectX::XMFLOAT3> vertices_b_vec = box_b->GetWorldVertices();

        DirectX::XMFLOAT3 vertices_a[8];
        DirectX::XMFLOAT3 vertices_b[8];
        std::copy(vertices_a_vec.begin(), vertices_a_vec.end(), vertices_a);
        std::copy(vertices_b_vec.begin(), vertices_b_vec.end(), vertices_b);

        DirectX::XMMATRIX transform_a = box_a->GetOwner()
            ? box_a->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX transform_b = box_b->GetOwner()
            ? box_b->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();

        DirectX::XMVECTOR axes_a[3] = {
            DirectX::XMVector3Normalize(transform_a.r[0]),
            DirectX::XMVector3Normalize(transform_a.r[1]),
            DirectX::XMVector3Normalize(transform_a.r[2])
        };

        DirectX::XMVECTOR axes_b[3] = {
            DirectX::XMVector3Normalize(transform_b.r[0]),
            DirectX::XMVector3Normalize(transform_b.r[1]),
            DirectX::XMVector3Normalize(transform_b.r[2])
        };

        // Test 15 potential separating axes
        for (int i = 0; i < 3; ++i) {
            if (!TestSeparatingAxis(axes_a[i], vertices_a, vertices_b)) {
                return false;
            }
        }

        for (int i = 0; i < 3; ++i) {
            if (!TestSeparatingAxis(axes_b[i], vertices_a, vertices_b)) {
                return false;
            }
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                DirectX::XMVECTOR cross_axis = DirectX::XMVector3Cross(axes_a[i], axes_b[j]);
                float length_squared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(cross_axis));

                if (length_squared < 0.0001f) continue;

                cross_axis = DirectX::XMVector3Normalize(cross_axis);
                if (!TestSeparatingAxis(cross_axis, vertices_a, vertices_b)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool CheckBoxVsBoxRigidbody(
        const BoxCollider* box_a,
        const BoxCollider* box_b,
        DirectX::XMFLOAT3& out_correction) {

        std::vector<DirectX::XMFLOAT3> vertices_a_vec = box_a->GetWorldVertices();
        std::vector<DirectX::XMFLOAT3> vertices_b_vec = box_b->GetWorldVertices();

        DirectX::XMFLOAT3 vertices_a[8];
        DirectX::XMFLOAT3 vertices_b[8];
        std::copy(vertices_a_vec.begin(), vertices_a_vec.end(), vertices_a);
        std::copy(vertices_b_vec.begin(), vertices_b_vec.end(), vertices_b);

        DirectX::XMMATRIX transform_a = box_a->GetOwner()
            ? box_a->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX transform_b = box_b->GetOwner()
            ? box_b->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();

        DirectX::XMVECTOR axes_a[3] = {
            DirectX::XMVector3Normalize(transform_a.r[0]),
            DirectX::XMVector3Normalize(transform_a.r[1]),
            DirectX::XMVector3Normalize(transform_a.r[2])
        };

        DirectX::XMVECTOR axes_b[3] = {
            DirectX::XMVector3Normalize(transform_b.r[0]),
            DirectX::XMVector3Normalize(transform_b.r[1]),
            DirectX::XMVector3Normalize(transform_b.r[2])
        };

        float min_penetration_depth = FLT_MAX;
        DirectX::XMVECTOR minimum_translation_axis = DirectX::XMVectorZero();

        auto TestAxisWithPenetration = [&](const DirectX::XMVECTOR& axis) -> bool {
            float min_a = FLT_MAX;
            float max_a = -FLT_MAX;
            float min_b = FLT_MAX;
            float max_b = -FLT_MAX;

            for (int i = 0; i < 8; ++i) {
                DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&vertices_a[i]);
                float projection = DirectX::XMVectorGetX(DirectX::XMVector3Dot(vertex, axis));
                min_a = min(min_a, projection);
                max_a = max(max_a, projection);
            }

            for (int i = 0; i < 8; ++i) {
                DirectX::XMVECTOR vertex = DirectX::XMLoadFloat3(&vertices_b[i]);
                float projection = DirectX::XMVectorGetX(DirectX::XMVector3Dot(vertex, axis));
                min_b = min(min_b, projection);
                max_b = max(max_b, projection);
            }

            if (max_a < min_b || max_b < min_a) {
                return false;
            }

            float penetration = min(max_a - min_b, max_b - min_a);
            if (penetration < min_penetration_depth) {
                min_penetration_depth = penetration;

                DirectX::XMFLOAT3 center_a = box_a->GetWorldCenter();
                DirectX::XMFLOAT3 center_b = box_b->GetWorldCenter();
                DirectX::XMVECTOR center_difference = DirectX::XMVectorSubtract(
                    DirectX::XMLoadFloat3(&center_b),
                    DirectX::XMLoadFloat3(&center_a)
                );

                float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(axis, center_difference));
                minimum_translation_axis = (dot > 0) ? axis : DirectX::XMVectorNegate(axis);
            }

            return true;
            };

        for (int i = 0; i < 3; ++i) {
            if (!TestAxisWithPenetration(axes_a[i])) return false;
        }

        for (int i = 0; i < 3; ++i) {
            if (!TestAxisWithPenetration(axes_b[i])) return false;
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                DirectX::XMVECTOR cross_axis = DirectX::XMVector3Cross(axes_a[i], axes_b[j]);
                float length_squared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(cross_axis));

                if (length_squared < 0.0001f) continue;

                cross_axis = DirectX::XMVector3Normalize(cross_axis);
                if (!TestAxisWithPenetration(cross_axis)) return false;
            }
        }

        DirectX::XMVECTOR correction = DirectX::XMVectorScale(
            minimum_translation_axis,
            min_penetration_depth
        );
        DirectX::XMStoreFloat3(&out_correction, correction);

        return true;
    }

    // ============================================================================
    // OBB vs Cylinder (approximated with closest point on cylinder axis)
    // ============================================================================

    bool CheckBoxVsCylinderSimple(
        const BoxCollider* box,
        const CapsuleCollider* cylinder) {
        // Get cylinder segment
        DirectX::XMFLOAT3 cylinder_start, cylinder_end;
        cylinder->GetCylinderSegment(cylinder_start, cylinder_end);

        DirectX::XMVECTOR segment_start = DirectX::XMLoadFloat3(&cylinder_start);
        DirectX::XMVECTOR segment_end = DirectX::XMLoadFloat3(&cylinder_end);

        // Get OBB vertices
        std::vector<DirectX::XMFLOAT3> box_vertices_vec = box->GetWorldVertices();

        // Find closest point on cylinder axis to any OBB vertex
        float min_distance = FLT_MAX;

        for (const auto& vertex : box_vertices_vec) {
            DirectX::XMVECTOR vertex_vec = DirectX::XMLoadFloat3(&vertex);
            DirectX::XMVECTOR closest_on_segment = ClosestPointOnSegment(
                vertex_vec, segment_start, segment_end);
            DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(vertex_vec, closest_on_segment);
            float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));
            min_distance = min(min_distance, distance);
        }

        // Also check if cylinder endpoints are inside the OBB
        DirectX::XMFLOAT3 box_center = box->GetWorldCenter();
        DirectX::XMFLOAT3 box_size = box->GetSize();
        DirectX::XMMATRIX box_transform = box->GetOwner()
            ? box->GetOwner()->GetWorldTransformMatrix()
            : DirectX::XMMatrixIdentity();
        DirectX::XMMATRIX box_inverse = DirectX::XMMatrixInverse(nullptr, box_transform);

        // Transform cylinder endpoints to OBB local space
        DirectX::XMVECTOR local_start = DirectX::XMVector3TransformCoord(segment_start, box_inverse);
        DirectX::XMVECTOR local_end = DirectX::XMVector3TransformCoord(segment_end, box_inverse);

        DirectX::XMFLOAT3 half_size = {
            box_size.x * 0.5f,
            box_size.y * 0.5f,
            box_size.z * 0.5f
        };

        auto IsPointInBox = [&half_size](DirectX::XMVECTOR point) -> bool {
            float x = DirectX::XMVectorGetX(point);
            float y = DirectX::XMVectorGetY(point);
            float z = DirectX::XMVectorGetZ(point);
            return abs(x) <= half_size.x && abs(y) <= half_size.y && abs(z) <= half_size.z;
            };

        if (IsPointInBox(local_start) || IsPointInBox(local_end)) {
            return true;
        }

        return min_distance < cylinder->GetRadius();
    }

    bool CheckBoxVsCylinderRigidbody(
        const BoxCollider* box,
        const CapsuleCollider* cylinder,
        DirectX::XMFLOAT3& out_correction) {
        // Simplified approximation using closest point approach
        DirectX::XMFLOAT3 cylinder_start, cylinder_end;
        cylinder->GetCylinderSegment(cylinder_start, cylinder_end);

        DirectX::XMVECTOR segment_start = DirectX::XMLoadFloat3(&cylinder_start);
        DirectX::XMVECTOR segment_end = DirectX::XMLoadFloat3(&cylinder_end);
        DirectX::XMVECTOR cylinder_center = DirectX::XMVectorScale(
            DirectX::XMVectorAdd(segment_start, segment_end), 0.5f);

        DirectX::XMFLOAT3 box_center = box->GetWorldCenter();
        DirectX::XMVECTOR box_center_vec = DirectX::XMLoadFloat3(&box_center);

        // Find closest point on cylinder axis to box center
        DirectX::XMVECTOR closest_on_cylinder = ClosestPointOnSegment(
            box_center_vec, segment_start, segment_end);

        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(box_center_vec, closest_on_cylinder);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));

        // Approximate box as sphere with radius = half diagonal
        DirectX::XMFLOAT3 box_size = box->GetSize();
        float box_radius = sqrtf(box_size.x * box_size.x +
            box_size.y * box_size.y +
            box_size.z * box_size.z) * 0.5f;

        float combined_radius = cylinder->GetRadius() + box_radius;

        if (distance < combined_radius) {
            float penetration_depth = combined_radius - distance;
            if (distance > 0.0001f) {
                DirectX::XMVECTOR collision_normal = DirectX::XMVector3Normalize(difference);
                DirectX::XMVECTOR correction_vector = DirectX::XMVectorScale(
                    collision_normal, penetration_depth);
                DirectX::XMStoreFloat3(&out_correction, correction_vector);
            }
            else {
                out_correction = { 0, penetration_depth, 0 };
            }
            return true;
        }

        return false;
    }

    // ============================================================================
    // AABB vs Cylinder
    // ============================================================================

    bool CheckAABBVsCylinderSimple(
        const AABBCollider* aabb,
        const CapsuleCollider* cylinder) {
        // Get cylinder segment
        DirectX::XMFLOAT3 cylinder_start, cylinder_end;
        cylinder->GetCylinderSegment(cylinder_start, cylinder_end);

        DirectX::XMVECTOR segment_start = DirectX::XMLoadFloat3(&cylinder_start);
        DirectX::XMVECTOR segment_end = DirectX::XMLoadFloat3(&cylinder_end);

        // Get AABB bounds
        DirectX::XMFLOAT3 aabb_min, aabb_max;
        aabb->GetWorldBounds(aabb_min, aabb_max);

        // Check if cylinder axis intersects AABB
        DirectX::XMFLOAT3 aabb_center = {
            (aabb_min.x + aabb_max.x) * 0.5f,
            (aabb_min.y + aabb_max.y) * 0.5f,
            (aabb_min.z + aabb_max.z) * 0.5f
        };
        DirectX::XMVECTOR aabb_center_vec = DirectX::XMLoadFloat3(&aabb_center);

        // Find closest point on cylinder axis to AABB center
        DirectX::XMVECTOR closest_point = ClosestPointOnSegment(
            aabb_center_vec, segment_start, segment_end);

        // Clamp closest point to AABB
        DirectX::XMFLOAT3 closest_float;
        DirectX::XMStoreFloat3(&closest_float, closest_point);

        DirectX::XMFLOAT3 clamped = {
            max(aabb_min.x, min(closest_float.x, aabb_max.x)),
            max(aabb_min.y, min(closest_float.y, aabb_max.y)),
            max(aabb_min.z, min(closest_float.z, aabb_max.z))
        };

        DirectX::XMVECTOR clamped_vec = DirectX::XMLoadFloat3(&clamped);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(closest_point, clamped_vec);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));

        return distance < cylinder->GetRadius();
    }

    bool CheckAABBVsCylinderRigidbody(
        const AABBCollider* aabb,
        const CapsuleCollider* cylinder,
        DirectX::XMFLOAT3& out_correction) {
        DirectX::XMFLOAT3 cylinder_start, cylinder_end;
        cylinder->GetCylinderSegment(cylinder_start, cylinder_end);

        DirectX::XMVECTOR segment_start = DirectX::XMLoadFloat3(&cylinder_start);
        DirectX::XMVECTOR segment_end = DirectX::XMLoadFloat3(&cylinder_end);

        DirectX::XMFLOAT3 aabb_min, aabb_max;
        aabb->GetWorldBounds(aabb_min, aabb_max);

        DirectX::XMFLOAT3 aabb_center = {
            (aabb_min.x + aabb_max.x) * 0.5f,
            (aabb_min.y + aabb_max.y) * 0.5f,
            (aabb_min.z + aabb_max.z) * 0.5f
        };
        DirectX::XMVECTOR aabb_center_vec = DirectX::XMLoadFloat3(&aabb_center);

        DirectX::XMVECTOR closest_point = ClosestPointOnSegment(
            aabb_center_vec, segment_start, segment_end);

        DirectX::XMFLOAT3 closest_float;
        DirectX::XMStoreFloat3(&closest_float, closest_point);

        DirectX::XMFLOAT3 clamped = {
            max(aabb_min.x, min(closest_float.x, aabb_max.x)),
            max(aabb_min.y, min(closest_float.y, aabb_max.y)),
            max(aabb_min.z, min(closest_float.z, aabb_max.z))
        };

        DirectX::XMVECTOR clamped_vec = DirectX::XMLoadFloat3(&clamped);
        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(closest_point, clamped_vec);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));

        if (distance < cylinder->GetRadius()) {
            float penetration_depth = cylinder->GetRadius() - distance;
            if (distance > 0.0001f) {
                DirectX::XMVECTOR collision_normal = DirectX::XMVector3Normalize(difference);
                DirectX::XMVECTOR correction_vector = DirectX::XMVectorScale(
                    collision_normal, penetration_depth);
                DirectX::XMStoreFloat3(&out_correction, correction_vector);
            }
            else {
                out_correction = { 0, penetration_depth, 0 };
            }
            return true;
        }

        return false;
    }

    // ============================================================================
    // Cylinder vs Cylinder
    // ============================================================================

    bool CheckCylinderVsCylinderSimple(
        const CapsuleCollider* cylinder_a,
        const CapsuleCollider* cylinder_b) {
        DirectX::XMFLOAT3 a_start, a_end, b_start, b_end;
        cylinder_a->GetCylinderSegment(a_start, a_end);
        cylinder_b->GetCylinderSegment(b_start, b_end);

        DirectX::XMVECTOR a_start_vec = DirectX::XMLoadFloat3(&a_start);
        DirectX::XMVECTOR a_end_vec = DirectX::XMLoadFloat3(&a_end);
        DirectX::XMVECTOR b_start_vec = DirectX::XMLoadFloat3(&b_start);
        DirectX::XMVECTOR b_end_vec = DirectX::XMLoadFloat3(&b_end);

        DirectX::XMVECTOR closest_a = ClosestPointOnSegment(b_start_vec, a_start_vec, a_end_vec);
        DirectX::XMVECTOR closest_b = ClosestPointOnSegment(closest_a, b_start_vec, b_end_vec);
        closest_a = ClosestPointOnSegment(closest_b, a_start_vec, a_end_vec);

        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(closest_b, closest_a);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));
        float radius_sum = cylinder_a->GetRadius() + cylinder_b->GetRadius();

        return distance < radius_sum;
    }

    bool CheckCylinderVsCylinderRigidbody(
        const CapsuleCollider* cylinder_a,
        const CapsuleCollider* cylinder_b,
        DirectX::XMFLOAT3& out_correction) {
        DirectX::XMFLOAT3 a_start, a_end, b_start, b_end;
        cylinder_a->GetCylinderSegment(a_start, a_end);
        cylinder_b->GetCylinderSegment(b_start, b_end);

        DirectX::XMVECTOR a_start_vec = DirectX::XMLoadFloat3(&a_start);
        DirectX::XMVECTOR a_end_vec = DirectX::XMLoadFloat3(&a_end);
        DirectX::XMVECTOR b_start_vec = DirectX::XMLoadFloat3(&b_start);
        DirectX::XMVECTOR b_end_vec = DirectX::XMLoadFloat3(&b_end);

        DirectX::XMVECTOR closest_a = ClosestPointOnSegment(b_start_vec, a_start_vec, a_end_vec);
        DirectX::XMVECTOR closest_b = ClosestPointOnSegment(closest_a, b_start_vec, b_end_vec);
        closest_a = ClosestPointOnSegment(closest_b, a_start_vec, a_end_vec);

        DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(closest_b, closest_a);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));
        float radius_sum = cylinder_a->GetRadius() + cylinder_b->GetRadius();

        if (distance < radius_sum) {
            float penetration_depth = radius_sum - distance;
            if (distance > 0.0001f) {
                DirectX::XMVECTOR collision_normal = DirectX::XMVector3Normalize(difference);
                DirectX::XMVECTOR correction_vector = DirectX::XMVectorScale(collision_normal, penetration_depth);
                DirectX::XMStoreFloat3(&out_correction, correction_vector);
            }
            else {
                out_correction = { 0, penetration_depth, 0 };
            }
            return true;
        }

        return false;
    }
}  // namespace CollisionDetection
