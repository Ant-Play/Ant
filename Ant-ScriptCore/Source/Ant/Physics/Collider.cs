
namespace Ant
{
    public class Collider
    {
        public ulong EntityID { get; protected set; }
        public bool IsTrigger { get; protected set; }

        private Entity m_Entity;
        private RigidBodyComponent m_RigidBodyComponent;

        public Entity Entity
        {
            get
            {
                if (m_Entity == null)
                    m_Entity = new Entity(EntityID);

                return m_Entity;
            }
        }

        public RigidBodyComponent RigidBody
        {
            get => m_RigidBodyComponent ?? (m_RigidBodyComponent = Entity.GetComponent<RigidBodyComponent>());
        }

        public override string ToString()
        {
            string type = "Collider";

            switch (this)
            {
                case BoxCollider _:
                    type = "BoxCollider";
                    break;
                case SphereCollider _:
                    type = "SphereCollider";
                    break;
                case CapsuleCollider _:
                    type = "CapsuleCollider";
                    break;
                case MeshCollider _:
                    type = "MeshCollider";
                    break;
            }

            return $"Collider({type}, {EntityID}, {IsTrigger})";
        }
    }

    public class BoxCollider : Collider
    {
        public Vector3 HalfSize { get; protected set; }
        public Vector3 Offset { get; protected set; }

        internal BoxCollider()
        {
            EntityID = 0;
            IsTrigger = false;
            HalfSize = Vector3.Zero;
            Offset = Vector3.Zero;
        }

        internal BoxCollider(ulong entityID, bool isTrigger, Vector3 halfSize, Vector3 offset)
        {
            EntityID = entityID;
            IsTrigger = isTrigger;
            HalfSize = halfSize;
            Offset = offset;
        }
    }

    public class SphereCollider : Collider
    {
        public float Radius { get; protected set; }

        internal SphereCollider()
        {
            EntityID = 0;
            IsTrigger = false;
            Radius = 0.0f;
        }

        internal SphereCollider(ulong entityID, bool isTrigger, float radius)
        {
            EntityID = entityID;
            IsTrigger = isTrigger;
            Radius = radius;
        }
    }

    public class CapsuleCollider : Collider
    {
        public float Radius { get; protected set; }
        public float Height { get; protected set; }

        internal CapsuleCollider()
        {
            EntityID = 0;
            IsTrigger = false;
            Radius = 0.0f;
            Height = 0.0f;
        }

        internal CapsuleCollider(ulong entityID, bool isTrigger, float radius, float height)
        {
            EntityID = entityID;
            IsTrigger = isTrigger;
            Radius = radius;
            Height = height;
        }
    }

    public class MeshCollider : Collider
    {
        public Mesh Mesh { get; protected set; }
        public StaticMesh StaticMesh { get; protected set; }

        public bool IsStaticMesh => InternalCalls.MeshCollider_IsStaticMesh(ref m_MeshHandle);

        private AssetHandle m_MeshHandle;

        internal MeshCollider()
        {
            EntityID = 0;
            IsTrigger = false;

            m_MeshHandle = AssetHandle.Invalid;
            Mesh = null;
            StaticMesh = null;
        }

        internal MeshCollider(ulong entityID, bool isTrigger, AssetHandle meshHandle)
        {
            EntityID = entityID;
            IsTrigger = isTrigger;

            if (meshHandle.IsValid())
            {
                if (InternalCalls.MeshCollider_IsStaticMesh(ref meshHandle))
                    StaticMesh = new StaticMesh(meshHandle);
                else
                    Mesh = new Mesh(meshHandle);

                m_MeshHandle = meshHandle;
            }
            else
            {
                m_MeshHandle = AssetHandle.Invalid;
            }
        }
    }
}