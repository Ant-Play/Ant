using System;
using Ant;

namespace FPSExample
{
	public class FPSPlayer : Entity
	{
		public float WalkingSpeed = 10.0f;
		public float RunSpeed = 20.0f;
		public float JumpForce = 50.0f;

		public Prefab ProjectilePrefab;

		public Entity[] EntityArray;

		[NonSerialized]
		public float MouseSensitivity = 10.0f;

		private bool m_Colliding = false;
		public float CurrentSpeed { get; private set; }

		public string Hello = "Hello my name is Peter Nilsson";

		private RigidBodyComponent m_RigidBody;
		private TransformComponent m_Transform;
		private TransformComponent m_CameraTransform;

		private Entity m_CameraEntity;

		private Vector2 m_LastMousePosition;

		private float m_CurrentYMovement = 0.0f;

		private Vector2 m_MovementDirection = new Vector2(0.0F);
		private bool m_ShouldJump = false;

		private Vector3 m_PreviousPosition;
		float m_DistanceTraveled = 0.0f;
		//private SimpleSound[] m_FootstepSounds = new SimpleSound[10];
		private Ant.Random rand = new Ant.Random();
		public int PlayFootstepSounds = 0;
		public float FootstepLength_m = 1.4f;
		public float FootstepsVolume = 0.22f;

		private RaycastData m_RaycastData;

		private float m_ShootingCooldown = 0.15f;
		private float m_ShootingCooldownTimer = 0.0f;

		private Entity m_ProjectileParent;

		private readonly AudioCommandID m_PlayShotSoundGraphSoundID = new AudioCommandID("play_soundgraph");
		private readonly AudioCommandID m_PlayShotSoundID = new AudioCommandID("play_nonsoundgraph");
		private readonly AudioCommandID m_PlayFootstepID = new AudioCommandID("play_footstep");

		/*private void GetFoostepSounds()
		{
			for (int i = 0; i < m_FootstepSounds.Length; ++i)
			{
				string filepath;
				if (i < 9)
				{
					filepath = String.Format("audio/AudioDemo/footsteps/footstep-0{0}.wav", i + 1);
					m_FootstepSounds[i] = new SimpleSound(filepath);
				}
				else
				{
					filepath = String.Format("audio/AudioDemo/footsteps/footstep-{0}.wav", i + 1);
					m_FootstepSounds[i] = new SimpleSound(filepath);
				}
			}
		}*/

		private void PlayFootstepSound()
		{
			float rv = Ant.Random.Float() * 0.06f;
			float rp = Ant.Random.Float() * 0.3f;
			//int idx = rand.Next(m_FootstepSounds.Length);
			//Audio.PlaySound2D(m_FootstepSounds[idx], FootstepsVolume - rv, 1.0f - rp);

			Audio.PostEventAtLocation(m_PlayFootstepID, Transform.WorldTransform.Position, Transform.WorldTransform.Rotation);

			m_DistanceTraveled = 0.0f;
		}

		protected override void OnCreate()
		{
			Audio.PreloadEventSources(m_PlayShotSoundGraphSoundID);
			Audio.PreloadEventSources(m_PlayShotSoundID);

			m_Transform = GetComponent<TransformComponent>();
			m_RigidBody = GetComponent<RigidBodyComponent>();

			CurrentSpeed = WalkingSpeed;

			CollisionBeginEvent += (n) => { m_Colliding = true; };
			CollisionEndEvent += (n) => { m_Colliding = false; };

			m_CameraEntity = Scene.FindEntityByTag("Camera");
			m_CameraTransform = m_CameraEntity.GetComponent<TransformComponent>();

			m_LastMousePosition = Input.GetMousePosition();

			Input.SetCursorMode(CursorMode.Locked);

			// Setup footsteps
			//GetFoostepSounds();
			m_PreviousPosition = Transform.Translation;
			FootstepLength_m = Math.Max(0.0f, FootstepLength_m);
			if (FootstepsVolume < 0.0f)
				FootstepsVolume = 0.22f;
			else
				FootstepsVolume = Math.Min(1.0f, FootstepsVolume);

			m_RaycastData = new RaycastData();
			m_RaycastData.MaxDistance = 50.0f;
			m_RaycastData.RequiredComponents = new[] { typeof(MeshComponent) };

			m_ProjectileParent = Scene.FindEntityByTag("ProjectileHolder");
		}

