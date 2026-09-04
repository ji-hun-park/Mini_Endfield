using Unity.Entities;
using Unity.Mathematics;

namespace Endfield.ECS
{
    /// <summary>
    /// Component defining movement data for spawned character entities.
    /// Handled in parallel by CharacterMovementSystem.
    /// </summary>
    public struct CharacterMovementComponent : IComponentData
    {
        public float3 Velocity;
        public float MoveSpeed;
        public float3 BoundsMin;
        public float3 BoundsMax;
        public float DirectionChangeTimer;
        public uint RandomSeed;
    }
}
