#include "bihc.h"
#include <string.h>
#include <stdlib.h>

#pragma off(unreferenced);

static FILE *ofp;
static FILE *toc;
static FILE *bi;

int inCode=0;                   //inside sourcecode (for automatic forced newlines)
static int insideTopic=0;       //inside topic or footnote
static int highlightLevel;
int currentLevel=1;

static void OutputString(const char*);

//list stuff
enum listtype { ordered_list, unordered_list, simple_list, parameter_list, definition_list };
static struct {
        listtype type;
        int indent1;
        int indent2;
        int totalindent;
        int count;
} liststack[30];
static int listsp=0;
#define SLIST_INDENT1      1
#define SLIST_INDENT2    549
#define OLIST_INDENT1    250
#define OLIST_INDENT2    300
#define ULIST_INDENT1    250
#define ULIST_INDENT2    300
#define DLIST_INDENT1      1
#define DLIST_INDENT2   1999
#define PLIST_INDENT1      1
#define PLIST_INDENT2   1999

static void PushList(listtype type, int indent1, int indent2) {
        liststack[listsp].type        = type;
        liststack[listsp].indent1     = indent1;
        liststack[listsp].indent2     = indent2;
        liststack[listsp].totalindent = indent1+indent2;
        liststack[listsp].count       = 0;
        listsp++;
        
        int base=0;
        int i1,i2;
        for(int s=0; s<listsp-1; s++)
                base+=liststack[s].totalindent;
        i1=base+liststack[listsp-1].indent1;
        i2=i1+liststack[listsp-1].indent2;
        fprintf(ofp, "\\par\\pard\\sb100\\tx%d\\li%d\\fi-%d\n", i2, i2, i2-i1);
}

static void PopList() {
        listsp--;
        if(listsp==0) {
                fprintf(ofp, "\\par\\pard\\sb100\n");
        } else {
                //recalculate indents
                int base=0;
                int i1,i2;
                for(int s=0; s<listsp-1; s++)
                        base+=liststack[s].totalindent;
                i1=base+liststack[listsp-1].indent1;
                i2=i1+liststack[listsp-1].indent2;
                fprintf(ofp, "\\par\\pard\\sb100\\tx%d\\li%d\\fi-%d\n", i2, i2, i2-i1);
        }
}

static void NewListItem() {
        liststack[listsp-1].count++;
        if(liststack[listsp-1].count==1) {
                //first item
                //nothing
        } else
                fprintf(ofp, "\\par\n");
}
static void GotoIndent1() {
        //fprintf(ofp, "\\tab\n");
}
static void GotoIndent2() {
        fprintf(ofp, "\\tab\n");
}
static int ListItemCount() {
        return liststack[listsp-1].count;
}
static listtype TopListType() {
        return liststack[listsp-1].type;
}
static void OutputNewParagraphInsideList() {
        fprintf(ofp, "\\par\n\\tab\n");
}
static int insideList() {
        return listsp!=0;
}
static int OrderedListNesting() {
        int nesting=0;
        for(int s=0; s<listsp; s++) {
                if(liststack[s].type==ordered_list)
                        nesting++;
        }
        return nesting;
}

//
// The RTF format
//

#define DEFAULTFONT_POINTS 10
void OutputFileStart(const char *filename)
{
        ofp=fopen(filename,"w");
        if(!ofp)
                error("Could not create '%s'",filename);
        char drive[_MAX_DRIVE],dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];
        _splitpath(filename,drive,dir,name,ext);
        char tocname[_MAX_PATH];
        _makepath(tocname,drive,dir,name,".toc");
        toc = fopen(tocname,"wt");
        if(!toc)
                error("Could not create '%s'",tocname);
        char biname[_MAX_PATH];
        _makepath(biname,drive,dir,name,".bi");
        bi = fopen(biname,"wt");
        if(!toc)
                error("Could not create '%s'",biname);

        // Start of RTF file
        fputs( "{\\rtf1\\ansi\n", ofp );

        // Font table
        fputs( "{\\fonttbl\n",               ofp );
        fputs( " \\f0\\fswiss Arial;\n",     ofp );
        fputs( " \\f1\\fmodern Courier;\n",  ofp );
        fputs( " \\f2\\ftech Symbol;}\n",    ofp );
        //default font is 0
        fputs( "\\deff0\n",                  ofp );

        // blue color
        fputs( "{\\colortbl;\\red0\\green0\\blue196;}\n", ofp );

        // Select default font
        fprintf(ofp, "\\fs%d\n", DEFAULTFONT_POINTS*2 );

        // Standard spaces before paragraph
        fputs( "\\sb100\n", ofp );
}

