// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once
#include "PhysicsCommands.h"
#include "PhysicsPose.h"
#include "PhysicsActorDescriptor.h"
#include "PhysicsShapeTypes.h"
#include "PhysicsFilterData.h"
//temporary until we add the query abstraction
#include "PhysicsQueryTypes.h"

namespace PhysicsCommands
{
    namespace ShapeData
    {
        // Retrieves the actor descriptor (handle, user data, type) for the body this shape is attached to.
        struct GetActorDescriptor : Command
        {
            static constexpr auto command = Shape::GetActorDescriptor;
            Physics::ActorDescriptor value;
        };

        // Retrieves the engine-side user data pointer associated with this shape.
        struct GetUserData : Command
        {
            static constexpr auto command = Shape::GetUserData;
            void* value;
        };

        // Retrieves the current shape flags (e.g. SceneQuery, Trigger).
        struct GetFlags : Command
        {
            static constexpr auto command = Shape::GetFlags;

            Physics::ShapeFlags flags;
        };

        // Sets shape flags (e.g. SceneQuery, Trigger) controlling how the shape participates in simulation and queries.
        struct SetFlags : Command
        {
            static constexpr auto command = Shape::SetFlags;

            Physics::ShapeFlags flags;
        };

        // Retrieves the collision filter data for this shape, used by the broadphase to determine which shapes can collide.
        struct GetFilterData : Command
        {
            static constexpr auto command = Shape::GetFilterData;

            Physics::FilterData data;
        };

        // Sets the collision filter data for this shape, controlling layer-based collision filtering and pair event flags.
        struct SetFilterData : Command
        {
            static constexpr auto command = Shape::SetFilterData;

            Physics::FilterData data;
        };

        // Enables or disables contact modification for this shape, allowing runtime adjustment of contact points.
        struct SetModifiableContacts : Command
        {
            static constexpr auto command = Shape::SetModifiableContacts;
            bool modifiable;
        };

        // Configures which collision pair events (e.g. contact, trigger, CCD) this shape generates for a given layer.
        struct SetSupportedMessages : Command
        {
            static constexpr auto command = Shape::SetSupportedMessages;
            Physics::ShapePairEventFlags flags;
            int layer;
        };

        // Assigns a physics material to this shape, controlling friction and bounciness.
        struct SetMaterial : Command
        {
            static constexpr auto command = Shape::SetMaterial;
            void* value;
        };

        // Retrieves the shape's pose in the owning body's local space.
        struct GetLocalPose : Command
        {
            static constexpr auto command = Shape::GetLocalPose;
            Physics::Pose value;
        };

        // Retrieves the shape's current world-space pose.
        struct GetPose : Command
        {
            static constexpr auto command = Shape::GetPose;
            Physics::Pose value;
        };

        // Sets the shape's pose in the owning body's local space.
        struct SetPose : Command
        {
            static constexpr auto command = Shape::SetPose;
            Physics::Pose value;
        };

        // Sets the contact offset (skin width) for the shape. Contacts are generated when shapes are within this distance.
        struct SetContactOffset : Command
        {
            static constexpr auto command = Shape::SetContactOffset;
            float value;
        };

        // Retrieves the geometry type (Sphere, Box, Capsule, ConvexMesh, TriangleMesh, or Terrain) of this shape.
        struct GetGeometryType : Command
        {
            static constexpr auto command = Shape::GetGeometryType;
            Physics::GeometryType value;
        };

        // Extracts the raw mesh data (vertices, triangles) from a collision mesh shape via a callback.
        struct ExtractCollisionMeshData : Command
        {
            static constexpr auto command = Shape::ExtractCollisionMeshData;

            void* ctx;
            Physics::CollisionMeshDataExtractionCallback callback;
        };

        // Creates a new collision shape with the given geometry, material, user data, and flags.
        // Returns the shape handle via 'value'.
        struct CreateShape : Command
        {
            static constexpr auto command = Shape::CreateShape;
            Physics::ShapeGeometry geom;
            Physics::SDKObjectHandle materialPtr;
            void* userData;
            Physics::SDKObjectHandle value;
            Physics::ShapeFlags flags;
        };

        // Destroys a collision shape and releases its backend resources.
        struct DestroyShape : Command
        {
            static constexpr auto command = Shape::DestroyShape;
        };

        // Updates the geometry of an existing shape (e.g. resizing a box or sphere).
        struct SetGeometry : Command
        {
            static constexpr auto command = Shape::SetGeometry;
            Physics::ShapeGeometry value;
        };

        // Retrieves the current geometry data of this shape.
        struct GetGeometry : Command
        {
            static constexpr auto command = Shape::GetGeometry;
            Physics::ShapeGeometry value;
        };

