using System;
using System.Runtime.CompilerServices;

namespace Ant
{
    public class Entity
    {
        public ulong ID { get; private set; }

        ~Entity()
        {
        }

        public T CreateComponent<T>() where T : Component, new()
        {
            CreateComponent_Native(ID, typeof(T));
            var component = new T
            {
                Entity = this
            };
            return component;
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(ID, typeof(T));
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>()) return null;
            var component = new T
            {
                Entity = this
            };
            return component;
        }

        public Matrix4 GetTransform()
        {
            GetTransform_Native(ID, out var matrix);
            return matrix;
        }

        public void SetTransform(Matrix4 transform)
        {
            SetTransform_Native(ID, ref transform);
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateComponent_Native(ulong entityID, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(ulong entityID, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetTransform_Native(ulong entityID, out Matrix4 matrix);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTransform_Native(ulong entityID, ref Matrix4 matrix);
    }
}