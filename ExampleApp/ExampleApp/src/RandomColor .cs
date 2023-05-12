﻿using System;




using Ant;

namespace Example
{
    public class RandomColor :Entity
    {

        void OnCreate()
        {
            Random random = new Random();

            MeshComponent meshComponent = GetComponent<MeshComponent>();
            MaterialInstance material = meshComponent.Mesh.GetMaterial(0);
            float r = (float)random.NextDouble();
            float g = (float)random.NextDouble();
            float b = (float)random.NextDouble();
            material.Set("u_AlbedoColor", new Vector3(r, g, b));
        }
    }
}