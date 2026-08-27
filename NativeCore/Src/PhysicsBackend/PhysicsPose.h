// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once
#include "PhysicsVecTypes.h"
namespace Physics
{
    // Represents a rigid body transform as a quaternion rotation (XYZW) and a position vector.
    // 'isValid' indicates whether this pose contains meaningful data (e.g. false for an uninitialized or failed query).
    // Defaults to identity rotation at the origin.
    struct Pose
    {
    private:
        template<size_t STORAGE_SIZE>
        struct floatStorage { float data[STORAGE_SIZE]; };

        using float3Storage = floatStorage<3>;
        using float4Storage = floatStorage<4>;

    public:
        constexpr inline Pose()
            : pos{ 0,0,0 }
            , quat{ 0,0,0,1.0f }
            , isValid(true)
            , padding{ 0,0,0 } {}

        constexpr inline Pose(float px, float py, float pz, float qx, float qy, float qz, float qw)
            : pos{ px,py,pz }
            , quat{ qx,qy,qz,qw }
            , isValid(true)
            , padding{ 0,0,0 } {}

        template<typename float3Type, typename float4Type>
        inline Pose(float3Type const& pos, float4Type const& rot)
            : pos{ pos.x, pos.y, pos.z }
            , quat{ rot.x, rot.y, rot.z, rot.w }
            , isValid(true)
            , padding{ 0,0,0 } {}

        template<typename float4Type>
        inline void SetQuatFromDataWithXYZWLayout(const float4Type& q)
        {
            quat = Physics::Vec4::Load(q);
        }

        template<typename float3Type>
        inline void SetPosFromDataWithXYZLayout(const float3Type& p)
        {
            pos = Physics::Vec3::Load(p);
        }

        template<typename float4Type>
        inline void WriteQuatToDataWithXYZWLayout(float4Type& q) const
        {
            quat.Store(q);
        }

        template<typename float3Type>
        inline void WritePosToDataWithXYZLayout(float3Type& p) const
        {
            pos.Store(p);
        }

        inline bool operator==(const Pose& p) const
        {
            return pos == p.pos && quat == p.quat;
        }

        Vec4 quat;
        Vec3 pos;
        bool isValid;

    private:
        uint8_t padding[3];
    };
}
