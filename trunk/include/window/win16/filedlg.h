#ifndef __FILEDLG_H_INCLUDED
#define __FILEDLG_H_INCLUDED

class __export StdFileHack;
class BIFCLASS FStdFileDialog : public FModalDialog {
        const char *okButtonText;
        friend class StdFileHack;
public:
        FStdFileDialog()
          : FModalDialog()
          { okButtonText=0; }

        enum sfd_type {
                sfd_open,
                sfd_saveas
        };

        Bool DoModal(FWnd *pOwner, sfd_type type,
                     char *filename,
                     const char *filterMask=0,
                     const char *dlgTitle=0, const char *okButtonText=0
                    );
        Bool DoModal(FWnd *pOwner, sfd_type type,
                     char *filenameBuffer, int maxFilenameBuffer,
                     char **filename, int maxFiles,
                     const char *filterMask=0,
                     const char *dlgTitle=0, const char *okButtonText=0
                    );

protected:
        void EndDialog(int iReturn) { ::EndDialog(GetHwnd(),iReturn); }
        Bool Command(FCommandEvent&) { return False; }
};

class BIFCLASS FStdOpenFileDialog : public FStdFileDialog {
public:
        FStdOpenFileDialog() : FStdFileDialog() {}

        Bool DoModal(FWnd *pOwner,
                     char *filename,
                     const char *filterMask=0,
                     const char *dlgTitle=0, const char *okButtonText=0
                    )
        {
                return FStdFileDialog::DoModal(pOwner,
                                               FStdFileDialog::sfd_open,
                                               filename,
                                               filterMask,
                                               dlgTitle,okButtonText
                                              );
        }

        Bool DoModal(FWnd *pOwner,
                     char *filenameBuffer, int maxFilenameBuffer,
                     char **filename, int maxFiles,
                     const char *filterMask=0,
                     const char *dlgTitle=0, const char *okButtonText=0
                    )
        {
                return FStdFileDialog::DoModal(pOwner,
                                               FStdFileDialog::sfd_open,
                                               filenameBuffer, maxFilenameBuffer,
                                               filename, maxFiles,
                                               filterMask,
                                               dlgTitle,okButtonText
                                              );
        }
};

class BIFCLASS FStdSaveFileDialog : public FStdFileDialog {
public:
        FStdSaveFileDialog() : FStdFileDialog() {}

        Bool DoModal(FWnd *pOwner,
                     char *filename,
                     const char *filterMask=0,
                     const char *dlgTitle=0, const char *okButtonText=0
                    )
        {
                return FStdFileDialog::DoModal(pOwner,
                                               FStdFileDialog::sfd_saveas,
                                               filename,
                                               filterMask,
                                               dlgTitle,okButtonText
                                              );
        }
};

#endif
