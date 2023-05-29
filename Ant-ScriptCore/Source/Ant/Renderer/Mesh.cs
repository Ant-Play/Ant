namespace Ant
{
    public class Mesh : MeshBase
    {
        internal AssetHandle m_Handle;
        public AssetHandle Handle => m_Handle;

        internal Mesh() { m_Handle = AssetHandle.Invalid; }
        internal Mesh(AssetHandle handle) { m_Handle = handle; }

        public Material BaseMaterial => GetMaterial(0);

        public override Material GetMaterial(int index)
        {
            if (!InternalCalls.Mesh_GetMaterialByIndex(ref m_Handle, index, out AssetHandle materialHandle))
                return null;

            return materialHandle.IsValid() ? new Material(m_Handle, materialHandle, null) : null;
        }

        public override int GetMaterialCount() => InternalCalls.Mesh_GetMaterialCount(ref m_Handle);
    }
}
