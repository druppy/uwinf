#include <stdio.h>

void OutputFileStart(const char *filename);
void OutputNewParagraph();
void OutputChar(char c);
void OutputFileEnd();
void CancelOutput(const char *filename);

void dir_todo(int argc, char **argv);
void dir_caution(int argc, char **argv);
void dir_ecaution(int argc, char **argv);
void dir_dl(int argc, char **argv);
void dir_edl(int argc, char **argv);
void dir_dt(int argc, char **argv);
void dir_dd(int argc, char **argv);
void dir_footnote(int argc, char **argv);
void dir_efootnote(int argc, char **argv);
void dir_topic(int argc, char **argv);
void dir_etopic(int argc, char **argv);
void dir_key(int argc, char **argv);
void dir_ref(int argc, char **argv);
void dir_reffootnote(int argc, char **argv);
void dir_hp(int argc, char **argv);
void dir_ehp(int argc, char **argv);
void dir_note(int argc, char **argv);
void dir_enote(int argc, char **argv);
void dir_ol(int argc, char **argv);
void dir_eol(int argc, char **argv);
void dir_li(int argc, char **argv);
void dir_parml(int argc, char **argv);
void dir_eparml(int argc, char **argv);
void dir_pt(int argc, char **argv);
void dir_pd(int argc, char **argv);
void dir_sl(int argc, char **argv);
void dir_esl(int argc, char **argv);
void dir_ul(int argc, char **argv);
void dir_eul(int argc, char **argv);
void dir_code(int argc, char **argv);
void dir_ecode(int argc, char **argv);
void dir_graphic(int argc, char **argv);

void error(const char *fmt, ...);
void warning(const char *fmt, ...);
void skipWhite();
void skipWhiteAndNL();

extern int currentLevel;
extern int autoIndex;
extern int inCode;
extern int multiHelp;

