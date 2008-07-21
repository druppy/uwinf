#ifndef __HWMOUSE_HPP
#define __HWMOUSE_HPP

class HWMouse {
public:
        HWMouse();
        ~HWMouse();

        int Initialize();
        void Terminate();

        void SetPointerShape(void *);
        void QueryPosition(int *column, int *row);
        void SetPosition(int column, int row);
        unsigned queryCurrentButtonStatus();

        void ShowPointer();
        void HidePointer(int lx, int ty, int rx, int by);
        void HidePointer();
        void SetScreenSize(int cx, int cy);
};

extern HWMouse hwmouse_internal;

#endif

