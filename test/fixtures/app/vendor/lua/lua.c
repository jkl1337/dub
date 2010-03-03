/*
** $Id: lua.c,v 1.160 2006/06/02 15:34:00 roberto Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lua_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"



static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;



static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}


static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}


static void print_usage (void) {
  fprintf(stderr,
  "usage: %s [options] [script [args]].\n"
  "Available options are:\n"
  "  -e stat  execute string " LUA_QL("stat") "\n"
  "  -l name  require library " LUA_QL("name") "\n"
  "  -i       enter interactive mode after executing " LUA_QL("script") "\n"
  "  -v       show version information\n"
  "  --       stop handling options\n"
  "  -        execute stdin and stop handling options\n"
  ,
  progname);
  fflush(stderr);
}


static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
  return status;
}


static int traceback (lua_State *L) {
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}


static void print_version (void) {
  l_message(NULL, LUA_RELEASE "  " LUA_COPYRIGHT);
}


static int getargs (lua_State *L, char **argv, int n) {
  int narg;
  int i;
  int argc = 0;
  while (argv[argc]) argc++;  /* count total number of arguments */
  narg = argc - (n + 1);  /* number of arguments to the script */
  luaL_checkstack(L, narg + 3, "too many arguments to script");
  for (i=n+1; i < argc; i++)
    lua_pushstring(L, argv[i]);
  lua_createtable(L, narg, n + 1);
  for (i=0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - n);
  }
  return narg;
}


static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report(L, status);
}


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report(L, status);
}


static int dolibrary (lua_State *L, const char *name) {
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return report(L, lua_pcall(L, 1, 0, 0));
}


static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getfield(L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  lua_pop(L, 1);  /* remove global */
  return p;
}


static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
    if (strstr(msg, LUA_QL("<eof>")) == tp) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}


static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  if (lua_readline(L, b, prmt) == 0)
    return 0;  /* no input */
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[l-1] = '\0';  /* remove it */
  if (firstline && b[0] == '=')  /* first line starts with `=' ? */
    lua_pushfstring(L, "return %s", b+1);  /* change it to `return' */
  else
    lua_pushstring(L, b);
  lua_freeline(L, b);
  return 1;
}


static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;  /* no input */
  for (;;) {  /* repeat until gets a complete line */
    status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_strlen(L, 1), "=stdin");
    if (!incomplete(L, status)) break;  /* cannot try to add lines? */
    if (!pushline(L, 0))  /* no more input? */
      return -1;
    lua_pushliteral(L, "\n");  /* add a new line... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
  }
  lua_saveline(L, 1);
  lua_remove(L, 1);  /* remove line */
  return status;
}


static void dotty (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = NULL;
  while ((status = loadline(L)) != -1) {
    if (status == 0) status = docall(L, 0, 0);
    report(L, status);
    if (status == 0 && lua_gettop(L) > 0) {  /* any result to print? */
      lua_getglobal(L, "print");
      lua_insert(L, 1);
      if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
        l_message(progname, lua_pushfstring(L,
                               "error calling " LUA_QL("print") " (%s)",
                               lua_tostring(L, -1)));
    }
  }
  lua_settop(L, 0);  /* clear stack */
  fputs("\n", stdout);
  fflush(stdout);
  progname = oldprogname;
}


static int handle_script (lua_State *L, char **argv, int n) {
  int status;
  const char *fname;
  int narg = getargs(L, argv, n);  /* collect arguments */
  lua_setglobal(L, "arg");
  fname = argv[n];
  if (strcmp(fname, "-") == 0 && strcmp(argv[n-1], "--") != 0)
    fname = NULL;  /* stdin */
  status = luaL_loadfile(L, fname);
  lua_insert(L, -(narg+1));
  if (status == 0)
    status = docall(L, narg, 0);
  else
    lua_pop(L, narg);
  return report(L, status);
}


/* check that argument has no extra characters at the end */
#define notail(x)	{if ((x)[2] != '\0') return -1;}


