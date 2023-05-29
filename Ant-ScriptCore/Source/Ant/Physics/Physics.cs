using System;
using System.Runtime.InteropServices;

namespace Ant
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastHit
    {
        public ulong EntityID { get; internal set; }
        public Vector3 Position { get; internal set; }
        public Vector3 Normal { get; internal set; }
        public float Distance { get; internal set; }
        public Collider HitCollider { get; internal set; }

        public Entity Entity => Scene.FindEntityByID(EntityID);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastData
    {
        public Vector3 Origin;
        public Vector3 Direction;
        public float MaxDistance;
        public Type[] RequiredComponents;
        public ulong[] ExcludedEntities;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SphereCastData
    {
        public Vector3 Origin;
        public Vector3 Direction;
        public float Radius;
        public float MaxDistance;
        public Type[] RequiredComponents;
        public ulong[] ExcludedEntities;
    }

    public class RaycastHit2D
    {
        public Entity Entity { get; internal set; }
        public Vector2 Position { get; internal set; }
        public Vector2 Normal { get; internal set; }
        public float Distance { get; internal set; }

        internal RaycastHit2D()
        {
            Entity = null;
            Position = Vector2.Zero;
            Normal = Vector2.Zero;
            Distance = 0.0f;
        }

        internal RaycastHit2D(Entity entity, Vector2 position, Vector2 normal, float distance)
        {
            Entity = entity;
            Position = position;
            Normal = normal;
            Distance = distance;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastData2D
    {
        public Vector2 Origin;
        public Vector2 Direction;
        public float MaxDistance;
        public Type[] RequiredComponents;
    }

    public enum ActorLockFlag : uint
    {
        TranslationX = 1 << 0,
        TranslationY = 1 << 1,
        TranslationZ = 1 << 2,
        TranslationXYZ = TranslationX | TranslationY | TranslationZ,

        RotationX = 1 << 3,
        RotationY = 1 << 4,
        RotationZ = 1 << 5,
        RotationXYZ = RotationX | RotationY | RotationZ
    }

    public enum EFalloffMode { Constant, Linear }

    public static class Physics
    {
        public static Vector3 Gravity
        {
            get
            {
                InternalCalls.Physics_GetGravity(out Vector3 gravity);
                return gravity;
            }

            set => InternalCalls.Physics_SetGravity(ref value);
        }

        /// <summary>
        /// Adds a radial impulse to the scene. Any entity within the radius of the origin will be pushed/pulled according to the strength.
        /// You'd use this method for something like an explosion.
        /// </summary>
        /// <param name="origin">The origin of the impulse in world space</param>
        /// <param name="radius">The radius of the area affected by the impulse (1 unit = 1 meter radius)</param>
        /// <param name="strength">The strength of the impulse</param>
        /// <param name="falloff">The falloff method used when calculating force over distance</param>
        /// <param name="velocityChange">Setting this value to <b>true</b> will make this impulse ignore an actors mass</param>
        public static void AddRadialImpulse(Vector3 origin, float radius, float strength, EFalloffMode falloff = EFalloffMode.Constant, bool velocityChange = false)
            => InternalCalls.Physics_AddRadialImpulse(ref origin, radius, strength, falloff, velocityChange);

        public static bool Raycast(RaycastData raycastData, out RaycastHit hit) => InternalCalls.Physics_Raycast(ref raycastData, out hit);
        public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit, params Type[] componentFilters)
        {
            s_RaycastData.Origin = origin;
            s_RaycastData.Direction = direction;
            s_RaycastData.MaxDistance = maxDistance;
            s_RaycastData.RequiredComponents = componentFilters;
            return InternalCalls.Physics_Raycast(ref s_RaycastData, out hit);
        }

        public static bool SphereCast(SphereCastData spherecastData, out RaycastHit hit) => InternalCalls.Physics_SphereCast(ref spherecastData, out hit);
        public static bool SphereCast(Vector3 origin, Vector3 direction, float radius, float maxDistance, out RaycastHit hit, params Type[] componentFilters)
        {
            s_SphereCastData.Origin = origin;
            s_SphereCastData.Direction = direction;
            s_SphereCastData.Radius = radius;
            s_SphereCastData.MaxDistance = maxDistance;
            s_SphereCastData.RequiredComponents = componentFilters;
            return InternalCalls.Physics_SphereCast(ref s_SphereCastData, out hit);
        }

        public static RaycastHit2D[] Raycast2D(RaycastData2D raycastData) => InternalCalls.Physics_Raycast2D(ref raycastData);
        public static RaycastHit2D[] Raycast2D(Vector2 origin, Vector2 direction, float maxDistance, params Type[] componentFilters)
        {
            s_RaycastData2D.Origin = origin;
            s_RaycastData2D.Direction = direction;
            s_RaycastData2D.MaxDistance = maxDistance;
            s_RaycastData2D.RequiredComponents = componentFilters;
            return InternalCalls.Physics_Raycast2D(ref s_RaycastData2D);
        }

        public static Collider[] OverlapBox(Vector3 origin, Vector3 halfSize) => InternalCalls.Physics_OverlapBox(ref origin, ref halfSize);
        public static Collider[] OverlapCapsule(Vector3 origin, float radius, float halfHeight) => InternalCalls.Physics_OverlapCapsule(ref origin, radius, halfHeight);
        public static Collider[] OverlapSphere(Vector3 origin, float radius) => InternalCalls.Physics_OverlapSphere(ref origin, radius);

        public static int OverlapBoxNonAlloc(Vector3 origin, Vector3 halfSize, Collider[] colliders) => InternalCalls.Physics_OverlapBoxNonAlloc(ref origin, ref halfSize, colliders);
        public static int OverlapCapsuleNonAlloc(Vector3 origin, float radius, float halfHeight, Collider[] colliders) => InternalCalls.Physics_OverlapCapsuleNonAlloc(ref origin, radius, halfHeight, colliders);
        public static int OverlapSphereNonAlloc(Vector3 origin, float radius, Collider[] colliders) => InternalCalls.Physics_OverlapSphereNonAlloc(ref origin, radius, colliders);

        private static RaycastData s_RaycastData;
        private static SphereCastData s_SphereCastData;
        private static RaycastData2D s_RaycastData2D;
    }
}