void OutputFileEnd()
{
        //close RTF block
        fputs( "}", ofp );
        fclose(ofp);
        fclose(toc);
        fclose(bi);
}

void CancelOutput(const char *filename) {
        //close file and unlink it before exiting due to an error
        fclose(ofp);
        remove(filename);
        fclose(toc);
        fclose(bi);
}


void dir_todo(int argc, char **argv) {
        //argv[1] : todo-string
        fprintf(ofp, "{\\b\\fs30 todo: %s}\n",argv[0]);
}

//--caution---
void dir_caution(int argc, char **argv) {
        fprintf(ofp, "{\\b{Caution: }}");
}
void dir_ecaution(int argc, char **argv) {
        //nothing
}

//---definition list---
void dir_dl(int argc, char **argv) {
        //argv[0] : term-heading
        //argv[1] : description-heading
        if(argc==0) {
                //nothing
                PushList(definition_list,DLIST_INDENT1,DLIST_INDENT2);
        } else if(argc==2) {
                PushList(definition_list,DLIST_INDENT1,DLIST_INDENT2);
                NewListItem();
                GotoIndent1();
                OutputString(argv[0]);
                GotoIndent2();
                OutputString(argv[1]);
        } else if(argc==3) {
                int chars=atoi(argv[2]);
                int twips = chars*DEFAULTFONT_POINTS*20;
                //characters are not as wide as they are tall
                twips = twips*3/5;
                PushList(definition_list,DLIST_INDENT1,twips);
                if(argv[0][0]) {
                        NewListItem();
                        GotoIndent1();
                        OutputString(argv[0]);
                        GotoIndent2();
                        OutputString(argv[1]);
                }
        } else
                error("Definition-list takes 0, 2 or 3 arguments, not %d",argc);

        skipWhiteAndNL();
}
void dir_edl(int argc, char **argv) {
        PopList();
}
void dir_dt(int argc, char **argv) {
        NewListItem();
        GotoIndent1();
        skipWhiteAndNL();
}
void dir_dd(int argc, char **argv) {
        GotoIndent2();
        skipWhiteAndNL();
}

//---footnote---
void dir_footnote(int argc, char **argv) {
        //argv[1] : footnote-ID
        fprintf(ofp, "#{\\footnote %s}\n",argv[0]);
        insideTopic=1;
}
void dir_efootnote(int argc, char **argv) {
        fprintf(ofp, "\\page");
        insideTopic=0;
}

//---topic---
void dir_topic(int argc, char **argv) {
        //argv[0] : topic-ID
        //argv[1] : level
        //argv[2] : title
        //(argv[3]: full name)
        enum { upafter, down, same, downonce} level;
        if(stricmp(argv[1],"upafter")==0) level=upafter;
        else if(stricmp(argv[1],"down")==0) level=down;
        else if(stricmp(argv[1],"same")==0) level=same;
        else if(stricmp(argv[1],"downonce")==0) level=downonce;
        else error("<level> must be 'upafter', 'down', 'same' or 'downonce'");
        if(level==down || level==downonce) currentLevel++;

        fprintf(ofp, "#{\\footnote %s}\n", argv[0]);
        fprintf(ofp, "${\\footnote %s}\n", argc>=4?argv[3]:argv[2]);
        fprintf(ofp, "+{\\footnote defbrowse}\n");
        if(autoIndex) {
                //generate automatic keywords
                dir_key(1,argv+2);
                if(argc>=4)
                        dir_key(1,argv+3);
        }
        fprintf(ofp, "{\\fs26\\cf1\\b %s}\\par\n", argv[2]);
        fprintf(toc, "%d %s %s\n",currentLevel,argv[0],argv[2]);
        skipWhiteAndNL();
        if(level==upafter || level==downonce) currentLevel--;
        insideTopic=1;
}
void dir_etopic(int argc, char **argv) {
        fprintf(ofp, "\\page\n\n");
        insideTopic=0;
}
void dir_key(int argc, char **argv) {
        //argv[0] : keyword
        if(argv[0][0]=='k' || argv[0][0]=='K')
                fprintf(ofp, "K{\\footnote  %s}\n",argv[0]);
        else
                fprintf(ofp, "K{\\footnote %s}\n",argv[0]);
        skipWhiteAndNL();
}

