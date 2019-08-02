#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.c"
#include "vcs.c"
#include <time.h>
#include "queue.c"

#define IMPLIED		0
#define ACCUMULATOR	1
#define IMMEDIATE	2

#define ZERO_PAGE	3
#define ZERO_PAGE_X	4
#define ZERO_PAGE_Y	5

#define ABSOLUTE	6
#define ABSOLUTE_X	7
#define ABSOLUTE_Y	8

#define ABS_INDIRECT 9
#define INDIRECT_X	10
#define INDIRECT_Y	11

#define RELATIVE	12

#define ASS_CODE	13

/* Marked bits */

#define REFERENCED 1
#define VALID_ENTRY 2
#define DATA 4
#define GFX  8
#define REACHABLE 16

#define BYTE 		unsigned char
#define ADDRESS 	unsigned int

extern int clength[];

struct resource {
	ADDRESS start;
	ADDRESS load;
	unsigned int length;
	ADDRESS end;
	int disp_data;
} app_data;

BYTE *mem = NULL;				/* Memory */
BYTE labels[4096];
BYTE reserved[64];
BYTE ioresrvd[24];
char orgmnc[16],linebuff[80],nextline[80];
FILE *cfg;

unsigned int pc,pcbeg,pcend,offset,rti_adr,start_adr,k;
int aflag,cflag,dflag,fflag,pflag,rflag,sflag,intflag,lineno,charcnt;

struct qnode *addressq;

/* Prototypes */

void disasm(unsigned int,int);
int check_bit(BYTE, int);
unsigned int read_adr(void);
int load_config(char *);
void showgfx(unsigned char);
void check_range(unsigned int, unsigned int);
int mark(unsigned int,int);
unsigned int filesize(FILE *stream);
int file_load(char[]);

