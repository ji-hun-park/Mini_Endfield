// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once
#include <cstdint>
#include "PhysicsCommands.h"
#include "PhysicsQueryTypes.h"
#include "PhysicsShapeTypes.h"
#include "PhysicsPose.h"

namespace PhysicsCommands
{
    namespace QueryData
    {
        // Computes the penetration depth and separation direction between two overlapping shapes.
        // Returns 'areOverlapping' as true if the shapes intersect, with 'outDirection' and 'outDistance'
        // describing the minimum translation to separate them.
        struct ComputeShapePenetration : Command
        {
            static constexpr auto command = Query::ComputeShapePenetration;

            void* shapeA;
            void* shapeB;
            Physics::Pose poseA;
            Physics::Pose poseB;

            Physics::Vec3 outDirection;
            float outDistance;
            bool areOverlapping;
        };

        // Finds the closest point on a shape's surface to a given world-space point.
        // Returns the closest point via 'outPoint' and the signed distance via 'outDistance'.
        struct ClosestPointOnShape : Command
        {
            static constexpr auto command = Query::ClosestPointOnShape;

            Physics::Pose pose;
            Physics::Vec3 point;

            Physics::Vec3 outPoint;
            float outDistance;
        };

        // Performs a scene-wide raycast from 'origin' along 'direction' up to 'maxDistance'.
        // Filters results using 'filter'. Returns the closest hit via 'hit' and the number of hits via 'hitCount'.
        struct RayCast : Command
        {
            static constexpr auto command = Query::RayCast;

            Physics::QueryHit hit;
            Physics::QueryFilterData filter;
            Physics::Vec3 origin;
            Physics::Vec3 direction;
            uint32_t hitCount;
            float maxDistance;
        };

        // Performs a raycast against a single specific shape (identified via Context).
        // Uses 'filterOptions' to control hit flags. Returns the hit result and count.
        struct RayCastAgainstShape : Command
        {
            static constexpr auto command = Query::RayCastAgainstShape;

            Physics::Vec3 origin;
            Physics::Vec3 direction;
            Physics::QueryFilterFlags filterOptions;
            Physics::QueryHit hit;
            uint32_t hitCount;
            float maxDistance;
        };

        // Performs a scene-wide raycast and delivers all hits via a collector callback.
        // The collector is invoked for each hit, allowing custom accumulation or filtering logic.
        struct RaycastWithCollector : Command
        {
            static constexpr auto command = Query::RayCastWithCollector;

            Physics::QueryHitResultCollectorCallback collector;
            void* dataContext;
            Physics::QueryFilterData filter;
            Physics::Vec3 origin;
            Physics::Vec3 direction;
            float maxDistance;
        };

        // Tests whether any shape in the scene overlaps with the given geometry at the specified pose.
        // Returns the first overlapping shape's user data via 'shapeUserData', or null if no overlap.
        struct Overlap : Command
        {
            static constexpr auto command = Query::Overlap;

            Physics::ShapeGeometry geometry;
            Physics::Pose pose;
            Physics::QueryFilterData filter;

            void* shapeUserData;
        };

        // Tests for all shapes overlapping with the given geometry and delivers results via a collector callback.
        // The collector is invoked for each overlapping shape found.
        struct OverlapWithCollector : Command
        {
            static constexpr auto command = Query::OverlapWithCollector;

            Physics::ShapeGeometry geometry;
            Physics::Pose pose;
            Physics::QueryFilterData filter;
            void* dataContext;
            Physics::QueryShapesResultCollectorCallback collector;
        };

        // Tests for overlaps using the broadphase acceleration structure and delivers results via a broadphase-specific collector.
        // More efficient than narrow-phase overlap when only approximate (AABB-level) results are needed.
        struct OverlapWithBroadphaseCollector : Command
        {
            static constexpr auto command = Query::OverlapWithBroadphaseCollector;

            Physics::ShapeGeometry geometry;
            Physics::Pose pose;
            Physics::QueryFilterData filter;
            void* dataContext;
            Physics::BroadPhaseQueryShapeCollectorCallback collector;

        };

        // Sweeps a geometry shape along a direction to find the closest hit in the scene.
        // Returns the hit result and count. Used for Physics.SphereCast, CapsuleCast, BoxCast, etc.
        struct ShapeCast : Command
        {
            static constexpr auto command = Query::ShapeCast;

            Physics::ShapeGeometry geometry;
            Physics::Pose pose;
            Physics::QueryFilterData filter;
            Physics::Vec3 direction;
            float maxDistance;

            Physics::QueryHit hit;
            uint32_t hitCount;
        };

        // Sweeps a geometry shape against a single specific target shape (point-to-point sweep test).
        // When 'distanceAsDepth' is true, the returned distance represents penetration depth instead of sweep distance.
        struct ShapeCastAgainstShape : Command
        {
            static constexpr auto command = Query::ShapeCastAgainstShape;

            Physics::ShapeGeometry targetGeometry;
            Physics::Pose targetPose;

            Physics::QueryFilterData filter;
            Physics::Vec3 direction;
            float maxDistance;

            Physics::QueryHit hit;
            uint32_t hitCount;
            bool distanceAsDepth;
        };

        // Sweeps a geometry shape and delivers all hits via a collector callback.
        // When 'keepInitialOverlaps' is true, shapes already overlapping at the start position are also reported.
        struct ShapeCastWithCollector : Command
        {
            static constexpr auto command = Query::ShapeCastWithCollector;

            Physics::ShapeGeometry geometry;
            Physics::Pose pose;
            //added only for supporting the current behaviour of Physics.XCastAll/NonAlloc(...) where initial overlap hits are also reported
            //this behavior should be unified in later versions. As the other ShapeCast command usages discard the initial overlaps
            bool keepInitialOverlaps;
            Physics::QueryFilterData filter;
            Physics::Vec3 direction;
            float maxDistance;

            void* dataContext;
            Physics::QueryHitResultCollectorCallback collector;
        };

        // Sweeps all shapes attached to a body along a direction to find the closest hit in the scene.
        // The body's current shape configuration is used as the sweep geometry.
        struct BodyCast : Command
        {
            static constexpr auto command = Query::BodyCast;

            Physics::Vec3 direction;
            float maxDistance;
            Physics::QueryFilterData filter;
            Physics::SDKObjectHandle body;

            Physics::QueryHit hit;
            uint32_t hitCount;
        };

        // Sweeps all shapes attached to a body and delivers all hits via a collector callback.
        // Similar to BodyCast but supports collecting multiple results.
        struct BodyCastWithCollector : Command
        {
            static constexpr auto command = Query::BodyCastWithCollector;

            Physics::Vec3 direction;
            float maxDistance;
            Physics::QueryFilterData filter;
            Physics::SDKObjectHandle body;

            void* dataContext;
            Physics::QueryHitResultCollectorCallback collector;
        };
    }
}
