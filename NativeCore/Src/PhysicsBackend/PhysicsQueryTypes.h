// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once
#include <cstdint>
#include "PhysicsCommonTypes.h"
#include "PhysicsShapeTypes.h"

namespace Physics
{
    struct QueryHit;

    // Callback invoked for each batch of raycast or shape cast hits. Returns false to stop collecting further hits.
    using QueryHitResultCollectorCallback = bool (*)(void* context, QueryHit* hits, const size_t count);

    // Callback invoked for each batch of overlap results (user data pointers). Returns false to stop collecting.
    using QueryShapesResultCollectorCallback = bool (*)(void* context, void** shapeUserDatas, const size_t count);

    // Callback invoked per-shape during broadphase overlap queries. Receives the shape/body handles,
    // user data pointer, and geometry type. Returns false to stop the query.
    using BroadPhaseQueryShapeCollectorCallback = bool (*)(void* context, SDKObjectHandle shape, SDKObjectHandle body, void* shapeUserDataPtr, GeometryType shapeType);

    // Result of a raycast, shape cast, or sweep query. Contains the hit position, surface normal,
    // triangle face ID, hit distance, barycentric UV coordinates, and the engine-side entity ID of the hit object.
    struct QueryHit
    {
        Vec3 point;
        Vec3 normal;
        uint32_t faceID;
        float distance;
        float uv[2];
        EngineObjectId userData;

        static constexpr QueryHit Invalid() { return { {0,0,0}, {0,0,0}, 0, 0.0f,{0.0f, 0.0f}, kEngineObjectId_None }; }
    };

    // Bitmask flags controlling which objects and behaviors are included in a scene query.
    enum class QueryFilterFlags
    {
        None = 0,
        UseDynamicBodies = 1 << 0,             // Include dynamic (non-kinematic) bodies
        UseStaticBodies = 1 << 1,              // Include static bodies
        UseTriggerShapes = 1 << 2,             // Include trigger shapes in results
        AlwaysReportTerrainMeshHoleHits = 1 << 3, // Report hits on terrain mesh holes
        AllowTriangleMeshBackfaceHits = 1 << 4,   // Report hits on backfaces of triangle meshes
        AllowMultipleTriangleMeshHits = 1 << 5,   // Report multiple hits per triangle mesh
        AllowEarlyOutOnFirstHit = 1 << 6          // Stop after the first hit (performance optimization)
    };

    inline constexpr QueryFilterFlags operator | (QueryFilterFlags lhs, QueryFilterFlags rhs)
    {
        using type_t = std::underlying_type_t<QueryFilterFlags>;
        return static_cast<QueryFilterFlags>(static_cast<type_t>(lhs) | static_cast<type_t>(rhs));
    }

    inline constexpr QueryFilterFlags& operator |= (QueryFilterFlags& lhs, QueryFilterFlags rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr QueryFilterFlags operator ~(QueryFilterFlags v)
    {
        using type_t = std::underlying_type_t<QueryFilterFlags>;

        return static_cast<QueryFilterFlags>(~static_cast<type_t>(v));
    }

    inline constexpr QueryFilterFlags operator &(QueryFilterFlags lhs, QueryFilterFlags rhs)
    {
        using type_t = std::underlying_type_t<QueryFilterFlags>;
        return static_cast<QueryFilterFlags>(static_cast<type_t>(lhs) & static_cast<type_t>(rhs));
    }

    inline constexpr QueryFilterFlags operator &=(QueryFilterFlags& lhs, QueryFilterFlags rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    // Combined filter parameters for a scene query: filter flags controlling which objects to test,
    // and a layer mask selecting which collision layers to include.
    struct QueryFilterData
    {
        QueryFilterFlags options;
        int mask;
    };
}
