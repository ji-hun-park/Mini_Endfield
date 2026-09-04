using Unity.Burst;
using Unity.Entities;
using Unity.Mathematics;
using Unity.Transforms;

namespace Endfield.ECS.Systems
{
    /// <summary>
    /// Burst-compiled parallel system for moving 10,000+ characters at 60+ FPS.
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
            float dt = SystemAPI.Time.DeltaTime;

            var job = new MovementJob
            {
                DeltaTime = dt
            };

            state.Dependency = job.ScheduleParallel(state.Dependency);
        }

        [BurstCompile]
        private partial struct MovementJob : IJobEntity
        {
            public float DeltaTime;

            public void Execute(ref LocalTransform transform, ref CharacterMovementComponent movement)
            {
                // 1. Move position
                float3 pos = transform.Position + movement.Velocity * DeltaTime;

                // 2. Boundary collision and reflection
                bool bounced = false;
                if (pos.x < movement.BoundsMin.x)
                {
                    pos.x = movement.BoundsMin.x;
                    movement.Velocity.x = math.abs(movement.Velocity.x);
                    bounced = true;
                }
                else if (pos.x > movement.BoundsMax.x)
                {
                    pos.x = movement.BoundsMax.x;
                    movement.Velocity.x = -math.abs(movement.Velocity.x);
                    bounced = true;
                }

                if (pos.z < movement.BoundsMin.z)
                {
                    pos.z = movement.BoundsMin.z;
                    movement.Velocity.z = math.abs(movement.Velocity.z);
                    bounced = true;
                }
                else if (pos.z > movement.BoundsMax.z)
                {
                    pos.z = movement.BoundsMax.z;
                    movement.Velocity.z = -math.abs(movement.Velocity.z);
                    bounced = true;
                }

                // Keep Y position stable
                pos.y = movement.BoundsMin.y;

                // 3. Periodic wandering direction change
                movement.DirectionChangeTimer -= DeltaTime;
                if (movement.DirectionChangeTimer <= 0f && !bounced)
                {
                    // LCG pseudo-random for zero-allocation performance
                    movement.RandomSeed = (movement.RandomSeed * 1664525u + 1013904223u);
                    float angle = ((movement.RandomSeed >> 16) / 65535.0f) * (math.PI * 2f);
                    movement.Velocity = new float3(math.cos(angle) * movement.MoveSpeed, 0f, math.sin(angle) * movement.MoveSpeed);
                    movement.DirectionChangeTimer = 2.0f + ((movement.RandomSeed & 0x7) * 0.4f);
                }

                // 4. Smoothly rotate towards velocity direction
                float speedSq = math.lengthsq(movement.Velocity);
                if (speedSq > 0.001f)
                {
                    float3 dir = math.normalize(movement.Velocity);
                    quaternion targetRot = quaternion.LookRotationSafe(dir, math.up());
                    transform.Rotation = math.slerp(transform.Rotation, targetRot, math.clamp(DeltaTime * 10f, 0f, 1f));
                }

                transform.Position = pos;
            }
        }
    }
}