void main(int argc,char *argv[])
{
    int c,i,j;
    char file[50],config[50], parms[132];
    char oflag;
    time_t currtime;

	mem=(BYTE *)malloc(4096);
	memset(mem,0,4096);
	app_data.start=0x0;
    app_data.load=0x0000;
	app_data.length=0;
    app_data.end=0x0FFF;
	app_data.disp_data=0;
    addressq = NULL;
    intflag = 0;
    
    strcpy(file,"");
    aflag = 1;
    cflag = 0;
    fflag = 0;
    pflag = 0;
    sflag = 0;
    rflag = 0;
    dflag = 1;
    strcpy(orgmnc,"   ORG ");
    strcpy(parms,"");
    for (i=0;i<argc;i++) {
        strcat(parms,argv[i]);
        strcat(parms," ");
    }
    while (--argc > 1 && (*++argv)[0] == '-')
        while (c = *++argv[0])
            switch(c) {
            case 'a':
                aflag = 0;
                break;
            case 'c':
                cflag = 1;
                i=0;
                while (*++argv[0] != '\0')
                    config[i++] = *argv[0];
                config[i]=*argv[0]--;
                fprintf(stderr,"Using %s config file\n",config);
                break;
            case 'd':
                dflag = 0;
                break;
            case 'o':
                oflag = *++argv[0];
                switch (oflag) {
                case '1':
                    strcpy(orgmnc,"   ORG ");
                    break;
                case '2':
                    strcpy(orgmnc,"   *=");
                    break;
                case '3':
                    strcpy(orgmnc,"   .OR ");
                    break;
                default:
                    fprintf(stderr,"Illegal org type %c\n",oflag);
                    break;
                }
                break;
            case 'p':
                pflag = 1;
                break;
            case 's':
                sflag = 1;
                break;
            case 'i':
                intflag = 1;
                break;
            case 'r':
                rflag = 1;
                break;
            case 'f':
                fflag = 1;
                break;
            default:
                fprintf(stderr,"DiStella: illegal option %c\n",c);
                exit(1);
            }
    strcpy(file,*++argv);
                    
    if (argc != 1) {
        fprintf(stderr,"DiStella v2.10 - February 25, 1997\n");
        fprintf(stderr,"\nUse: DiStella [options] file\n");
        fprintf(stderr," options:\n");
        fprintf(stderr,"   -a  Turns 'A' off in accumulator instructions\n");
        fprintf(stderr,"   -c  Defines optional config file to use.  (e.g. -cpacman.cfg)\n");
        fprintf(stderr,"   -d  Disables automatic code determination\n");
        fprintf(stderr,"   -f  Forces correct address length\n");
        fprintf(stderr,"   -i  Process Interrupt Handler\n");
        fprintf(stderr,"   -o# ORG variation: # = 1- ORG $XXXX  2- *=$XXXX  3- .OR $XXXX\n");
        fprintf(stderr,"   -p  Insert psuedo-mnemonic 'processor 6502'\n");
        fprintf(stderr,"   -r  Relocate calls out of address range\n");
        fprintf(stderr,"   -s  Cycle count\n");
        fprintf(stderr,"\n Config file:\n");
        fprintf(stderr,"   ORG XXXX - Start of disassembly\n");
        fprintf(stderr,"   DATA XXXX XXXX - Defines data range\n");
        fprintf(stderr,"   GFX XXXX XXXX - Defines graphics range - overrides DATA definition\n");
        fprintf(stderr,"   CODE XXXX XXXX - Defines code range - overrides DATA and GFX\n");
        fprintf(stderr,"\n Example: DiStella -pafs pacman.bin > pacman.s\n");
        fprintf(stderr,"\n Email: rcolbert@oasis.novia.net or dan.boris@coat.com\n");
        exit(0);
    }

    if (!file_load(file)) {
        fprintf(stderr,"Unable to load %s\n",file);
        exit(0);
    }
    
    pc=app_data.end-3;
    start_adr=read_adr();
    if (app_data.end == 0x7ff)
        offset=(start_adr & 0xf800);
    else
        offset=(start_adr - (start_adr % 0x1000));

    if (cflag && !load_config(config)) {
        fprintf(stderr,"Unable to load config file %s\n",config);
        exit(0);
    }

    fprintf(stderr,"PASS 1\n");

    addressq=addq(addressq,start_adr);
    rti_adr=read_adr();
    if (intflag) {
        addressq=addq(addressq,rti_adr);
        mark(rti_adr,REFERENCED);
    }
    if (dflag) {
      while(addressq != NULL) {
          pc=addressq->address;
          pcbeg=pc;
          addressq=delq(addressq);
          disasm(pc,1);
          for (k=pcbeg;k<=pcend;k++)
                  mark(k,REACHABLE);
      }
    
      for (k=0;k<=app_data.end;k=k+1) {
        if (!check_bit(labels[k],REACHABLE))
            mark(k+offset,DATA);
      }
    }

    fprintf(stderr,"PASS 2\n");
    disasm(offset,2);

    time(&currtime);
    printf("; Disassembly of %s\n",file);
    printf("; Disassembled %s",ctime(&currtime));
    printf("; Using DiStella v2.0\n;\n");
    printf("; Command Line: %s\n;\n",parms);
    if (cflag) {
        printf("; %s contents:\n;\n",config);
        while (fgets(parms,79,cfg) != NULL)
            printf(";      %s",parms);
    }
    printf("\n");
    if (pflag)
        printf("      processor 6502\n");

    for (i=0;i<=0x3d;i++)
        if (reserved[i] == 1) {
            printf("%s",stella[i]);
            for(j=strlen(stella[i]);j<7;j++)
                printf(" ");
            printf(" =  $%0.2X\n",i);
        }

    for (i=0x280;i<=0x297;i++)
        if (ioresrvd[i-0x280] == 1) {
            printf("%s",ioregs[i-0x280]);
            for(j=strlen(ioregs[i-0x280]);j<7;j++)
                printf(" ");
            printf(" =  $%0.4X\n",i);
        }
            
    for (i=0;i<0x1000;i++)
        if ((labels[i] & 3) == 1) {
            printf("L%0.4X   =   ",i+offset);
            printf("$%0.4X\n",i+offset);
        }

    printf("\n");
    printf("    %s",orgmnc);
    printf("$%0.4X\n",offset);

    fprintf(stderr,"PASS 3\n");
    strcpy(linebuff,"");
    strcpy(nextline,"");
    disasm(offset,3);
	free(mem);
}

