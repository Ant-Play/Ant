namespace Ant
{
    public static class SceneManager
    {
        public static void LoadScene(string scene)
        {
            if (!InternalCalls.SceneManager_IsSceneValid(scene))
            {
                Log.Error("SceneManager: Tried to a scene with an invalid filepath '{0}'", scene);
                return;
            }

            Scene.OnSceneChange();
            InternalCalls.SceneManager_LoadScene(scene);
        }

        /*public static ulong GetCurrentSceneID() => InternalCalls.SceneManager_GetCurrentSceneID();

		public static void LoadScene(ulong sceneID)
		{
			if (!InternalCalls.SceneManager_IsSceneIDValid(sceneID))
			{
				Log.Error("SceneManager: Tried to a scene with an invalid ID '{0}'", sceneID);
				return;
			}

			Scene.OnSceneChange();
			InternalCalls.SceneManager_LoadSceneByID(sceneID);
		}*/
    }
}