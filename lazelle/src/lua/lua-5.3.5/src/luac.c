/*
** $Id: luac.c,v 1.76 2018/06/19 01:32:02 lhf Exp $
** Lua compiler (saves bytecodes to files; also lists bytecodes)
** See Copyright Notice in lua.h
*/

#define luac_c
#define LUA_CORE

#include "lprefix.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "lua.h"
#include "lauxlib.h"

#include "lobject.h"
#include "lstate.h"
#include "lundump.h"

static void luasplit(const Proto* f);
static void PrintFunction(const Proto* f, int full);
#define luaU_print	PrintFunction

#define PROGNAME	"luac"		/* default program name */
#define OUTPUT		PROGNAME ".out"	/* default output file */

static int listing=0;			/* list bytecodes? */
static int dumping=1;			/* dump bytecodes? */
static int split=0;             /* dump bytecodes for cpu */
static int stripping=0;			/* strip debug information? */
static char Output[]={ OUTPUT };	/* default output file name */
static const char* output=Output;	/* actual output file name */
static const char* progname=PROGNAME;	/* actual program name */

static void fatal(const char* message)
{
 fprintf(stderr,"%s: %s\n",progname,message);
 exit(EXIT_FAILURE);
}

static void cannot(const char* what)
{
 fprintf(stderr,"%s: cannot %s %s: %s\n",progname,what,output,strerror(errno));
 exit(EXIT_FAILURE);
}

static void usage(const char* message)
{
 if (*message=='-')
  fprintf(stderr,"%s: unrecognized option '%s'\n",progname,message);
 else
  fprintf(stderr,"%s: %s\n",progname,message);
 fprintf(stderr,
  "usage: %s [options] [filenames]\n"
  "Available options are:\n"
  "  -l       list (use -l -l for full listing)\n"
  "  -o name  output to file 'name' (default is \"%s\")\n"
  "  -p       parse only\n"
  "  -s       strip debug information\n"
  "  -sb      dump bytecodes for cpu\n"
  "  -v       show version information\n"
  "  --       stop handling options\n"
  "  -        stop handling options and process stdin\n"
  ,progname,Output);
 exit(EXIT_FAILURE);
}

#define IS(s)	(strcmp(argv[i],s)==0)

static int doargs(int argc, char* argv[])
{
 int i;
 int version=0;
 if (argv[0]!=NULL && *argv[0]!=0) progname=argv[0];
 for (i=1; i<argc; i++)
 {
  if (*argv[i]!='-')			/* end of options; keep it */
   break;
  else if (IS("--"))			/* end of options; skip it */
  {
   ++i;
   if (version) ++version;
   break;
  }
  else if (IS("-"))			/* end of options; use stdin */
   break;
  else if (IS("-l"))			/* list */
   ++listing;
  else if (IS("-o"))			/* output file */
  {
   output=argv[++i];
   if (output==NULL || *output==0 || (*output=='-' && output[1]!=0))
    usage("'-o' needs argument");
   if (IS("-")) output=NULL;
  }
  else if (IS("-p"))			/* parse only */
   dumping=0;
  else if (IS("-sb"))            /* split only */
   split=1;
  else if (IS("-s"))			/* strip debug information */
   stripping=1;
  else if (IS("-v"))			/* show version */
   ++version;
  else					/* unknown option */
   usage(argv[i]);
 }
 if (i==argc && (listing || !dumping))
 {
  dumping=0;
  argv[--i]=Output;
 }
 if (version)
 {
  printf("%s\n",LUA_COPYRIGHT);
  if (version==argc-1) exit(EXIT_SUCCESS);
 }
 return i;
}

#define FUNCTION "(function()end)();"

static const char* reader(lua_State *L, void *ud, size_t *size)
{
 UNUSED(L);
 if ((*(int*)ud)--)
 {
  *size=sizeof(FUNCTION)-1;
  return FUNCTION;
 }
 else
 {
  *size=0;
  return NULL;
 }
}

#define toproto(L,i) getproto(L->top+(i))

static const Proto* combine(lua_State* L, int n)
{
 if (n==1)
  return toproto(L,-1);
 else
 {
  Proto* f;
  int i=n;
  if (lua_load(L,reader,&i,"=(" PROGNAME ")",NULL)!=LUA_OK) fatal(lua_tostring(L,-1));
  f=toproto(L,-1);
  for (i=0; i<n; i++)
  {
   f->p[i]=toproto(L,i-n-1);
   if (f->p[i]->sizeupvalues>0) f->p[i]->upvalues[0].instack=0;
  }
  f->sizelineinfo=0;
  return f;
 }
}

static int writer(lua_State* L, const void* p, size_t size, void* u)
{
 UNUSED(L);
 return (fwrite(p,size,1,(FILE*)u)!=1) && (size!=0);
}