static int collectargs (char **argv, int *pi, int *pv, int *pe) {
  int i;
  for (i = 1; argv[i] != NULL; i++) {
    if (argv[i][0] != '-')  /* not an option? */
        return i;
    switch (argv[i][1]) {  /* option */
      case '-':
        notail(argv[i]);
        return (argv[i+1] != NULL ? i+1 : 0);
      case '\0':
        return i;
      case 'i':
        notail(argv[i]);
        *pi = 1;  /* go through */
      case 'v':
        notail(argv[i]);
        *pv = 1;
        break;
      case 'e':
        *pe = 1;  /* go through */
      case 'l':
        if (argv[i][2] == '\0') {
          i++;
          if (argv[i] == NULL) return -1;
        }
        break;
      default: return -1;  /* invalid option */
    }
  }
  return 0;
}


static int runargs (lua_State *L, char **argv, int n) {
  int i;
  for (i = 1; i < n; i++) {
    if (argv[i] == NULL) continue;
    lua_assert(argv[i][0] == '-');
    switch (argv[i][1]) {  /* option */
      case 'e': {
        const char *chunk = argv[i] + 2;
        if (*chunk == '\0') chunk = argv[++i];
        lua_assert(chunk != NULL);
        if (dostring(L, chunk, "=(command line)") != 0)
          return 1;
        break;
      }
      case 'l': {
        const char *filename = argv[i] + 2;
        if (*filename == '\0') filename = argv[++i];
        lua_assert(filename != NULL);
        if (dolibrary(L, filename))
          return 1;  /* stop if file fails */
        break;
      }
      default: break;
    }
  }
  return 0;
}


static int handle_luainit (lua_State *L) {
  const char *init = getenv(LUA_INIT);
  if (init == NULL) return 0;  /* status OK */
  else if (init[0] == '@')
    return dofile(L, init+1);
  else
    return dostring(L, init, "=" LUA_INIT);
}


struct Smain {
  int argc;
  char **argv;
  int status;
};


static int pmain (lua_State *L) {
  struct Smain *s = (struct Smain *)lua_touserdata(L, 1);
  char **argv = s->argv;
  int script;
  int has_i = 0, has_v = 0, has_e = 0;
  globalL = L;
  if (argv[0] && argv[0][0]) progname = argv[0];
  lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
  luaL_openlibs(L);  /* open libraries */
  lua_gc(L, LUA_GCRESTART, 0);
  s->status = handle_luainit(L);
  if (s->status != 0) return 0;
  script = collectargs(argv, &has_i, &has_v, &has_e);
  if (script < 0) {  /* invalid args? */
    print_usage();
    s->status = 1;
    return 0;
  }
  if (has_v) print_version();
  s->status = runargs(L, argv, (script > 0) ? script : s->argc);
  if (s->status != 0) return 0;
  if (script)
    s->status = handle_script(L, argv, script);
  if (s->status != 0) return 0;
  if (has_i)
    dotty(L);
  else if (script == 0 && !has_e && !has_v) {
    if (lua_stdin_is_tty()) {
      print_version();
      dotty(L);
    }
    else dofile(L, NULL);  /* executes stdin as a file */
  }
  return 0;
}
// ======================================================================================================================
// ======================================================================================================================

#include "matrix.h"

#include "lua_cpp_helper.h"


using namespace dub;


/* ============================ Constructors     ====================== */


/** dub::Matrix::Matrix()
 * app/include/matrix.h:14
 */
static int Matrix_Matrix1(lua_State *L) {
  Matrix * retval__    = new Matrix();
  lua_pushclass<Matrix>(L, retval__, "dub.Matrix");
  return 1;
}


/** dub::Matrix::Matrix(int rows, int cols)
 * app/include/matrix.h:16
 */
static int Matrix_Matrix2(lua_State *L) {
  int rows             = luaL_checkint   (L, 1);
  int cols             = luaL_checkint   (L, 2);
  Matrix * retval__    = new Matrix(rows, cols);
  lua_pushclass<Matrix>(L, retval__, "dub.Matrix");
  return 1;
}



/** Overloaded function chooser for Matrix(...) */
static int Matrix_Matrix(lua_State *L) {
  int type__ = lua_type(L, 1);
  if (type__ == LUA_TNUMBER) {
    return Matrix_Matrix2(L);
  } else if (type__ == LUA_TNONE) {
    return Matrix_Matrix1(L);
  } else {
    // use any to raise errors
    return Matrix_Matrix1(L);
  }
}

