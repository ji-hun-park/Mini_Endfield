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
        public float BaseY;
        public float PhaseOffset;
        public float Amplitude;
        public float AnimationSpeed;
    }
}