static int pmain(lua_State* L)
{
 int argc=(int)lua_tointeger(L,1);
 char** argv=(char**)lua_touserdata(L,2);
 const Proto* f;
 int i;
 if (!lua_checkstack(L,argc)) fatal("too many input files");
 for (i=0; i<argc; i++)
 {
  const char* filename=IS("-") ? NULL : argv[i];
  if (luaL_loadfile(L,filename)!=LUA_OK) fatal(lua_tostring(L,-1));
 }
 f=combine(L,argc);
 if (listing) luaU_print(f,listing>1);
 if (dumping)
 {
  FILE* D= (output==NULL) ? stdout : fopen(output,"wb");
  if (D==NULL) cannot("open");
  lua_lock(L);
  luaU_dump(L,f,writer,D,stripping);
  lua_unlock(L);
  if (ferror(D)) cannot("write");
  if (fclose(D)) cannot("close");
 }
 if (split) {
    luasplit(f);
 }
 return 0;
}

int main(int argc, char* argv[])
{
 lua_State* L;
 int i=doargs(argc,argv);
 argc-=i; argv+=i;
 if (argc<=0) usage("no input files given");
 L=luaL_newstate();
 if (L==NULL) fatal("cannot create state: not enough memory");
 lua_pushcfunction(L,&pmain);
 lua_pushinteger(L,argc);
 lua_pushlightuserdata(L,argv);
 if (lua_pcall(L,2,0,0)!=LUA_OK) fatal(lua_tostring(L,-1));
 lua_close(L);
 return EXIT_SUCCESS;
}

/*
** $Id: luac.c,v 1.76 2018/06/19 01:32:02 lhf Exp $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <ctype.h>
#include <stdio.h>

#define luac_c
#define LUA_CORE

#include "ldebug.h"
#include "lobject.h"
#include "lopcodes.h"

#define VOID(p)		((const void*)(p))

static void PrintString(const TString* ts)
{
 const char* s=getstr(ts);
 size_t i,n=tsslen(ts);
 printf("%c",'"');
 for (i=0; i<n; i++)
 {
  int c=(int)(unsigned char)s[i];
  switch (c)
  {
   case '"':  printf("\\\""); break;
   case '\\': printf("\\\\"); break;
   case '\a': printf("\\a"); break;
   case '\b': printf("\\b"); break;
   case '\f': printf("\\f"); break;
   case '\n': printf("\\n"); break;
   case '\r': printf("\\r"); break;
   case '\t': printf("\\t"); break;
   case '\v': printf("\\v"); break;
   default:	if (isprint(c))
   			printf("%c",c);
		else
			printf("\\%03d",c);
  }
 }
 printf("%c",'"');
}

static void PrintConstant(const Proto* f, int i)
{
 const TValue* o=&f->k[i];
 switch (ttype(o))
 {
  case LUA_TNIL:
	printf("nil");
	break;
  case LUA_TBOOLEAN:
	printf(bvalue(o) ? "true" : "false");
	break;
  case LUA_TNUMFLT:
	{
	char buff[100];
	sprintf(buff,LUA_NUMBER_FMT,fltvalue(o));
	printf("%s",buff);
	if (buff[strspn(buff,"-0123456789")]=='\0') printf(".0");
	break;
	}
  case LUA_TNUMINT:
	printf(LUA_INTEGER_FMT,ivalue(o));
	break;
  case LUA_TSHRSTR: case LUA_TLNGSTR:
	PrintString(tsvalue(o));
	break;
  default:				/* cannot happen */
	printf("? type=%d",ttype(o));
	break;
 }
}

#define UPVALNAME(x) ((f->upvalues[x].name) ? getstr(f->upvalues[x].name) : "-")
#define MYK(x)		(-1-(x))