/* ============================ Destructor       ====================== */

static int Matrix_destructor(lua_State *L) {
  Matrix **userdata = (Matrix**)luaL_checkudata(L, 1, "dub.Matrix");
  if (*userdata) delete *userdata;
  *userdata = NULL;
  return 0;
}

/* ============================ tostring         ====================== */

static int Matrix__tostring(lua_State *L) {
  Matrix **userdata = (Matrix**)luaL_checkudata(L, 1, "dub.Matrix");
  lua_pushfstring(L, "dub.Matrix: %p", *userdata);
  return 1;
}

/* ============================ Member Methods   ====================== */


/** double dub::Matrix::cols()
 * app/include/matrix.h:29
 */
static int Matrix_cols(lua_State *L) {
  Matrix *self__       = *((Matrix**)luaL_checkudata(L, 1, "dub.Matrix"));
  lua_remove(L, 1);
  double retval__      = self__->cols();
  lua_pushnumber(L, retval__);
  return 1;
}


/** double dub::Matrix::rows()
 * app/include/matrix.h:33
 */
static int Matrix_rows(lua_State *L) {
  Matrix *self__       = *((Matrix**)luaL_checkudata(L, 1, "dub.Matrix"));
  lua_remove(L, 1);
  double retval__      = self__->rows();
  lua_pushnumber(L, retval__);
  return 1;
}


/** size_t dub::Matrix::size()
 * app/include/matrix.h:25
 */
static int Matrix_size(lua_State *L) {
  Matrix *self__       = *((Matrix**)luaL_checkudata(L, 1, "dub.Matrix"));
  lua_remove(L, 1);
  size_t retval__      = self__->size();
  lua_pushnumber(L, retval__);
  return 1;
}




/* ============================ Lua Registration ====================== */

static const struct luaL_Reg Matrix_member_methods[] = {
  {"cols"              , Matrix_cols},
  {"rows"              , Matrix_rows},
  {"size"              , Matrix_size},
  {"__tostring"        , Matrix__tostring},
  {"__gc"              , Matrix_destructor},
  {NULL, NULL},
};

static const struct luaL_Reg Matrix_namespace_methods[] = {
  {"Matrix"            , Matrix_Matrix},
  {NULL, NULL},
};

static void luaopen_dub_Matrix(lua_State *L) {
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "dub.Matrix"); // "dub.Matrix"

  // metatable.__index = metatable (find methods in the table itself)
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  // register member methods
  luaL_register(L, NULL, Matrix_member_methods);  // dub_Matrix_member_methods

  // register class methods in a global table like "dub.Matrix"
  luaL_register(L, "dub", Matrix_namespace_methods); // dub_Matrix_class_methods
}
#include "matrix.h"

#include "lua_cpp_helper.h"


using namespace dub;


/* ============================ Constructors     ====================== */


/** dub::FloatMat::FloatMat()
 * app/include/matrix.h:62
 */
static int FloatMat_FloatMat1(lua_State *L) {
  FloatMat * retval__  = new FloatMat();
  lua_pushclass<FloatMat>(L, retval__, "dub.FloatMat");
  return 1;
}


/** dub::FloatMat::FloatMat(int rows, int cols)
 * app/include/matrix.h:64
 */
static int FloatMat_FloatMat2(lua_State *L) {
  int rows             = luaL_checkint   (L, 1);
  int cols             = luaL_checkint   (L, 2);
  FloatMat * retval__  = new FloatMat(rows, cols);
  lua_pushclass<FloatMat>(L, retval__, "dub.FloatMat");
  return 1;
}



/** Overloaded function chooser for FloatMat(...) */
static int FloatMat_FloatMat(lua_State *L) {
  int type__ = lua_type(L, 1);
  if (type__ == LUA_TNUMBER) {
    return FloatMat_FloatMat2(L);
  } else if (type__ == LUA_TNONE) {
    return FloatMat_FloatMat1(L);
  } else {
    // use any to raise errors
    return FloatMat_FloatMat1(L);
  }
}

/* ============================ Destructor       ====================== */

static int FloatMat_destructor(lua_State *L) {
  FloatMat **userdata = (FloatMat**)luaL_checkudata(L, 1, "dub.FloatMat");
  if (*userdata) delete *userdata;
  *userdata = NULL;
  return 0;
}

