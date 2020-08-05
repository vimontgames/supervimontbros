#include "Precomp.h"
#include "SuperVimontBros/SuperVimontBros.h"

#include <iostream>
#include <sstream>

// Redirect SFML errors to debug output
class dbg_stream_for_cout : public std::stringbuf
{
public:
	~dbg_stream_for_cout() { sync(); }
	int sync()
	{
		::OutputDebugStringA(str().c_str());
		str(std::string()); // Clear the string buffer
		return 0;
	}
};
dbg_stream_for_cout g_DebugStreamFor_cout;

#if 0
int main()
#else
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#endif 
{	
	//assert(!"attach");

	std::cout.rdbuf(&g_DebugStreamFor_cout);
	std::streambuf* previous = sf::err().rdbuf(&g_DebugStreamFor_cout);

	// Run Game
	{
		SuperVimontBros game;

		game.m_name = "SuperVimontBros";
		game.m_scale = 3; 
		game.m_screenSize = { 1920 / game.m_scale, 1080 / game.m_scale };

		#ifdef _DEBUG
		game.m_fullscreen = false;
		game.m_debugDisplay = true;
		#elif defined(_RELEASE)
		game.m_fullscreen = false;
		game.m_debugDisplay = false;
		#elif defined(_FINAL)
		game.m_fullscreen = false;
		game.m_debugDisplay = false;
		#else
		#error Missing _DEBUG/_RELEASE/_FINAL target configuration
		#endif

		game.init();
		{
			game.run();
		}
		game.deinit();
	}
	
	return 0;
}