unsigned int filesize(FILE *stream)
{
   unsigned int curpos, length;

   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);
   return length;
}

unsigned int read_adr()
{
	BYTE d1,d2;

	d1=mem[pc++];
	d2=mem[pc++];
	return (unsigned int) ((d2 << 8)+d1);
}

int file_load(char file[50])
{
	FILE *fn;

    fn=fopen(file,"rb");
	if (fn == NULL) return 0;
	if (app_data.length == 0) {
		app_data.length = filesize(fn);
    }
    if (app_data.length == 2048)
        app_data.end = 0x7ff;
    else if (app_data.length == 4096)
        app_data.end = 0xfff;
    else {
        printf("Error: .bin file must be 2048 or 4096 bytes\n");
        exit(1);
    }
	rewind(fn);
	fread(&mem[app_data.load],1,app_data.length,fn);
	fclose(fn);
	if (app_data.start == 0)
		app_data.start = app_data.load;

	return 1;
}

int load_config(char *file)
{
    char cfg_line[80];
    char cfg_tok[80];
    unsigned int cfg_beg, cfg_end;

    lineno=0;

    if ((cfg=fopen(file,"r")) == NULL)
        return 0;

    cfg_beg=cfg_end=0;

    while (fgets(cfg_line,79,cfg)!=NULL) {
        strcpy(cfg_tok,"");
        sscanf(cfg_line,"%s %x %x",cfg_tok,&cfg_beg,&cfg_end);
        if (!strcmp(cfg_tok,"DATA")) {
            check_range(cfg_beg,cfg_end);
            for(;cfg_beg<=cfg_end;) {
                mark(cfg_beg,DATA);
                if (cfg_beg == cfg_end)
                    cfg_end = 0;
                else
                    cfg_beg++;
            }
        } else if (!strcmp(cfg_tok,"GFX")) {
            check_range(cfg_beg,cfg_end);
            for(;cfg_beg<=cfg_end;) {
                mark(cfg_beg,GFX);
                if (cfg_beg == cfg_end)
                    cfg_end = 0;
                else
                    cfg_beg++;
            }
        } else if (!strcmp(cfg_tok,"ORG")) {
            offset = cfg_beg;
        } else if (!strcmp(cfg_tok,"CODE")) {
            check_range(cfg_beg,cfg_end);
            for(;cfg_beg<=cfg_end;) {
                mark(cfg_beg,REACHABLE);
                if (cfg_beg == cfg_end)
                    cfg_end = 0;
                else
                    cfg_beg++;
            }
        } else {
            fprintf(stderr,"Invalid line in config file - line ignored\n",lineno);
        }
    }
    rewind(cfg);
    return 1;
}

void check_range(unsigned int beg, unsigned int end)
{
    lineno++;
    if (beg > end) {
        fprintf(stderr,"Beginning of range greater than End in config file in line %d\n",lineno);
        exit(1);
    }

    if (beg > app_data.end + offset) {
        fprintf(stderr,"Beginning of range out of range in line %d\n",lineno);
        exit(1);
    }

    if (beg < offset) {
        fprintf(stderr,"Beginning of range out of range in line %d\n",lineno);
        exit(1);
    }
}

