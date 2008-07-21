#include "scanner.h"
#include "bfile.h"
#include <string.h>


inline int isLiteralStart(char c) {
        //is the character a possible start on a literal
        return (c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_';
}
inline int isLiteralCont(char c) {
        return isLiteralStart(c) || (c>='0' && c<='9');
}

static void skip(BufferedFile *fp, const char *chars);
static void doNumber(BufferedFile *fp, char *buf, char c);
static void doLiteral(BufferedFile *fp, char *buf, char c);

token_class getToken(BufferedFile *fp, char *buf) {
        skip(fp," \t");
        int c;

        c=fp->get();
        switch(c) {
                case BufferedFile::Eof: {
                        *buf='\0';
                        return tc_eof;
                }
                case '\n': {
                        *buf++='\n';
                        *buf='\0';
                        return tc_eol;
                }
                case ',': {
                        *buf++=',';
                        *buf='\0';
                        return tc_comma;
                }
                case '#': {
                        //return the whole line a a token
                        *buf++=(char)c;                   
                        while((c=fp->get())!=fp->Eof && c!='\n') {
                                *buf++=(char)c;
                        }
                        *buf='\0';
                        return tc_ppd;
                }
                case '"': {
                        //Note: we do not handle concatenated strings
                        *buf++=(char)c;
                        while((c=fp->get())!=fp->Eof && c!='"') {
                                *buf++=(char)c;
                        }
                        *buf++=(char)c;
                        *buf='\0';
                        return tc_string;

                }
                case '/': {
                        //check for comment start
                        c=fp->get();
                        if(c=='*') {
                                *buf++='/';
                                *buf++='*';
                                *buf='\0';
                                return tc_commentstart;
                        } else if(c=='/') {
                                //C++ comment
                                *buf++='/';
                                *buf++='/';
                                while((c=fp->get())!=fp->Eof && c!='\n') {
                                        *buf++=(char)c;
                                }
                                *buf++='\0';
                                return tc_cppcomment;
                        } else {
                                fp->unget((char)c);                         
                                *buf++=(char)c;
                                *buf='\0';
                                return tc_unknown;
                        }
                }
                case '-': {
                        //could be negative number
                        c=fp->get();
                        if(c>='0' || c<='9')
                                *buf++='-';
                        else
                                fp->unget((char)c);
                }
                //*fallthrough*
                default: {
                        if(c>='0' && c<='9') {
                                doNumber(fp,buf,(char)c);
                                return tc_number;
                        } else if(isLiteralStart((char)c)) {
                                doLiteral(fp,buf,(char)c);
                                return tc_literal;
                        } else {
                                *buf++=(char)c;
                                *buf='\0';
                                return tc_unknown;
                        }
                }
        }
}


static void skip(BufferedFile *fp, const char *chars) {
        //skip until eof or char not in chars
        int c;
        do {
                c=fp->get();
        } while(c!=fp->Eof && strchr(chars,c));
        if(c!=fp->Eof) fp->unget((char)c);
}


static void doNumber(BufferedFile *fp, char *buf, char c) {
        /* We could recognize decimal/hexadecimal&octal numbers and stop when
         * something illegal occurs. This is not worth it since MS-RC
         * accepts "87F" as a resource name and since we are probably dealing
         * with a correct resource file
         */
        doLiteral(fp,buf,(char)c);
}


static void doLiteral(BufferedFile *fp, char *buf, char c) {
        int nc=c;
        do {
                *buf++=(char)nc;
                nc=fp->get();
        } while(nc!=fp->Eof && isLiteralCont((char)nc));
        *buf='\0';
        if(nc!=fp->Eof) fp->unget((char)nc);
}