//--reference---
void dir_ref(int argc, char **argv) {
        //argv[0] : ID
        //argv[1] : link-text
        fprintf(ofp, "{\\uldb %s}{\\v %s}",argv[1],argv[0]);
}

void dir_reffootnote(int argc, char **argv) {
        //argv[0] : ID
        //argv[1] : link-text
        fprintf(ofp, "{\\ul %s}{\\v %s}",argv[1],argv[0]);
}

//--highlight---
void dir_hp(int argc, char **argv) {
        //argv[0] : highlight level
        highlightLevel=atoi(argv[0]);
        if(highlightLevel<0 || highlightLevel>9)
                error("highlight-level must be between 1 and 9");
        switch(highlightLevel) {
                case 1: //italic
                        fprintf(ofp, "{\\i{");
                        break;
                case 2: //bold
                        fprintf(ofp, "{\\b{");
                        break;
                default: //bold&italic
                        fprintf(ofp, "{\\i\\b{");
        }
}
void dir_ehp(int argc, char **argv) {
        fprintf(ofp, "}}");
}

//---note---
void dir_note(int argc, char **argv) {
        fprintf(ofp, "{\\b{Note:}} ");
        skipWhiteAndNL();
}
void dir_enote(int argc, char **argv) {
        //nothing
}

//--ordered list---
void dir_ol(int argc, char **argv) {
        PushList(ordered_list,OLIST_INDENT1,OLIST_INDENT2);
        skipWhiteAndNL();
}
void dir_eol(int argc, char **argv) {
        PopList();
}

void dir_li(int argc, char **argv) {
        if(TopListType()==ordered_list) {
                NewListItem();
                GotoIndent1();
                if(OrderedListNesting()%2==1) {
                        //odd levels are numbered with normal numbers
                        fprintf(ofp, "%d.", ListItemCount());
                } else {
                         //even levels are numbered with letters
                         fprintf(ofp, "%c.", 'a'-1+ListItemCount());
                }
                GotoIndent2();
        } else if(TopListType()==unordered_list) {
                NewListItem();
                GotoIndent1();
                fprintf(ofp, "{\\f2\\'B7}");
                GotoIndent2();
        } else if(TopListType()==simple_list) {
                NewListItem();
                GotoIndent1();
                GotoIndent2();
        } else {
                error("list item but list is not ordered/unordered/simple");
        }
        skipWhiteAndNL();
                
}

//--parameter list---
void dir_parml(int argc, char **argv) {
        //(argv[0] : tsize)
        fprintf(ofp, "{\\b{Parameters:}}\n");
        if(argc==0) {
                PushList(parameter_list, PLIST_INDENT1,PLIST_INDENT2);
        } else {
                int chars=atoi(argv[0]);
                int twips = chars*DEFAULTFONT_POINTS*20;
                //characters are not as wide as they are tall
                twips = twips*3/5;
                PushList(definition_list,PLIST_INDENT1,twips);
        }
        skipWhiteAndNL();
}
void dir_eparml(int argc, char **argv) {
        PopList();
}
void dir_pt(int argc, char **argv) {
        NewListItem();
        GotoIndent1();
        skipWhiteAndNL();
}
void dir_pd(int argc, char **argv) {
        GotoIndent2();
        skipWhiteAndNL();
}

//---simple list---
void dir_sl(int argc, char **argv) {
        PushList(simple_list,SLIST_INDENT1,SLIST_INDENT2);
        skipWhiteAndNL();
}
void dir_esl(int argc, char **argv) {
        PopList();
}

//--unordered list---
void dir_ul(int argc, char **argv) {
        PushList(unordered_list,ULIST_INDENT1,ULIST_INDENT2);
        skipWhiteAndNL();
}
void dir_eul(int argc, char **argv) {
        PopList();
}

//---code---
void dir_code(int argc, char **argv) {
        fprintf(ofp, "\\line{\\f1\n");
        inCode=1;
        skipWhiteAndNL();
}
void dir_ecode(int argc, char **argv) {
        fprintf(ofp, "}\n");
        inCode=0;
        skipWhiteAndNL();
        OutputNewParagraph();
}


