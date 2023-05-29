using Ant;

//
// Adapted from https://github.com/SebLague/Boids
//

namespace Sandbox
{
	public class Boid : Entity
	{
		BoidSettings settings;

		// State
		[HideFromEditor]
		public Vector3 position;
		[HideFromEditor]
		public Vector3 forward;
		Vector3 velocity;

		// To update:
		Vector3 acceleration;
		[HideFromEditor]
		public Vector3 avgFlockHeading;
		[HideFromEditor]
		public Vector3 avgAvoidanceHeading;
		[HideFromEditor]
		public Vector3 centreOfFlockmates;
		[HideFromEditor]
		public int numPerceivedFlockmates;

		// Cached
		Material material;
		Transform cachedTransform;

		public Entity Target;

		protected override void OnCreate()
		{
			cachedTransform = Transform.WorldTransform;
		}

		public void Initialize(BoidSettings settings, Entity target)
		{
			Target = target;
			this.settings = settings;

			position = Translation;
			forward = cachedTransform.Forward;

			float startSpeed = (settings.minSpeed + settings.maxSpeed) / 2;
			velocity = Transform.WorldTransform.Forward * startSpeed;
		}

		public void UpdateBoid(float ts)
		{
			Vector3 acceleration = Vector3.Zero;

			if (Target != null)
			{
				Vector3 offsetToTarget = (Target.Translation - position);
				acceleration = SteerTowards(offsetToTarget) * settings.targetWeight;
			}

			if (numPerceivedFlockmates != 0)
			{
				centreOfFlockmates /= numPerceivedFlockmates;

				Vector3 offsetToFlockmatesCentre = (centreOfFlockmates - position);

				var alignmentForce = SteerTowards(avgFlockHeading) * settings.alignWeight;
				var cohesionForce = SteerTowards(offsetToFlockmatesCentre) * settings.cohesionWeight;
				var seperationForce = SteerTowards(avgAvoidanceHeading) * settings.seperateWeight;

				acceleration += alignmentForce;
				acceleration += cohesionForce;
				acceleration += seperationForce;
			}

			if (IsHeadingForCollision())
			{
				Vector3 collisionAvoidDir = ObstacleRays();
				Vector3 collisionAvoidForce = SteerTowards(collisionAvoidDir) * settings.avoidCollisionWeight;
				acceleration += collisionAvoidForce;
			}

			velocity += acceleration * ts;
			float speed = velocity.Length();
			Vector3 dir = velocity / speed;
			speed = Mathf.Clamp(speed, settings.minSpeed, settings.maxSpeed);
			velocity = dir * speed;

			cachedTransform.Position += velocity * ts;

			// TODO(Yan): we really just need a way to set Forward like in Unity
			Quaternion q = Quaternion.QuaternionLookRotation(dir, Vector3.Up);
			SetRotation(q); // cachedTransform.Forward = dir;

			position = cachedTransform.Position;
			Translation = position;
			forward = dir;
		}

		bool IsHeadingForCollision()
		{
			RaycastHit hit;
			if (Physics.SphereCast(position, forward, settings.boundsRadius, settings.collisionAvoidDst, out hit, null))
			{
				return true;
			}
			else { }
			return false;
		}

		Vector3 ObstacleRays()
		{
			Vector3[] rayDirections = BoidHelper.directions;
			RaycastHit hit;
			for (int i = 0; i < rayDirections.Length; i++)
			{
				Vector3 dir = (new Quaternion(cachedTransform.Rotation) * new Quaternion(rayDirections[i])) * Vector3.Up;
				if (!Physics.SphereCast(position, dir, settings.boundsRadius, settings.collisionAvoidDst, out hit, null))
				{
					return dir;
				}
			}

			return forward;
		}

		Vector3 SteerTowards(Vector3 vector)
		{
			Vector3 v = vector.Normalized() * settings.maxSpeed - velocity;
			return Vector3.ClampLength(v, settings.maxSteerForce);
		}

	}
}
