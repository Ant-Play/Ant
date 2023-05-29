namespace Ant
{
    public class StaticMesh
    {
        internal AssetHandle m_Handle;
        public AssetHandle Handle => m_Handle;

        internal StaticMesh() { m_Handle = AssetHandle.Invalid; }
        internal StaticMesh(AssetHandle handle) { m_Handle = handle; }

        public Material BaseMaterial => GetMaterial(0);

        public Material GetMaterial(int index)
        {
            if (!InternalCalls.StaticMesh_GetMaterialByIndex(ref m_Handle, index, out AssetHandle materialHandle))
                return null;

            return new Material(m_Handle, materialHandle, null);
        }

        public int GetMaterialCount() => InternalCalls.StaticMesh_GetMaterialCount(ref m_Handle);
    }
}