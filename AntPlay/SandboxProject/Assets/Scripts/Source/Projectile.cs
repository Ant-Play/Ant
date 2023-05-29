﻿using System;
using Ant;

namespace PrefabSandbox
{
    public class Projectile : Entity
    {

        public float LifeTime = 2.0f;

        public Prefab ExplosiveParticle;
        private bool m_Destroy = false;

        protected override void OnCreate()
        {
            LifeTime = 2.0f;
            CollisionBeginEvent += OnCollisionBegin;
        }

        protected override void OnUpdate(float ts)
        {
            LifeTime -= ts;
            if (LifeTime <= 0.0f)
                Destroy();

            if (m_Destroy)
            {
                for (int i = 0; i < 10; i++)
                {
                    Entity particle = Scene.InstantiatePrefab(ExplosiveParticle, Transform.Translation);
                    Vector3 dir = new Vector3(Ant.Random.Float() * 2.0f - 1.0f, Ant.Random.Float() * 2.0f - 1.0f + 1.0f, Ant.Random.Float() * 2.0f - 1.0f);
                    Vector3 force = dir * Ant.Random.Float() * 5.0f;
                    particle.GetComponent<RigidBodyComponent>().AddForce(force, ForceMode.Impulse);
                }
                Destroy();
            }
        }

        public void OnCollisionBegin(Entity other)
        {
            if (other.GetComponent<TagComponent>().Tag == "Player")
                return;

            m_Destroy = true;
        }


    }
}
