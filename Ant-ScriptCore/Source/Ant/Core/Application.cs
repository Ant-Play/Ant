
namespace Ant
{
    public static class Application
    {
        public static uint Width => InternalCalls.Application_GetWidth();
        public static uint Height => InternalCalls.Application_GetHeight();
        public static float AspectRatio => Width / (float)Height;

        public static void Quit() => InternalCalls.Application_Quit();

    }
}