		protected override void OnUpdate(float ts)
		{
			if (Input.IsKeyPressed(KeyCodes.Escape) && Input.GetCursorMode() == CursorMode.Locked)
				Input.SetCursorMode(CursorMode.Normal);

			if (Input.IsMouseButtonPressed(MouseButton.Left) && Input.GetCursorMode() == CursorMode.Normal)
            {
				Input.SetCursorMode(CursorMode.Locked);
				m_LastMousePosition = Input.GetMousePosition();
            }

			CurrentSpeed = Input.IsKeyPressed(KeyCodes.LeftControl) ? RunSpeed : WalkingSpeed;

			UpdateRaycasting();
			UpdateMovementInput();
			UpdateRotation(ts);
			UpdateShooting(ts);

			// Play footstep sounds
			m_DistanceTraveled += Transform.Translation.Distance(m_PreviousPosition);
			m_PreviousPosition = Transform.Translation;
			if (m_DistanceTraveled >= FootstepLength_m)
			{
				// Making sure we're touching the ground
				if (m_Colliding && PlayFootstepSounds == 1)
				{
					PlayFootstepSound();
				}
			}
		}

		private void SpawnProjectile()
        {
			if (!ProjectilePrefab)
				return;

            Entity projectile = Scene.InstantiatePrefab(ProjectilePrefab);
            var rb = projectile.GetComponent<RigidBodyComponent>();
            rb.Translation = Transform.Translation + m_CameraTransform.WorldTransform.Forward;

			if (m_ProjectileParent != null)
				projectile.Parent = m_ProjectileParent;

			Vector3 force = m_CameraTransform.WorldTransform.Forward * 20.0f;
            rb.AddForce(force, ForceMode.Impulse);
        }

        private void SpawnProjectileContainer()
        {
            if (!ProjectilePrefab)
                return;

            Entity projectile = Scene.InstantiatePrefab(ProjectilePrefab);
            var rb = projectile.GetComponent<RigidBodyComponent>();
            rb.Translation = Transform.Translation + m_CameraTransform.WorldTransform.Forward;

            if (m_ProjectileParent != null)
                projectile.Parent = m_ProjectileParent;

            Vector3 force = m_CameraTransform.WorldTransform.Forward * 20.0f;
            rb.AddForce(force, ForceMode.Impulse);
        }


        private void UpdateShooting(float ts)
        {
			m_ShootingCooldownTimer -= ts;
			bool trigger = GetTriggerAxis() > 0.0f;

			if ((trigger || Input.IsMouseButtonPressed(MouseButton.Left)) && m_ShootingCooldownTimer < 0.0f)
			{
				m_ShootingCooldownTimer = m_ShootingCooldown;
				SpawnProjectileContainer();

				Audio.PostEventAtLocation(m_PlayShotSoundGraphSoundID, m_Transform.WorldTransform.Position, m_Transform.WorldTransform.Rotation);
			}
			else if (Input.IsMouseButtonPressed(MouseButton.Right) && m_ShootingCooldownTimer < 0.0f)
			{
				m_ShootingCooldownTimer = m_ShootingCooldown;
				SpawnProjectileContainer();
				
				Audio.PostEventAtLocation(m_PlayShotSoundID, m_Transform.WorldTransform.Position, m_Transform.WorldTransform.Rotation);
			}
		}

		private float GetMovementAxis(int axis, float deadzone = 0.2f)
        {
            float value = Input.GetControllerAxis(0, axis);
            return Mathf.Abs(value) < deadzone ? 0.0f : value;
        }

		private float GetHorizontalMovementAxis(float deadzone = 0.2f)
        {
            // Dpad
            byte hat = Input.GetControllerHat(0, 0);
			if ((hat & 2) != 0)
				return 1.0f;
            if ((hat & 8) != 0)
                return -1.0f;

			// Analogue stick
			return GetMovementAxis(0, deadzone);
		}

        private float GetVerticalMovementAxis(float deadzone = 0.2f)
        {
			// Dpad
            byte hat = Input.GetControllerHat(0, 0);
            if ((hat & 4) != 0)
                return 1.0f;
            if ((hat & 1) != 0)
                return -1.0f;

            // Analogue stick
            return GetMovementAxis(1, deadzone);
		}

        private float GetTriggerAxis(float deadzone = 0.2f)
        {
            return GetMovementAxis(5, deadzone);
        }

        private void UpdateMovementInput()
		{
			m_MovementDirection.X = 0.0f;
			m_MovementDirection.Y = 0.0f;

			m_MovementDirection.X = GetHorizontalMovementAxis();
			m_MovementDirection.Y = -GetVerticalMovementAxis();

			if (Input.IsKeyDown(KeyCodes.W))
				m_MovementDirection.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCodes.S))
				m_MovementDirection.Y = -1.0f;

			if (Input.IsKeyDown(KeyCodes.A))
				m_MovementDirection.X = -1.0f;
			else if (Input.IsKeyDown(KeyCodes.D))
				m_MovementDirection.X = 1.0f;