        // Maps a physics mesh triangle index back to the original source mesh triangle index,
        // accounting for any re-indexing that occurred during mesh cooking.
        struct RemapTriangleIndexToSourceIndex : Command
        {
            static constexpr auto command = Shape::RemapTriangleIndexToSourceIndex;

            uint32_t physicsMeshTriangleIndex;
            uint32_t outSourceMeshTriangleIndex;
        };

        // Retrieves the world-space axis-aligned bounding box of this shape as center and extents.
        struct GetWorldBounds : Command
        {
            static constexpr auto command = Shape::GetWorldBounds;
            Physics::Vec3 extents;
            Physics::Vec3 center;
        };

        // Computes the axis-aligned bounding box of this shape at a hypothetical pose, without changing the shape's actual pose.
        struct GetBoundsAtPose : Command
        {
            static constexpr auto command = Shape::GetBoundsAtPose;
            Physics::Pose pose;
            Physics::Vec3 extents;
            Physics::Vec3 center;
        };

        // Queries whether a collision pair identified by two ignore IDs should be ignored.
        // Returns true via 'value' if the pair is in the ignore list.
        struct ShouldIgnoreCollision : Command
        {
            static constexpr auto command = Shape::ShouldIgnoreCollision;
            uint16_t ignoreId0;
            uint16_t ignoreId1;
            bool value;
        };

        // Adds or removes a collision ignore pair between two shapes identified by their user data and ignore IDs.
        // When 'value' is true, the pair is ignored; when false, collisions are re-enabled.
        struct IgnoreCollision : Command
        {
            static constexpr auto command = Shape::IgnoreCollision;
            void* userData0;
            void* userData1;
            uint16_t ignoreId0;
            uint16_t ignoreId1;
            bool value;

        };

        // Removes all collision ignore entries associated with a collider that is being destroyed.
        struct CleanupIgnoredColliders : Command
        {
            static constexpr auto command = Shape::CleanupIgnoredColliders;
            const void* colliderPtr;
        };

        // Cooks a collision mesh (convex or triangle) from raw vertex/triangle data.
        // The result is delivered asynchronously via the 'onMeshReadyCallback'.
        //cooking
        struct CookCollisionMesh : Command
        {
            static constexpr auto command = Shape::CookCollisionMesh;

            Physics::CollisionMeshCookingDescriptor descriptor;

            void* dataContext;
            Physics::CollisionMeshReportingCallback onMeshReadyCallback;
        };

        // Cooks a collision mesh and returns the result as a serialized byte stream via the callback.
        // The stream can be stored and later used with CreateCollisionMeshFromStream.
        struct CookCollisionMeshStream : Command
        {
            static constexpr auto command = Shape::CookCollisionMeshStream;

            Physics::CollisionMeshCookingDescriptor descriptor;

            void* dataContext;
            Physics::CollisionMeshStreamReportingCallback onStreamReadyCallback;
        };

        // Creates a collision mesh from a previously cooked serialized byte stream.
        // 'convex' indicates whether the stream contains a convex or triangle mesh.
        // Returns the mesh handle via 'outCollisionMesh'.
        struct CreateCollisionMeshFromStream : Command
        {
            static constexpr auto command = Shape::CreateCollisionMeshFromStream;

            uint8_t* stream;
            size_t streamSize;
            bool convex;

            Physics::SDKObjectHandle outCollisionMesh;
        };

        // Creates a height field collision geometry from the given descriptor (sample grid, dimensions, scales).
        // Returns the height field handle via 'heightField'.
        struct CreateHeightField : Command
        {
            static constexpr auto command = Shape::CreateHeightField;

            Physics::HeightFieldDescriptor descriptor;
            void* heightField = nullptr;
        };

        // Updates a rectangular sub-region of an existing height field with new sample data.
        struct UpdateHeightFieldRegion : Command
        {
            static constexpr auto command = Shape::UpdateHeightFieldRegion;

            Physics::HeightFieldDescriptor descriptor;
            void* heightField;
            int xBase;
            int yBase;
            int width;
            int height;
        };

        // Destroys a height field and releases its backend resources.
        struct DestroyHeightField : Command
        {
            static constexpr auto command = Shape::DestroyHeightField;

            void* heightField;
        };

        // Destroys a cooked collision mesh (convex or triangle) and releases its backend resources.
        struct DestroyCollisionMesh : Command
        {
            static constexpr auto command = Shape::DestroyCollisionMesh;

            Physics::SDKObjectHandle collisionMesh;
            bool convex;
        };
    }
}
