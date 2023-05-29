using System;
using Ant;

namespace ThirdPersonExample
{
	// These scripts are intended to be used on a "third person" character, set up as follows:
	// 
	// Parent entity: e.g. the player.
	//    - Transform component                  <-- the position, rotation, and scale of the player character
	//    - Character controller component       <-- a physics "actor" governing interaction between the player character and the physics of the game world
	//    - Capsule collider component           <-- a physics collider approximating the players shape
	//    - Script component: PlayerController   <-- Contains game logic for moving the player character
	//    - Child entity: Camera                 <-- camera is a child of the player. So that it moves with the player
	//        - Transform component              <-- the camera transform is just the distance that it's offset from the player, and which direction its facing in.
	//        - Script component:                <-- Contains game logic for moving the camera
	//    - Child entity: Model                  <-- This is the actual model (aka "mesh") for the player character
	//
	// The player and camera controllers work in concert to provide the following functionality:
	// - WASD keys move the player forwards, left, right, backwards relative to the direction the camera is facing.
	// - Left shift key to run
	// - Spacebar to jump
	// - Mouse changes direction that the camera is facing.
	// - Escape key: stops camera movement so that you can (for example) fiddle around with settings in AntPlay.
	// - F5 key: resumes camera movement

	public class CameraController : Entity
	{

		public float DistanceFromPlayer = 8.0f;
		public float MouseSensitivity = 1.0f;

		private Vector2 m_LastMousePosition;

		private bool m_CameraMovementEnabled = true;

		protected override void OnUpdate(float ts)
		{

			if (Input.IsKeyPressed(KeyCodes.Escape) && m_CameraMovementEnabled)
			{
				m_CameraMovementEnabled = false;
			}

			if (Input.IsKeyPressed(KeyCodes.F5) && !m_CameraMovementEnabled)
			{
				m_CameraMovementEnabled = true;
			}

			Vector2 currentMousePosition = Input.GetMousePosition();
			if (m_CameraMovementEnabled)
			{
				Vector2 delta = m_LastMousePosition - currentMousePosition;
				Vector3 rotation = Transform.Rotation;
				Vector3 translation = Transform.Translation;
				rotation.Y = (rotation.Y - delta.X * MouseSensitivity * ts) % Mathf.TwoPI;

				translation.X = DistanceFromPlayer * Mathf.Sin(rotation.Y);
				translation.Z = DistanceFromPlayer * -Mathf.Cos(rotation.Y);

				Transform.Translation = translation;
				Transform.Rotation = rotation;
			}
			m_LastMousePosition = currentMousePosition;
		}

	}


	public class PlayerController : Entity
	{

		public float WalkSpeed = 1.5f;
		public float RunSpeed = 3.0f;
		public float RotationSpeed = 10.0f;
		public float JumpPower = 3.0f;

		private TransformComponent m_CameraTransform;
		private TransformComponent m_ModelTransform;

		private CharacterControllerComponent m_CharacterController;
		private Vector3 m_LastMovement;

		protected override void OnCreate()
		{
			// Find the camera's transform component.
			// We use this to figure out which in direction the WASD keys move the player
			Entity camera = Scene.FindEntityByTag("Camera");
			if (camera != null)
			{
				m_CameraTransform = camera.GetComponent<TransformComponent>();
			}
			else
			{
				Log.Error("'Camera' entity was not found!");
			}

			Entity child = Scene.FindEntityByTag("Model");
			m_ModelTransform = child.GetComponent<TransformComponent>();

			if (m_ModelTransform == null)
			{
				Log.Error("Could not find Player model's transform.");
			}

			m_CharacterController = GetComponent<CharacterControllerComponent>();
			if (m_CharacterController == null)
			{
				Log.Error("Could not find Player model's transform.");
			}
		}

