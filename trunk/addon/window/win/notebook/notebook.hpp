struct FNoteBook_tab {
	PageId firstPage;
        char *tabName;
        FRect rect;
};

Bool leftScrollVisible, rightScrollVisible;
int firstVisiblePage,
    selectedTab,
    focusedTab;

struct FNoteBook_page {
	PageId page;
        int tab_index;
        Bool major;
        char *text;
        FWnd *wnd;
};