/* ============================ tostring         ====================== */

static int FloatMat__tostring(lua_State *L) {
  FloatMat **userdata = (FloatMat**)luaL_checkudata(L, 1, "dub.FloatMat");
  lua_pushfstring(L, "dub.FloatMat: %p", *userdata);
  return 1;
}

/* ============================ Member Methods   ====================== */


/** size_t dub::FloatMat::cols()
 * app/include/matrix.h:77
 */
static int FloatMat_cols(lua_State *L) {
  FloatMat *self__     = *((FloatMat**)luaL_checkudata(L, 1, "dub.FloatMat"));
  lua_remove(L, 1);
  size_t retval__      = self__->cols();
  lua_pushnumber(L, retval__);
  return 1;
}


/** void dub::FloatMat::fill(T value)
 * app/include/matrix.h:85
 */
static int FloatMat_fill(lua_State *L) {
  FloatMat *self__     = *((FloatMat**)luaL_checkudata(L, 1, "dub.FloatMat"));
  lua_remove(L, 1);
  float value          = luaL_checknumber(L, 1);
  self__->fill(value);
  return 0;
}


/** T dub::FloatMat::get(size_t row, size_t col)
 * app/include/matrix.h:89
 */
static int FloatMat_get(lua_State *L) {
  FloatMat *self__     = *((FloatMat**)luaL_checkudata(L, 1, "dub.FloatMat"));
  lua_remove(L, 1);
  size_t row           = luaL_checknumber(L, 1);
  size_t col           = luaL_checknumber(L, 2);
  float retval__       = self__->get(row, col);
  lua_pushnumber(L, retval__);
  return 1;
}


/** size_t dub::FloatMat::rows()
 * app/include/matrix.h:81
 */
static int FloatMat_rows(lua_State *L) {
  FloatMat *self__     = *((FloatMat**)luaL_checkudata(L, 1, "dub.FloatMat"));
  lua_remove(L, 1);
  size_t retval__      = self__->rows();
  lua_pushnumber(L, retval__);
  return 1;
}


/** size_t dub::FloatMat::size()
 * app/include/matrix.h:73
 */
static int FloatMat_size(lua_State *L) {
  FloatMat *self__     = *((FloatMat**)luaL_checkudata(L, 1, "dub.FloatMat"));
  lua_remove(L, 1);
  size_t retval__      = self__->size();
  lua_pushnumber(L, retval__);
  return 1;
}




/* ============================ Lua Registration ====================== */

static const struct luaL_Reg FloatMat_member_methods[] = {
  {"cols"              , FloatMat_cols},
  {"fill"              , FloatMat_fill},
  {"get"               , FloatMat_get},
  {"rows"              , FloatMat_rows},
  {"size"              , FloatMat_size},
  {"__tostring"        , FloatMat__tostring},
  {"__gc"              , FloatMat_destructor},
  {NULL, NULL},
};

static const struct luaL_Reg FloatMat_namespace_methods[] = {
  {"FloatMat"          , FloatMat_FloatMat},
  {"FMatrix"           , FloatMat_FloatMat},
  {NULL, NULL},
};

static void luaopen_dub_FloatMat(lua_State *L) {
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "dub.FloatMat"); // "dub.Matrix"

  // metatable.__index = metatable (find methods in the table itself)
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  // register member methods
  luaL_register(L, NULL, FloatMat_member_methods);  // dub_Matrix_member_methods

  // register class methods in a global table like "dub.Matrix"
  luaL_register(L, "dub", FloatMat_namespace_methods); // dub_Matrix_class_methods
}


// ======================================================================================================================
// ======================================================================================================================
int main (int argc, char **argv) {
  int status;
  struct Smain s;
  lua_State *L = lua_open();  /* create state */
  if (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    return EXIT_FAILURE;
  }
  luaopen_dub_Matrix(L);
  luaopen_dub_FloatMat(L);
  s.argc = argc;
  s.argv = argv;
  status = lua_cpcall(L, &pmain, &s);
  report(L, status);
  lua_close(L);
  return (status || s.status) ? EXIT_FAILURE : EXIT_SUCCESS;
}

