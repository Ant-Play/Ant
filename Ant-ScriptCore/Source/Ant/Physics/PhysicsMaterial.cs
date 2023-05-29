namespace Ant
{
    public class PhysicsMaterial
    {
        internal AssetHandle m_Handle;
        public AssetHandle Handle => m_Handle;

        internal PhysicsMaterial() { m_Handle = AssetHandle.Invalid; }
        internal PhysicsMaterial(AssetHandle handle)
        {
            m_Handle = handle;
        }

        public float StaticFriction
        {
            get => InternalCalls.PhysicsMaterial_GetStaticFriction(ref m_Handle);
            set => InternalCalls.PhysicsMaterial_SetStaticFriction(ref m_Handle, value);
        }

        public float DynamicFriction
        {
            get => InternalCalls.PhysicsMaterial_GetDynamicFriction(ref m_Handle);
            set => InternalCalls.PhysicsMaterial_SetDynamicFriction(ref m_Handle, value);
        }

        public float Restitution
        {
            get => InternalCalls.PhysicsMaterial_GetRestitution(ref m_Handle);
            set => InternalCalls.PhysicsMaterial_SetRestitution(ref m_Handle, value);
        }
    }
}