void disasm(unsigned int distart,int pass)
{
    BYTE op;
    BYTE d1,opsrc;
	unsigned int ad;
	short amode;
    int i,bytes,labfound,addbranch;

/*    pc=app_data.start; */
    pc=distart-offset;
	while(pc <= app_data.end) {
        if(pass == 3) {
          if (pc+offset == start_adr)
            printf("\nSTART:\n");
          if ((pc+offset == rti_adr) && (intflag))
            printf("\nINTERRUPT:\n");
        }
        if(check_bit(labels[pc],GFX)) {
/*         && !check_bit(labels[pc],REACHABLE)) { */
            if (pass == 2)
                mark(pc+offset,VALID_ENTRY);
            if (pass == 3) {
                if (check_bit(labels[pc],REFERENCED))
                    printf("L%0.4X: ",pc+offset);
                else
                    printf("       ",pc+offset);
                printf(".byte $%0.2X ; ",mem[pc]);
                showgfx(mem[pc]);
                printf(" $%0.4X\n",pc+offset);
            }
            pc++;
        } else
        if (check_bit(labels[pc],DATA) && !check_bit(labels[pc],GFX)) {
/*            && !check_bit(labels[pc],REACHABLE)) {  */
            mark(pc+offset,VALID_ENTRY);
            if (pass == 3) {
                bytes = 1;
                printf("L%0.4X: .byte ",pc+offset);
                printf("$%0.2X",mem[pc]);
            }
            pc++;

            while (check_bit(labels[pc],DATA) && !check_bit(labels[pc],REFERENCED)
                   && !check_bit(labels[pc],GFX) && pass == 3 && pc <= app_data.end) {
                if (pass == 3) {
                    bytes++;
                    if (bytes == 17) {
                        printf("\n       .byte $%0.2X",mem[pc]);
                        bytes = 1;
                    } else
                        printf(",$%0.2X",mem[pc]);
                }
                pc++;
            }
            if (pass == 3)
                printf("\n");
        } else {
            op=mem[pc];
            /* version 2.1 bug fix */
            if (pass == 2)
                mark(pc+offset,VALID_ENTRY);
            if (pass == 3)
                if (check_bit(labels[pc],REFERENCED)) {
                    printf("L%0.4X: ",pc+offset);
                } else
                    printf("       ");

            amode=lookup[op].addr_mode;
            if (app_data.disp_data) {
                for (i=0; i<clength[amode]; i++) {
                    if (pass == 3)
                        printf("%02X ",mem[pc+i]);
                }
                if (pass == 3)
                    printf("  ");
            }

            pc++;

            if (lookup[op].mnemonic[0] == '.') {
                amode = IMPLIED;
                if (pass == 3) {
                    sprintf(linebuff,".byte $%0.2X ;",op);
                    strcat(nextline,linebuff);
                }
            }

            if (pass == 1) {
                opsrc = lookup[op].source;
                if ((opsrc == M_REL) || (opsrc == M_ADDR) || (opsrc == M_AIND)) {
                    addbranch = 1;
                }
                else
                    addbranch = 0;
            } else if (pass == 3) {
                   sprintf(linebuff,"%s",lookup[op].mnemonic);
                   strcat(nextline,linebuff);
            }

            /* Version 2.1 added the extensions to mnemonics */
            switch(amode) {
/*              case IMPLIED: {
                    if (op == 0x40 || op == 0x60)
                            if (pass == 3) {
                                sprintf(linebuff,"\n");
                                strcat(nextline,linebuff);
                            }
                            break;
                }
*/
                case ACCUMULATOR: {
                     if (pass == 3)
                         if (aflag) {
                             sprintf(linebuff,"    A");
                             strcat(nextline,linebuff);
                         }
                     break;
                }
                case ABSOLUTE: {
                    ad=read_adr();
                    labfound = mark(ad,REFERENCED);
                    if (pass == 1) {
                        if ((addbranch) && !check_bit(labels[ad & app_data.end],REACHABLE)) {
                            if (ad > 0xfff)
                                 addressq=addq(addressq,(ad & app_data.end)+offset);
                            mark(ad,REACHABLE);

                        }
                    } else if (pass == 3) {
                        if (ad < 0x100 && fflag) {
                            sprintf(linebuff,".w  ");
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"    ");
                            strcat(nextline,linebuff);
                        }
                        if (labfound == 1) {
                            sprintf(linebuff,"L%0.4X",ad);
                            strcat(nextline,linebuff);
                        }
                        else if (labfound == 3) {
                            sprintf(linebuff,"%s",ioregs[ad-0x280]);
                            strcat(nextline,linebuff);
                        }
                        else if ((labfound == 4) && rflag) {
                            sprintf(linebuff,"L%0.4X",(ad & app_data.end)+offset);
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"$%0.4X",ad);
                            strcat(nextline,linebuff);
                        }
                    }
                    break;
                }
                case ZERO_PAGE: {
                    d1=mem[pc++];
                    labfound = mark(d1,REFERENCED);
                        if (pass == 3)
                        if (labfound == 2) {
                             sprintf(linebuff,"    %s",stella[d1]);
                             strcat(nextline,linebuff);
                        } else {
                             sprintf(linebuff,"    $%0.2X ",d1);
                             strcat(nextline,linebuff);
                        }
                    break;
                }
                case IMMEDIATE: {
                    d1=mem[pc++];
                    if (pass == 3) {
                        sprintf(linebuff,"    #$%0.2X ",d1);
                        strcat(nextline,linebuff);
                    }
                    break;
                }
                case ABSOLUTE_X: {
                    ad=read_adr();
                    labfound = mark(ad,REFERENCED);
                    if (pass == 3) {
                        if (ad < 0x100 && fflag) {
                            sprintf(linebuff,".wx ");
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"    ");
                            strcat(nextline,linebuff);
                        }
                        if (labfound == 1) {
                            sprintf(linebuff,"L%0.4X,X",ad);
                            strcat(nextline,linebuff);
                        }
                        else if (labfound == 3) {
                            sprintf(linebuff,"%s",ioregs[ad-0x280]);
                            strcat(nextline,linebuff);
                        }
                        else if ((labfound == 4) && rflag) {
                            sprintf(linebuff,"L%0.4X,X",(ad & app_data.end)+offset);
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"$%0.4X,X",ad);
                            strcat(nextline,linebuff);
                        }
                    }
                    break;
                }
                case ABSOLUTE_Y: {
                    ad=read_adr();
                    labfound = mark(ad,REFERENCED);
                    if (pass == 3) {
                        if (ad < 0x100 && fflag) {
                            sprintf(linebuff,".wy ");
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"    ");
                            strcat(nextline,linebuff);
                        }
                        if (labfound == 1) {
                            sprintf(linebuff,"L%0.4X,Y",ad);
                            strcat(nextline,linebuff);
                        }
                        else if (labfound == 3) {
                            sprintf(linebuff,"%s",ioregs[ad-0x280]);
                            strcat(nextline,linebuff);
                        }
                        else if ((labfound == 4) && rflag) {
                            sprintf(linebuff,"L%0.4X,Y",(ad & app_data.end)+offset);
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"$%0.4X,Y",ad);
                            strcat(nextline,linebuff);
                        }
                    }
                    break;
                }
                case INDIRECT_X: {
                    d1=mem[pc++];
                    if (pass == 3) {
                        sprintf(linebuff,"    ($%0.2X,X)",d1);
                        strcat(nextline,linebuff);
                    }
                    break;
                }
                case INDIRECT_Y: {
                    d1=mem[pc++];
                    if (pass == 3) {
                        sprintf(linebuff,"    ($%0.2X),Y",d1);
                        strcat(nextline,linebuff);
                    }
                    break;
                }
                case ZERO_PAGE_X: {
                    d1=mem[pc++];
                    labfound = mark(d1,REFERENCED);
                    if (pass == 3)
                        if (labfound == 2) {
                            sprintf(linebuff,"    %s,X",stella[d1]);
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"    $%0.2X,X",d1);
                            strcat(nextline,linebuff);
                        }
                    break;
                }
                case ZERO_PAGE_Y: {
                    d1=mem[pc++];
                    labfound = mark(d1,REFERENCED);
                    if (pass == 3)
                        if (labfound == 2) {
                            sprintf(linebuff,"    %s,Y",stella[d1]);
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"    $%0.2X,Y",d1);
                            strcat(nextline,linebuff);
                        }
                    break;
                }
                case RELATIVE: {
                    d1=mem[pc++];
                    ad=d1;
                    if (d1 >= 128) ad=d1-256;
                    labfound = mark(pc+ad+offset,REFERENCED);

                    if (pass == 1) {
                        if ((addbranch) && !check_bit(labels[pc+ad],REACHABLE)) {
                            addressq=addq(addressq,pc+ad+offset);
                            mark(pc+ad+offset,REACHABLE);
                     /*       addressq=addq(addressq,pc+offset); */
                        }
                    } else if (pass == 3)
                        if (labfound == 1) {
                            sprintf(linebuff,"    L%0.4X",pc+ad+offset);
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"    $%0.4X",pc+ad+offset);
                            strcat(nextline,linebuff);
                        }

                    break;
                }
                case ABS_INDIRECT: {
                    ad=read_adr();
                    labfound = mark(ad,REFERENCED);
                    if (pass == 3)
                        if (ad < 0x100 && fflag) {
                            sprintf(linebuff,".ind ");
                            strcat(nextline,linebuff);
                        }
                        if (labfound == 1) {
                            sprintf(linebuff,"(L%04X)",ad);
                            strcat(nextline,linebuff);
                        }
                        else if (labfound == 3) {
                            sprintf(linebuff,"(%s)",ioregs[ad]);
                            strcat(nextline,linebuff);
                        }
                        else {
                            sprintf(linebuff,"($%04X)",ad);
                            strcat(nextline,linebuff);
                        }
                    break;
                }
            }
            if (pass == 1) {
                if (!strcmp(lookup[op].mnemonic,"RTS") ||
                    !strcmp(lookup[op].mnemonic,"JMP") ||
/*                    !strcmp(lookup[op].mnemonic,"BRK") || */
                    !strcmp(lookup[op].mnemonic,"RTI")) {
                        pcend = (pc-1) + offset;
                        return;
                    }
            } else if (pass == 3) {
                printf("%s",nextline);
                for (charcnt=0;charcnt<15-strlen(nextline);charcnt++)
                    printf(" ");
                if (sflag)
                    printf(";%d",lookup[op].cycles);
                printf("\n");
                if (op == 0x40 || op == 0x60)
                    printf("\n");
                strcpy(nextline,"");
            }
        }
    }    
}

int mark(unsigned int address,int bit)
{

    if (address >= offset && address <=app_data.end + offset) {
        labels[address-offset] = labels[address-offset] | bit;
        return 1;
    } else if (address >= 0 && address <=0x3d) {
        reserved[address] = 1;
        return 2;
    } else if (address >= 0x280 && address <=0x297) {
        ioresrvd[address-0x280] = 1;
        return 3;
    } else if (address >= 0x1000) {
        labels[address & app_data.end] = labels[address & app_data.end] | bit;
        return 4;
    } else
        return 0;
}

int check_bit(BYTE bitflags, int i)
{
    int j;

    bitflags = bitflags & i;
    j = (int) bitflags;
    return j;
}

void showgfx(unsigned char c)
{
	int i;

    printf("|");
    for(i=0;i<8;i++) {
        if (c > 127)
            printf("X");
        else
            printf(" ");
        c = c << 1;
    }
    printf("|");
}