		protected override void OnUpdate(float ts)
		{
			float X = 0.0f;
			float Z = 0.0f;
			bool keyDown = false;
			Vector3 desiredRotation = m_ModelTransform.Rotation;

			if (Input.IsKeyDown(KeyCodes.W))
			{
				Z = 1.0f;
				keyDown = true;
				desiredRotation.Y = -m_CameraTransform.Rotation.Y;
			}
			else if (Input.IsKeyDown(KeyCodes.S))
			{
				Z = -1.0f;
				keyDown = true;
				desiredRotation.Y = -m_CameraTransform.Rotation.Y + Mathf.PI;
			}

			if (Input.IsKeyDown(KeyCodes.A))
			{
				X = -1.0f;
				keyDown = true;
				desiredRotation.Y = -m_CameraTransform.Rotation.Y + (Mathf.PI / 2.0f);
			}
			else if (Input.IsKeyDown(KeyCodes.D))
			{
				X = 1.0f;
				keyDown = true;
				desiredRotation.Y = -m_CameraTransform.Rotation.Y - (Mathf.PI / 2.0f);
			}

			if (m_CharacterController != null)
			{
				if (keyDown)
				{
					// rotate to face camera direction
					Vector3 currentRotation = m_ModelTransform.Rotation;

					float diff1 = Mathf.Modulo(desiredRotation.Y - currentRotation.Y, Mathf.TwoPI);
					float diff2 = Mathf.Modulo(currentRotation.Y - desiredRotation.Y, Mathf.TwoPI);
					bool useDiff2 = diff2 < diff1;

					float delta = useDiff2 ? Mathf.Max(-diff2, -Mathf.Abs(RotationSpeed * ts)) : Mathf.Min(diff1, Mathf.Abs(RotationSpeed * ts));

					currentRotation.Y += delta;
					m_ModelTransform.Rotation = currentRotation;

					if (Math.Abs(delta) < 0.001f)
					{
						Vector3 movement = (m_CameraTransform.LocalTransform.Right * X) + (m_CameraTransform.LocalTransform.Forward * Z);
						movement.Normalize();
						bool shiftDown = Input.IsKeyDown(KeyCodes.LeftShift);
						float speed = shiftDown ? RunSpeed : WalkSpeed;
						movement *= speed * ts;

						// We could simply move the character according to player's key presses and leave it at that.
						// Like this:
						//    m_CharacterController.Move(movement);
						//
						// However, that means the player can still control movement of the character even if it is
						// in mid-air.
						//
						// If you don't want the character to be able to change direction mid-air, then you
						// can use the IsGrounded property to check whether or not to move player, like the following code:
						//
						// The check on speed is needed so that the character will move down slopes more smoothly
						if (m_CharacterController.IsGrounded || m_CharacterController.SpeedDown < (shiftDown ? 4.0f : 2.0f))
						{
							m_CharacterController.Move(movement);
							m_LastMovement = movement / ts;
						}
						else
						{
							// If not grounded, carry on moving per last grounded movement.
							// This gives the effect of momentum, rather than just falling straight down.
							// Note adjustment by ts is in case frame times are not exactly the same.
							m_CharacterController.Move(m_LastMovement * ts);
						}
					}
				}
				if (m_CharacterController.IsGrounded && Input.IsKeyDown(KeyCodes.Space))
				{
					m_CharacterController.Jump(JumpPower);
				}
			}
		}
	}

	public class PlayerControllerAnimated : Entity
	{

		public float RotationSpeed = 10.0f;
		public float JumpPower = 3.0f;

		private CharacterControllerComponent m_CharacterController;
		private TransformComponent m_CameraTransform;
		private TransformComponent m_ModelTransform;
		private AnimationComponent m_Animation;
		private Vector3 m_LastMovement = Vector3.Zero;
		private Entity m_ModelEntity = null;

		protected override void OnCreate()
		{
			m_CharacterController = GetComponent<CharacterControllerComponent>();
			foreach (Entity child in Children)
			{
				// Find the player model's transform and animation components.
				// We use these to rotate the model in the direction of movement, and control animation
				if (child.GetComponent<TagComponent>().Tag == "Model")
				{
					m_ModelEntity = child;
					m_ModelTransform = child.GetComponent<TransformComponent>();
					m_Animation = child.GetComponent<AnimationComponent>();
				}
				// Find the player camera's transform component.
				// We use this to figure out which direction the WASD keys move the player
				if (child.GetComponent<TagComponent>().Tag == "Camera")
				{
					m_CameraTransform = child.GetComponent<TransformComponent>();
				}
			}

			if (m_CharacterController == null)
			{
				Log.Error("Could not find Player's character controller");
			}
			if (m_CameraTransform == null)
			{
				Log.Error("Could not find Player's camera!");
			}
			if (m_ModelTransform == null)
			{
				Log.Error("Could not find Player model's transform.");
			}
			if (m_Animation == null)
			{
				Log.Error("Could not find Player model's animation.");
			}
		}

