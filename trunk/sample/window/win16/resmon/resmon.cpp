#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_CONTROLS
#define BIFINCL_STATICTEXT
#define BIFINCL_APPLICATION
#define BIFINCL_COMMONHANDLERS
#include <bif.h>

#include "resmon.h"

//we need timers in order to update the contents of the dialog, so we define a timer handler and a timer event
struct BIFCLASS TimerEvent : FEvent {
        int GetTimerID() const { return (int)wParam; }
};

class BIFCLASS TimerHandler : public FHandler {
public:
        TimerHandler(FWnd *pwnd)
          : FHandler(pwnd)
          { SetDispatch(dispatchFunc_t(&TimerHandler::Dispatch)); }

        Bool StartTimer(int interval, int timerID);
        Bool StopTimer(int timerID);
        
protected:
        Bool Dispatch(FEvent &ev);

        virtual Bool Timer(TimerEvent &) { return False; }
};

Bool TimerHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_TIMER) {
                return Timer((TimerEvent&)ev);
        } else
                return False;
}

Bool TimerHandler::StartTimer(int interval, int timerID) {
        if(SetTimer(GetWnd()->GetHwnd(), timerID, interval, NULL))
                return True;
        else
                return False;
}

Bool TimerHandler::StopTimer(int timerID) {
	return (Bool)KillTimer(GetWnd()->GetHwnd(),timerID);
}




class BIFCLASS ResMonDialog : public FDialog,
		     public TimerHandler,
		     public FCreateDestroyHandler
{
	FStaticText c_SystemFree,
		    c_GDIFree,
		    c_UserFree;
public:
	ResMonDialog()
	  : FDialog(),
	    TimerHandler(this),
	    FCreateDestroyHandler(this)
	  {}

	Bool Create(FWnd *pOwner) {
		return FDialog::Create(pOwner,DIALOG_RESMON);
	}
	Bool CreateControls();

protected:
	Bool Timer(TimerEvent &ev);
	Bool Created(FCreatedEvent &);
	Bool Destroying(FDestroyingEvent &);
};

Bool ResMonDialog::CreateControls() {
	if(!c_SystemFree.Create(this,ID_SYSTEMFREE))
		return False;
	if(!c_GDIFree.Create(this,ID_GDIFREE))
		return False;
	if(!c_UserFree.Create(this,ID_USERFREE))
		return False;
	return True;
}

Bool ResMonDialog::Timer(TimerEvent &ev) {
        if(ev.GetTimerID()==1) {
                //update controls
                UINT free;
                free=GetFreeSystemResources(GFSR_SYSTEMRESOURCES);
                c_SystemFree.SetText_u(free);
                free=GetFreeSystemResources(GFSR_GDIRESOURCES);
                c_GDIFree.SetText_u(free);
                free=GetFreeSystemResources(GFSR_USERRESOURCES);
                c_UserFree.SetText_u(free);
		return True;
        } else
                return False;
}

Bool ResMonDialog::Created(FCreatedEvent &) {
        //create timer when the native window has been created
        StartTimer(1000,1);
        return False;   //pass on notification
}

Bool ResMonDialog::Destroying(FDestroyingEvent &) {
        //destroy timer before the native window is destroyed
        StopTimer(1);
        return False;   //pass on notification
}





class BIFCLASS ResMonApp : public FApplication {
        ResMonDialog dlg;
public:
        Bool StartUp(int,char**) {
                if(!dlg.Create(0)) return False;
                dlg.Show();
                dlg.BringToFront();
                return True;
        }
        int ShutDown() {
                dlg.Destroy();
                return 0;
        }
};

DEFBIFWINDOWMAIN(ResMonApp);

