using System;
using Ant;

public class RuntimeMesh : Entity
{

    public Mesh MyMesh;

    void OnCreate()
    {
        GetComponent<MeshComponent>().Mesh = MyMesh;
    }

}