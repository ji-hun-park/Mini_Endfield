using Unity.Burst;
using Unity.Entities;
using Unity.Mathematics;
using Unity.Transforms;

namespace Endfield.ECS.Systems
{
    /// <summary>
    /// Burst-compiled parallel system for moving characters along Y-axis in sine wave oscillation.
    /// Runs in SimulationSystemGroup before TransformSystemGroup so parent/child transforms propagate properly.
    /// </summary>
    [UpdateInGroup(typeof(SimulationSystemGroup))]
    [UpdateBefore(typeof(TransformSystemGroup))]
    public partial struct CharacterMovementSystem : ISystem
    {
        [BurstCompile]
        public void OnCreate(ref SystemState state)
        {
            state.RequireForUpdate<CharacterMovementComponent>();
        }

        [BurstCompile]
        public void OnUpdate(ref SystemState state)
        {
            float elapsedTime = (float)SystemAPI.Time.ElapsedTime;

            var job = new SineMovementJob
            {
                ElapsedTime = elapsedTime
            };

            state.Dependency = job.ScheduleParallel(state.Dependency);
        }

        [BurstCompile]
        private partial struct SineMovementJob : IJobEntity
        {
            public float ElapsedTime;

            public void Execute(ref LocalTransform transform, in CharacterMovementComponent movement)
            {
                // y = BaseY + A * sin(time * speed + phase)
                float currentY = movement.BaseY + movement.Amplitude * math.sin(ElapsedTime * movement.AnimationSpeed + movement.PhaseOffset);
                transform.Position.y = currentY;
            }
        }
    }
}