		protected override void OnUpdate(float ts)
		{
			if (m_ModelTransform == null || m_CameraTransform == null || m_CharacterController == null || m_Animation == null)
			{
				return;
			}

			bool keyDown = false;
			bool shiftDown = false;
			Vector3 desiredRotation = m_ModelTransform.Rotation;

			if (Input.IsKeyDown(KeyCodes.W))
			{
				keyDown = true;
				desiredRotation.Y = -m_CameraTransform.Rotation.Y;
			}
			else if (Input.IsKeyDown(KeyCodes.S))
			{
				keyDown = true;
				desiredRotation.Y = -m_CameraTransform.Rotation.Y + Mathf.PI;
			}

			if (Input.IsKeyDown(KeyCodes.A))
			{
				keyDown = true;
				desiredRotation.Y = -m_CameraTransform.Rotation.Y + (Mathf.PI / 2.0f);
			}
			else if (Input.IsKeyDown(KeyCodes.D))
			{
				keyDown = true;
				desiredRotation.Y = -m_CameraTransform.Rotation.Y - (Mathf.PI / 2.0f);
			}

			if (keyDown)
			{
				shiftDown = Input.IsKeyDown(KeyCodes.LeftShift);

				// rotate to face camera direction first, then move
				Vector3 currentRotation = m_ModelTransform.Rotation;

				float diff1 = Mathf.Modulo(desiredRotation.Y - currentRotation.Y, Mathf.TwoPI);
				float diff2 = Mathf.Modulo(currentRotation.Y - desiredRotation.Y, Mathf.TwoPI);
				bool useDiff2 = diff2 < diff1;

				float delta = useDiff2? Mathf.Max(-diff2, -Mathf.Abs(RotationSpeed * ts)) : Mathf.Min(diff1, Mathf.Abs(RotationSpeed * ts));

				currentRotation.Y += delta;
				m_ModelTransform.Rotation = currentRotation;

				// There is no need to set the entity's position as this is entirely driven by the animation root motion
			}

			if (m_CharacterController.IsGrounded || m_CharacterController.SpeedDown < (shiftDown ? 4.0f : 2.0f))
			{
				m_Animation.StateIndex = keyDown ? Input.IsKeyDown(KeyCodes.LeftShift) ? 2u : 1u : 0u;

				if (m_CharacterController.IsGrounded && Input.IsKeyDown(KeyCodes.Space))
				{
					m_CharacterController.Jump(JumpPower);
				}

				// Store the animation's current root motion as "movement.
				// If the character leaves the ground, we will apply that movement to give the illusion of momentum.
				// (as opposed to instantly falling straight down, which would look weird)
				// The thing is, the animation root motion is in coordinate system relative to the model entity.
				// We need to transform to coordinate system of the player entity (which is the entity we wish to move).
				//
				// Strictly speaking, it should be something like this:   m_LastMovement = Matrix3(Inverse(ParentEntity.WorldTransform) * Matrix3(ModelEntity.WorldTransform) * m_Animation.RootMotion.Position
				// But we do not yet have:
				//    construction of Matrix4 from a Transform
				//    Inverse function for Matrix4
				//    A Matrix3 class
				//    Multiplication of Vector3 by Matrix3
				//
				// So, in the meantime, this shortcut will have to do:
				// 1) Assume that there is no Parent (i.e. player entity is at top level of scene)
				// 2) Assume that the model is not scaled (i.e. so the only thing we need to account for is rotation of the model entity)
				// If 1) and 2) hold, then we have enough stuff implemented to be able to do the transformation
				Quaternion quat = new Quaternion(m_ModelEntity.Rotation);
				m_LastMovement = quat * m_Animation.RootMotion.Position / ts;
			}
			else
			{
				// If not grounded, carry on moving per last grounded movement.
				// This gives the effect of momentum, rather than just falling straight down.
				// Note adjustment by ts is in case frame times are not exactly the same.
				m_CharacterController.Move(m_LastMovement * ts);
				m_Animation.StateIndex = 4;
			}
		}
	}


	class PhysicsTestTrigger : Entity
	{
		private SkyLightComponent m_Lights;

		protected override void OnCreate()
		{
			Entity skylight = Scene.FindEntityByTag("Sky Light");
			if (skylight != null)
			{
				m_Lights = skylight.GetComponent<SkyLightComponent>();
			}
			else
			{
				Log.Error("'Sky Light' entity was not found, or it did not have a SkyLightComponent!");
			}

			TriggerBeginEvent += OnTriggerBegin;
			TriggerEndEvent += OnTriggerEnd;
		}

		void OnTriggerBegin(Entity other)
		{
			Log.Trace("Trigger begin");
			m_Lights.Intensity = 0.0f;
		}

		void OnTriggerEnd(Entity other)
		{
			Log.Trace("Trigger end");
			m_Lights.Intensity = 1.0f;
		}

	}

}