static void PrintCode(const Proto* f)
{
 const Instruction* code=f->code;
 int pc,n=f->sizecode;
 for (pc=0; pc<n; pc++)
 {
  Instruction i=code[pc];
  OpCode o=GET_OPCODE(i);
  int a=GETARG_A(i);
  int b=GETARG_B(i);
  int c=GETARG_C(i);
  int ax=GETARG_Ax(i);
  int bx=GETARG_Bx(i);
  int sbx=GETARG_sBx(i);
  int line=getfuncline(f,pc);
  printf("\t%d (0x%x)\t",pc+1, i);
  if (line>0) printf("[%d]\t",line); else printf("[-]\t");
  printf("%-9s\t",luaP_opnames[o]);
  switch (getOpMode(o))
  {
   case iABC:
    printf("%d",a);
    if (getBMode(o)!=OpArgN) printf(" %d",ISK(b) ? (MYK(INDEXK(b))) : b);
    if (getCMode(o)!=OpArgN) printf(" %d",ISK(c) ? (MYK(INDEXK(c))) : c);
    break;
   case iABx:
    printf("%d",a);
    if (getBMode(o)==OpArgK) printf(" %d",MYK(bx));
    if (getBMode(o)==OpArgU) printf(" %d",bx);
    break;
   case iAsBx:
    printf("%d %d",a,sbx);
    break;
   case iAx:
    printf("%d",MYK(ax));
    break;
  }
  switch (o)
  {
   case OP_LOADK:
    printf("\t; "); PrintConstant(f,bx);
    break;
   case OP_GETUPVAL:
   case OP_SETUPVAL:
    printf("\t; %s",UPVALNAME(b));
    break;
   case OP_GETTABUP:
    printf("\t; %s",UPVALNAME(b));
    if (ISK(c)) { printf(" "); PrintConstant(f,INDEXK(c)); }
    break;
   case OP_SETTABUP:
    printf("\t; %s",UPVALNAME(a));
    if (ISK(b)) { printf(" "); PrintConstant(f,INDEXK(b)); }
    if (ISK(c)) { printf(" "); PrintConstant(f,INDEXK(c)); }
    break;
   case OP_GETTABLE:
   case OP_SELF:
    if (ISK(c)) { printf("\t; "); PrintConstant(f,INDEXK(c)); }
    break;
   case OP_SETTABLE:
   case OP_ADD:
   case OP_SUB:
   case OP_MUL:
   case OP_MOD:
   case OP_POW:
   case OP_DIV:
   case OP_IDIV:
   case OP_BAND:
   case OP_BOR:
   case OP_BXOR:
   case OP_SHL:
   case OP_SHR:
   case OP_EQ:
   case OP_LT:
   case OP_LE:
    if (ISK(b) || ISK(c))
    {
     printf("\t; ");
     if (ISK(b)) PrintConstant(f,INDEXK(b)); else printf("-");
     printf(" ");
     if (ISK(c)) PrintConstant(f,INDEXK(c)); else printf("-");
    }
    break;
   case OP_JMP:
   case OP_FORLOOP:
   case OP_FORPREP:
   case OP_TFORLOOP:
    printf("\t; to %d",sbx+pc+2);
    break;
   case OP_CLOSURE:
    printf("\t; %p",VOID(f->p[bx]));
    break;
   case OP_SETLIST:
    if (c==0) printf("\t; %d",(int)code[++pc]); else printf("\t; %d",c);
    break;
   case OP_EXTRAARG:
    printf("\t; "); PrintConstant(f,ax);
    break;
   default:
    break;
  }
  printf("\n");
 }
}

#define SS(x)	((x==1)?"":"s")
#define S(x)	(int)(x),SS(x)

static void PrintHeader(const Proto* f)
{
 const char* s=f->source ? getstr(f->source) : "=?";
 if (*s=='@' || *s=='=')
  s++;
 else if (*s==LUA_SIGNATURE[0])
  s="(bstring)";
 else
  s="(string)";
 printf("\n%s <%s:%d,%d> (%d instruction%s at %p)\n",
 	(f->linedefined==0)?"main":"function",s,
	f->linedefined,f->lastlinedefined,
	S(f->sizecode),VOID(f));
 printf("%d%s param%s, %d slot%s, %d upvalue%s, ",
	(int)(f->numparams),f->is_vararg?"+":"",SS(f->numparams),
	S(f->maxstacksize),S(f->sizeupvalues));
 printf("%d local%s, %d constant%s, %d function%s\n",
	S(f->sizelocvars),S(f->sizek),S(f->sizep));
}

static void PrintDebug(const Proto* f)
{
 int i,n;
 n=f->sizek;
 printf("constants (%d) for %p:\n",n,VOID(f));
 for (i=0; i<n; i++)
 {
  printf("\t%d\t",i+1);
  PrintConstant(f,i);
  printf("\n");
 }
 n=f->sizelocvars;
 printf("locals (%d) for %p:\n",n,VOID(f));
 for (i=0; i<n; i++)
 {
  printf("\t%d\t%s\t%d\t%d\n",
  i,getstr(f->locvars[i].varname),f->locvars[i].startpc+1,f->locvars[i].endpc+1);
 }
 n=f->sizeupvalues;
 printf("upvalues (%d) for %p:\n",n,VOID(f));
 for (i=0; i<n; i++)
 {
  printf("\t%d\t%s\t%d\t%d\n",
  i,UPVALNAME(i),f->upvalues[i].instack,f->upvalues[i].idx);
 }
}

static void PrintFunction(const Proto* f, int full)
{
 int i,n=f->sizep;
 PrintHeader(f);
 PrintCode(f);
 if (full) PrintDebug(f);
 for (i=0; i<n; i++) PrintFunction(f->p[i],full);
}

