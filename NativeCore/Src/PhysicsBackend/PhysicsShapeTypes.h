// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once
#include <cstdint>
#include <type_traits>
#include "PhysicsVecTypes.h"
#include "PhysicsCommonTypes.h"

namespace Physics
{
    // Bitmask flags controlling how a shape participates in simulation and queries.
    enum class ShapeFlags
    {
        None = 0,
        SceneQuery = 1 << 0, // Shape is visible to scene queries (raycasts, overlaps, sweeps)
        Trigger = 1 << 1     // Shape is a trigger volume (generates enter/exit events, no physical response)
    };

    inline constexpr ShapeFlags operator | (ShapeFlags lhs, ShapeFlags rhs)
    {
        using type_t = std::underlying_type_t<ShapeFlags>;
        return static_cast<ShapeFlags>(static_cast<type_t>(lhs) | static_cast<type_t>(rhs));
    }

    inline constexpr ShapeFlags& operator |= (ShapeFlags& lhs, ShapeFlags rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr ShapeFlags operator & (ShapeFlags lhs, ShapeFlags rhs)
    {
        using type_t = std::underlying_type_t<ShapeFlags>;
        return static_cast<ShapeFlags>(static_cast<type_t>(lhs) & static_cast<type_t>(rhs));
    }

    inline constexpr ShapeFlags& operator &= (ShapeFlags& lhs, ShapeFlags rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    inline constexpr ShapeFlags operator ~ (ShapeFlags rhs)
    {
        using type_t = std::underlying_type_t<ShapeFlags>;
        return static_cast<ShapeFlags>(~static_cast<type_t>(rhs));
    }

    // Enumerates the supported collision geometry primitives.
    // Values are non-contiguous to match the underlying SDK's geometry type IDs.
    enum class GeometryType : uint16_t
    {
        Invalid = 0,
        Sphere = 1,
        Capsule = 3,
        Box = 4,
        ConvexMesh = 5,
        TriangleMesh = 6,
        Terrain = 7
    };

    // Box collision geometry defined by half-extents along each local axis.
    struct BoxGeometry
    {
        inline const bool operator==(const BoxGeometry& other) const
        {
            return halfExtents == other.halfExtents;
        }

        inline const bool operator!=(const BoxGeometry& other) const
        {
            return halfExtents != other.halfExtents;
        }

        Vec3 halfExtents;
    };

    // Sphere collision geometry defined by its radius.
    struct SphereGeometry
    {
        inline const bool operator==(const SphereGeometry& other) const
        {
            return radius == other.radius;
        }

        inline const bool operator!=(const SphereGeometry& other) const
        {
            return radius != other.radius;
        }

        float radius;
    };

    // Capsule collision geometry defined by its radius and total height (including hemispherical caps).
    struct CapsuleGeometry
    {
        inline const bool operator==(const CapsuleGeometry& other) const
        {
            return radius == other.radius && height == other.height;
        }

        inline const bool operator!=(const CapsuleGeometry& other) const
        {
            return radius != other.radius || height != other.height;
        }

        float radius;
        float height;
    };

    // Mesh-based collision geometry (convex hull or triangle mesh) referencing a cooked mesh with a per-axis scale.
    struct MeshGeometry
    {
        inline const bool operator==(const MeshGeometry& other) const
        {
            return mesh == other.mesh && scale == other.scale;
        }

        inline const bool operator!=(const MeshGeometry& other) const
        {
            return mesh != other.mesh || scale != other.scale;
        }

        void* mesh;
        Vec3 scale;
    };

    // Height field collision geometry referencing a height field data object with a per-axis scale.
    struct HeightFieldGeometry
    {
        inline const bool operator==(const HeightFieldGeometry& other) const
        {
            return heightField == other.heightField && scale == other.scale;
        }


        inline const bool operator!=(const HeightFieldGeometry& other) const
        {
            return heightField != other.heightField || scale != other.scale;
        }

        void* heightField;
        Vec3 scale;
    };

    // Tagged union holding any supported geometry type. The 'type' field determines which
    // union member is active. Maximum storage is 24 bytes (sized to fit MeshGeometry/HeightFieldGeometry).
    struct ShapeGeometry
    {
        inline const bool operator==(const ShapeGeometry& other) const
        {
            if (type != other.type)
                return false;

            switch(type)
            {
            case GeometryType::Box:
                return boxGeometry == other.boxGeometry;
            case GeometryType::Capsule:
                return capsuleGeometry == other.capsuleGeometry;
            case GeometryType::ConvexMesh:
            case GeometryType::TriangleMesh:
                return meshGeometry == other.meshGeometry;
            case GeometryType::Sphere:
                return sphereGeometry == other.sphereGeometry;
            case GeometryType::Terrain:
                return heightFieldGeometry == other.heightFieldGeometry;
            default:
                return false;
            }
        }

        inline const bool operator!=(const ShapeGeometry& other) const
        {
            return !operator==(other);
        }

        union
        {
            BoxGeometry boxGeometry;
            SphereGeometry sphereGeometry;
            CapsuleGeometry capsuleGeometry;
            MeshGeometry meshGeometry;
            HeightFieldGeometry heightFieldGeometry;
        };

        GeometryType type;
    };

    // Descriptor for creating or updating a height field. Provides the sample grid (int16 heights),
    // optional hole mask, and the resolution (samples per side for a square grid).
    struct HeightFieldDescriptor
    {
        const int16_t* heights = nullptr;
        const uint8_t* holes = nullptr;
        int resolution = 0;
        int holesCount = 0;
        int heightsCount = 0;
    };

    // Bitmask options controlling the collision mesh cooking process.
    enum class CollisionMeshProcessingOptions : uint16_t
    {
        None = 0,
        UseLegacyCookingSystem = 1 << 0,        // Use the legacy mesh cooking pipeline
        OptimizeForRuntimePerformance = 1 << 1, // Optimize the cooked mesh for faster runtime queries
        CleanupInputMesh = 1 << 2,              // Remove degenerate triangles and fix winding order
        WeldVertices = 1 << 3,                  // Merge vertices that are within a tolerance distance
    };

    inline constexpr CollisionMeshProcessingOptions operator | (CollisionMeshProcessingOptions lhs, CollisionMeshProcessingOptions rhs)
    {
        using type_t = std::underlying_type_t<CollisionMeshProcessingOptions>;
        return static_cast<CollisionMeshProcessingOptions>(static_cast<type_t>(lhs) | static_cast<type_t>(rhs));
    }

    inline constexpr CollisionMeshProcessingOptions& operator |= (CollisionMeshProcessingOptions& lhs, CollisionMeshProcessingOptions rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr CollisionMeshProcessingOptions operator & (CollisionMeshProcessingOptions lhs, CollisionMeshProcessingOptions rhs)
    {
        using type_t = std::underlying_type_t<CollisionMeshProcessingOptions>;
        return static_cast<CollisionMeshProcessingOptions>(static_cast<type_t>(lhs) & static_cast<type_t>(rhs));
    }

    // Descriptor for cooking a collision mesh from raw vertex and index data.
    // Specifies the vertex/index streams, element counts and sizes, cooking options,
    // and whether the mesh is convex or has flipped normals.
    struct CollisionMeshCookingDescriptor
    {
        void* vertexStream = NULL;
        void* indexStream = NULL;
        uint32_t vertexElementCount = 0;
        uint32_t indexElementCount = 0;

        Physics::CollisionMeshProcessingOptions options = CollisionMeshProcessingOptions::None;
        uint16_t vertexElementSize = 0;
        uint16_t indexElementSize = 0;
        bool convex = false;
        bool flippedNormals = false;
    };

    // Result code returned by mesh cooking operations.
    enum class CookingError
    {
        Success = 0,
        Failed = 1 << 0,  // Cooking failed (invalid input or internal error)
        Warning = 1 << 1, // Cooking succeeded with warnings (e.g. degenerate triangles removed)
    };

    inline constexpr CookingError operator | (CookingError lhs, CookingError rhs)
    {
        using type_t = std::underlying_type_t<CookingError>;
        return static_cast<CookingError>(static_cast<type_t>(lhs) | static_cast<type_t>(rhs));
    }

    inline constexpr CookingError operator & (CookingError lhs, CookingError rhs)
    {
        using type_t = std::underlying_type_t<CookingError>;
        return static_cast<CookingError>(static_cast<type_t>(lhs) & static_cast<type_t>(rhs));
    }

    // Callback invoked when mesh cooking to a stream completes. Receives the result code, optional message,
    // caller context, and the serialized stream data with its size in bytes.
    using CollisionMeshStreamReportingCallback = void (*)(CookingError result, const char* msg, void* context, void* stream, const size_t streamSizeinBytes);

    // Callback invoked when mesh cooking to an SDK mesh object completes. Receives the result code,
    // optional message, caller context, and the cooked mesh handle.
    using CollisionMeshReportingCallback = void (*)(CookingError result, const char* msg, void* context, Physics::SDKObjectHandle mesh);

    // Raw mesh data extracted from a cooked collision mesh via ExtractCollisionMeshData.
    // Provides read-only access to the vertex positions, vertex count/stride, and index data.
    struct SDKCollisionMeshData
    {
        const float* vertices;
        size_t vertexCount;
        size_t vertexStride;

        const void* indices;
        size_t indexCount;
        size_t indexStride;
    };

    // Callback invoked with the extracted collision mesh data.
    using CollisionMeshDataExtractionCallback = void (*)(void* context, SDKCollisionMeshData meshData);
}