//--graphic---
void dir_graphic(int argc, char **argv) {
        //argv[0]:  type 'bitmap', 'multiresolutionbitmap', 'hypergraphic'
        //argv[1]:  title
        //argv[2]:  type+filename
        //...
        if(stricmp(argv[0],"bitmap")==0 &&
           stricmp(argv[0],"multiresolutionbitmap")==0 &&
           stricmp(argv[0],"hypergraphic")==0)
                error("Invalid graphic type: '%s'",argv[0]);

        const char *winbmp=0;
        const char *ascii=0;
        for(int a=2; a<argc; a++) {
                if(strnicmp(argv[a],"winbmp:",7)==0)
                        winbmp=argv[a]+7;
                else if(strnicmp(argv[a],"ascii:",6)==0)
                        ascii=argv[a]+6;
        }

        char path[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], name[_MAX_FNAME], ext[_MAX_EXT];
        if(stricmp(argv[0],"bitmap")==0) {
                if(winbmp) {
                        _splitpath(winbmp,drive,dir,name,ext);
                        _makepath(path,drive,dir,name,".bmp");
                        fprintf(ofp, "\\{bml %s.bmp\\}",name);
                        fprintf(bi, "%s\n", path);
                } else if(ascii) {
                        fprintf(ofp, "{\\f1\\keep\n");
                        FILE *afp=fopen(ascii,"r");
                        if(afp) {
                                char l[256];
                                while(fgets(l,256,afp)) {
                                        for(int i=0; l[i] && l[i]!='\n';i++)
                                                OutputChar(l[i]);
                                        fprintf(ofp, "\\line\n");
                                }
                                fclose(afp);
                        } else {
                                warning("Could not open ascii drawing: '%s'\n",ascii);
                                fprintf(ofp, "Ascii drawing not available\line");
                        }
                        fprintf(ofp, "}\n");
                } else {
                        warning("No suitable graphic is available for IPF\n");
                        fprintf(ofp, ":p. Picture not available in IPF version.\n");
                }
        } else if(stricmp(argv[0],"multiresolutionbitmap")==0) {
                if(winbmp) {
                        _splitpath(winbmp,drive,dir,name,ext);
                        _makepath(path,drive,dir,name,".mrb");
                        fprintf(ofp, "\\{bml %s.mrb\\}",name);
                        fprintf(bi, "%s\n", path);
                } else {
                        warning("No suitable graphic is available for RTF\n");
                        fprintf(ofp, "(Picture not available in RTF version.)\n");
                }
        } else if(stricmp(argv[0],"hypergraphic")==0) {
                if(winbmp) {
                        _splitpath(winbmp,drive,dir,name,ext);
                        _makepath(path,drive,dir,name,".shg");
                        fprintf(ofp, "\\{bml %s.shg\\}",name);
                        fprintf(bi, "%s\n", path);
                } else {
                        warning("No suitable hypergraphic is available for RTF\n");
                        fprintf(ofp, "(Hypergraphic not available in RTF version.)\n");
                }
        }
        if(argv[1][0]) {
                fprintf(ofp, "\\par{\\fs16{");
                OutputString(argv[1]);
                fprintf(ofp, "}}\\par\n");
        }
}


void OutputNewParagraph()
{
        if(!insideTopic) return;
        if(insideList())
                OutputNewParagraphInsideList();
        else
                fputs("\\par\n\n", ofp );
}

static void OutputString(const char *s) {
        while(*s) OutputChar(*s++);
}

void OutputChar(char c)
{
        if(!insideTopic) return; //everything outside topics are ignored

        if( c == '\n' ) {
                if(inCode) {
                        //automatic forced newlines in codeblocks
                        fputs("\\line\n",ofp);
                } else {
                        //output a space before the newline, otherwise
                        //hc31 will join words
                        fputs(" \n",ofp);
                }
                return;
        }

        if( c == '\\' || c == '}' || c == '{' ) {
                //these three characters are special to RTF
                fputc('\\', ofp );
                fputc(c, ofp );
                return;
        }


        if( c & 0x80 ) {        // Test high bit
                //In general RTF can't handle high-bit characters
                //the BIH files are written in codepage 850 so we have to
                //translate from codepage 850 to Windows codepage
                switch(c) {
                        case '‘': c='æ'; break;
                        case '’': c='Æ'; break;
                        case '›': c='ø'; break;
                        case '': c='Ø'; break;
                        case '†': c='å'; break;
                        case '': c='Å'; break;
                }
                fprintf( ofp, "\\'%02X", (int)c );
        } else
                fputc( c, ofp );
}