uint32_t djbhash(const char *data, uint32_t length) {

    uint32_t hash = 5381;
    for(uint32_t x = 0; x < length; x++) {
        hash = ((hash << 5) + hash) + data[x];
    }

    return hash;
}

static uint32_t * splitfunction(const Proto *f, uint32_t *outsz) {
    // we want to gather the # pfunction instructions, upvals, 

    // allocate our storage buffer
    uint64_t bufidx = 0;
    uint32_t *buf = calloc(0x80000, 1);

    // copy in instructions
    buf[bufidx] = f->sizecode * 4;
    if ((bufidx << 2) + buf[bufidx] > 0x80000) exit(1);
    memcpy(&buf[bufidx+1], f->code, buf[bufidx]);
    bufidx = bufidx + 1 + f->sizecode;

    // copy in upvalues
    buf[bufidx++] = f->sizeupvalues * 4;
    if ((bufidx << 2) + buf[bufidx-1] > 0x80000) exit(1);
    for(int x = 0; x < f->sizeupvalues; x++) {
        uint32_t val = f->upvalues[x].instack;
        val += (f->upvalues[x].idx << 8);
        buf[bufidx++] = val;
    }

    // copy in constants
    buf[bufidx++] = f->sizek * 4;
    if ((bufidx << 2) + buf[bufidx-1] > 0x80000) exit(1);

    uint32_t *constbuf = calloc(0x10000, 1);
    uint32_t constbufidx = 0;
    for(int x = 0; x < f->sizek; x++) {

        if ((constbufidx << 2) > 0x10000) {
            printf("too many constants!\n");
            exit(1);
        }

        buf[bufidx++] = constbufidx;
        const TValue* o= &f->k[x];
        switch (ttype(o))
        {
            case LUA_TNIL:
                constbuf[constbufidx] = 4;
                constbuf[constbufidx+1] = 0;
                constbufidx += 2;
                break;
            case LUA_TBOOLEAN:
                constbuf[constbufidx] = 4;
                constbuf[constbufidx+1] = bvalue(o) ? 1 : 0;
                constbufidx += 2;
                break;
            case LUA_TNUMFLT:
                printf("float is not supported\n");
                exit(1);
            case LUA_TSHRSTR:
            case LUA_TLNGSTR:
                {
                    const char* s = svalue(o);
                    uint32_t length = vslen(o);
                    if ((constbufidx << 2) + 0x10 + length > 0x10000) exit(1);
                    memcpy(&constbuf[constbufidx + 3], s, length);
                    constbuf[constbufidx+2] = length;
                    constbuf[constbufidx+1] = djbhash(s, length);
                    constbuf[constbufidx] = 8 + length + 4 - (length % 4);
                    constbufidx = constbufidx + 1 + (constbuf[constbufidx] >> 2);
                }
                break;
            case LUA_TNUMINT:
                constbuf[constbufidx] = 4;
                constbuf[constbufidx+1] = ivalue(o);
                constbufidx += 2;
                break;
            default:
                printf("unknown type %d\n", ttype(o));
                exit(1);
                break;
        }
    }
    buf[bufidx++] = constbufidx * 4;
    printf("%d\n", constbufidx * 4);
    if ((bufidx << 2) + buf[bufidx-1] > 0x80000) exit(1);
    memcpy(&buf[bufidx], constbuf, constbufidx * 4);
    bufidx += constbufidx;
    free(constbuf);

    // copy in protos
    buf[bufidx++] = f->sizep;
    if ((bufidx << 2) + buf[bufidx-1] > 0x80000) exit(1);
    for(int x = 0; x < f->sizep; x++) {
        uint32_t innersz = 0;
        uint32_t *innerbuf = splitfunction(f->p[x], &innersz);
        buf[bufidx++] = innersz * 4;
        if ((bufidx << 2) + buf[bufidx-1] > 0x80000) exit(1);
        memcpy(&buf[bufidx], innerbuf, innersz * 4);
        bufidx += innersz;
        free(innerbuf);
    }

    // misc function attributes
    buf[bufidx++] = f->maxstacksize;
    buf[bufidx++] = f->sizeupvalues;

    if (outsz)
        *outsz = bufidx;
    return buf;
}

static void luasplit(const Proto* toplevel) {

    // get top level buf
    uint32_t bufsz = 0;
    uint32_t *buf = splitfunction(toplevel, &bufsz);

    int fd = open("/tmp/.dump.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (write(fd, buf, bufsz*4) != (bufsz*4)) {
        printf("failed to write out entire dump, somehow?\n");
        exit(1);
    }
    close(fd);
    exit(0);
}
