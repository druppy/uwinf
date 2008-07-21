/*************************************************************************
MODULE
	WHello.Cpp
DESCRIPTION
	BIF Windows "hello world" program, to demonstrate simplicity of
	using BIF !

	The only thisg happening in this program is bringing a window
	to live and then write the "Hello World" string in its center.

	This is mend as a small generik program for users to get an
	starting point for there own program.
HISTORY
	17.02.97 : First version.
	03.03.97 : Considering it done !
*************************************************************************/
#define BIFINCL_WINDOW
#define BIFINCL_APPWINDOW
#define BIFINCL_STATICTEXT
#define BIFINCL_APPLICATION
#include <bif.h>

///////////////////////
// The program window
class MyWindow : public FAppWindow {
public:
	Create( void )
	{
		FAppWindow::Create( NULL, 0, fcf_sysmenu|fcf_titlebar|fcf_sizeborder, 0 );
		m_text.Create( this, -1, NULL, FStaticText::st_center, "Hello World" );
		SetClient( &m_text );
		Show();
		BringToFront();
	}

private:
	FStaticText m_text;
};

/////////////////////////////
// The Application instance

class MyApplication : public FApplication {
public:
	Bool StartUp( int argc, char **argv )
	{
		m_mainwnd.Create();
		return( True );
	}
private:
	MyWindow m_mainwnd;
};

DEFBIFWINDOWMAIN( MyApplication );

//////////////////////
// End of WHello.Cpp
