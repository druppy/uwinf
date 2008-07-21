#ifndef __FILEDLG_H_INCLUDED
#define __FILEDLG_H_INCLUDED

class BIFCLASS FStdFileDialog : public FModalDialog {
        FILEDLG fild;
public:
        FStdFileDialog() : FModalDialog() {}

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
                     char *filenameBuffer, int maxFilenameBuf,
                     char **filename, int maxFiles,
                     const char *filterMask=0,
                     const char *dlgTitle=0, const char *okButtonText=0
                    );
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
                     char *filenameBuffer, int maxFilenameBuf,
                     char **filename, int maxFiles,
                     const char *filterMask=0,
                     const char *dlgTitle=0, const char *okButtonText=0
                    )
        {
                return FStdFileDialog::DoModal(pOwner,
                                               FStdFileDialog::sfd_open,
                                               filenameBuffer,maxFilenameBuf,
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