			m_ShouldJump = (Input.IsKeyPressed(KeyCodes.Space) || Input.IsControllerButtonPressed(0, 0)) && !m_ShouldJump;
		}

		Collider[] colliders = new Collider[10];

		private void UpdateRaycasting()
		{
			RaycastHit hitInfo;
			m_RaycastData.Origin = m_CameraTransform.Translation + m_CameraTransform.WorldTransform.Forward * 2.0f;
			m_RaycastData.Direction = m_CameraTransform.WorldTransform.Forward;

			const bool s_RaycastWithStruct = true;
			if (s_RaycastWithStruct)
			{
					if (Input.IsKeyPressed(KeyCodes.H) && Physics.Raycast(m_RaycastData, out hitInfo))
					{
						Entity entity = Scene.FindEntityByID(hitInfo.EntityID);
						TagComponent tag = entity.GetComponent<TagComponent>();
						MeshComponent mesh = entity.GetComponent<MeshComponent>();

						Log.Info("Raycast hit: {0}", tag.Tag);
						mesh.Mesh.BaseMaterial.AlbedoColor = new Vector3(1.0f, 0.0f, 0.0f);
					}
			}
			else
			{
				if (Input.IsKeyPressed(KeyCodes.H) && Physics.Raycast(m_CameraTransform.Translation + m_CameraTransform.WorldTransform.Forward * 2.0f, m_CameraTransform.WorldTransform.Forward, 50.0f, out hitInfo, typeof(MeshComponent)))
				{
					Entity entity = Scene.FindEntityByID(hitInfo.EntityID);
					TagComponent tag = entity.GetComponent<TagComponent>();
					MeshComponent mesh = entity.GetComponent<MeshComponent>();

					Log.Info("Raycast hit: {0}", tag.Tag);
					mesh.Mesh.BaseMaterial.AlbedoColor = new Vector3(1.0f, 0.0f, 0.0f);
				}
			}

			if (Input.IsKeyPressed(KeyCodes.L))
			{
				// NOTE: The NonAlloc version of Overlap functions should be used when possible since it doesn't allocate a new array
				//			whenever you call it. The normal versions allocates a brand new array every time.

				int numColliders = Physics.OverlapBoxNonAlloc(m_Transform.Translation, new Vector3(1.0f), colliders);

				Log.Info("Colliders: {0}", numColliders);

				// When using NonAlloc it's not safe to use a for each loop since some of the colliders may not exist
				for (int i = 0; i < numColliders; i++)
					Log.Info(colliders[i]);
			}
		}

		protected override void OnPhysicsUpdate(float fixedTimeStep)
		{
			UpdateMovement();
		}

		private void UpdateRotation(float ts)
		{
			if (Input.GetCursorMode() != CursorMode.Locked)
				return;


			{
				float sensitivity = 3.0f;

				float hAxis = -GetMovementAxis(2);
				float vAxis = -GetMovementAxis(3);
				Vector2 delta = new Vector2(hAxis * hAxis, vAxis * vAxis);
				delta *= new Vector2(Math.Sign(hAxis), Math.Sign(vAxis));
				m_CurrentYMovement = delta.X * sensitivity;
				float xRotation = delta.Y * sensitivity * ts;

				if (xRotation != 0.0f)
					m_CameraTransform.Rotation += new Vector3(xRotation, 0.0f, 0.0f);

				m_CameraTransform.Rotation = new Vector3(Mathf.Clamp(m_CameraTransform.Rotation.X * Mathf.Rad2Deg, -80.0f, 80.0f), 0.0f, 0.0f) * Mathf.Deg2Rad;
			}

            // Mouse
            {
                // TODO: Mouse position should be relative to the viewport
                Vector2 currentMousePosition = Input.GetMousePosition();
                Vector2 delta = m_LastMousePosition - currentMousePosition;
				if (delta.X != 0.0f)
					m_CurrentYMovement = delta.X * MouseSensitivity * ts;
				float xRotation = delta.Y * (MouseSensitivity * 0.05f) * ts;

				if (xRotation != 0.0f)
					m_CameraTransform.Rotation += new Vector3(xRotation, 0.0f, 0.0f);

				m_CameraTransform.Rotation = new Vector3(Mathf.Clamp(m_CameraTransform.Rotation.X * Mathf.Rad2Deg, -80.0f, 80.0f), 0.0f, 0.0f) * Mathf.Deg2Rad;
				m_LastMousePosition = currentMousePosition;
			}
		}

		private void UpdateMovement()
		{
            if (Input.GetCursorMode() != CursorMode.Locked)
                return;

			Rotation += m_CurrentYMovement * Vector3.Up * 0.05f;

			Vector3 movement = m_CameraTransform.WorldTransform.Right * m_MovementDirection.X + m_CameraTransform.WorldTransform.Forward * m_MovementDirection.Y;
			movement.Y = 0.0f;

			if (movement.Length() > 0.0f)
			{
				movement.Normalize();
				Vector3 velocity = movement * CurrentSpeed;
				velocity.Y = m_RigidBody.LinearVelocity.Y;
				m_RigidBody.LinearVelocity = velocity;
			}

			if (m_ShouldJump && m_Colliding)
			{
				m_RigidBody.AddForce(Vector3.Up * JumpForce, ForceMode.Impulse);
				m_ShouldJump = false;
			}
		}
	}
}
