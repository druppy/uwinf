#ifndef __SCANNER_H
#define __SCANNER_H

enum token_class {
        tc_unknown,
        tc_string,              // "...."
        tc_comma,               // ,
        tc_ppd,                 // #.......
        tc_commentstart,        // /*
        tc_cppcomment,          // //....
        tc_number,              // 12  45  0457 0x4F0
        tc_literal,             // hello  dialog  ws_popup  control
        tc_eol,                 // end of line
        tc_eof                  // end of file
};

class BufferedFile;
token_class getToken(BufferedFile *fp, char *buf);

#endif
