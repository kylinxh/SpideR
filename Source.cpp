#include "Factory.h"
#define Windows
#include <map>
#include <string>
#include <functional>
#include "Manager.h"
#include "Connection.h"

Factory* factory = nullptr;
bool IsRunning = true;
Console* display = nullptr;


int main() {

#ifdef Windows
	factory = new WindowsFactory;
#else
	factory = new LinuxFactory;
#endif // Windows

	std::string t = "test";
	Manager::instance().RegisterCommand("help", [=](void*) {display->WriteOut(t); });
	Manager::instance().RegisterCommand("connect", [=](void*) { Connection c; c.MakeConnection("https://forums.bakabt.me/index.php?topic=46346.0"); });
	//Manager::instance().RegisterCommand("err", [=](std::string err) { display->WriteOut(err); });

	display = factory->create_context();
	display->Display();

	//Manager::instance().FireCommand("Help");

//	HelpCommand h(display, &Console::WriteOut, t);
//	h.execute();
//	HelpCommand();


	while (IsRunning)
	{
		display->Input();
		//main loop2

	}
}