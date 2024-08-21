/* File generated automatically by the QuickJS compiler. */

#define _POSIX_C_SOURCE 200809L


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "libquickjs.h"

/*
const uint32_t hello_size = 87;

const uint8_t hello[87] = {
 0x01, 0x04, 0x0e, 0x63, 0x6f, 0x6e, 0x73, 0x6f,
 0x6c, 0x65, 0x06, 0x6c, 0x6f, 0x67, 0x16, 0x48,
 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72,
 0x6c, 0x64, 0x22, 0x65, 0x78, 0x61, 0x6d, 0x70,
 0x6c, 0x65, 0x73, 0x2f, 0x68, 0x65, 0x6c, 0x6c,
 0x6f, 0x2e, 0x6a, 0x73, 0x0d, 0x00, 0x02, 0x00,
 0x9e, 0x01, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00,
 0x14, 0x01, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x38,
 0xc4, 0x00, 0x00, 0x00, 0x42, 0xc5, 0x00, 0x00,
 0x00, 0x04, 0xc6, 0x00, 0x00, 0x00, 0x27, 0x01,
 0x00, 0xd2, 0x2b, 0x8e, 0x03, 0x01, 0x00,
};
*/

void qjs_add_global_func(JSContext *ctx, char* name, void* fptr, int num_args) {
  JSValue global_obj;
  global_obj = JS_GetGlobalObject(ctx);

  JS_SetPropertyStr(ctx, global_obj, name,
    JS_NewCFunction(ctx, fptr, name, num_args)
  );

  JS_FreeValue(ctx, global_obj);
}

int g_jscache_dev = -1;
uint8_t* g_response_buffer = NULL;

static void init_response() {
  g_response_buffer[0] = 0;
}

static JSValue check_response(JSContext *ctx) {
  //fprintf(stderr, "check_response %p %s\n", g_response_buffer, g_response_buffer);
  if (g_response_buffer[0] == 0) {
    fprintf(stderr, "Error: no response from kernel\n");
    printf("undefined\n");
    fflush(stdout);
    return JS_UNDEFINED;
  }

  printf("%s\n", g_response_buffer);
  fflush(stdout);
  char* v = strdup(g_response_buffer);
  // Return the response as a string
  return JS_NewString(ctx, v);
}

static JSValue  cache_init(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
  JSRuntime* rt = JS_GetRuntime(ctx);
  //fprintf(stderr, "cache_init\n");

  if (g_jscache_dev == -1) {
    g_jscache_dev = open("/dev/jscache", O_RDWR);
    if (g_jscache_dev < 0) {
      fprintf(stderr, "Error: could not open /dev/jscache\n");
      return JS_FALSE;
    }
  }

  if (g_response_buffer == NULL) {
    g_response_buffer = mmap((void*)0x7475616e0000, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    void* val = g_response_buffer;
    if (argc > 0) {
      // XXX
      val = argv[0].u.ptr;
    }
    //fprintf(stderr, "cache_init val: %p\n", val);
    ioctl(g_jscache_dev, 87, (void*)val);
  } else if (argc > 0) {
    // XXX
    rt = argv[0].u.ptr;
  }

  //fprintf(stderr, "cache_init rt: %p\n", rt);
  ioctl(g_jscache_dev, 88, (void*)rt);

  return JS_TRUE;
}

static JSValue cache_select(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
  if (argc < 1) {
    fprintf(stderr, "Error: cache_select requires 1 arguments\n");
    return JS_UNDEFINED;
  }

  init_response();

  unsigned long key = (unsigned long)JS_ToCString(ctx, argv[0]);
  //fprintf(stderr, "cache_insert fd: %d\n", g_jscache_dev);
  //fprintf(stderr, "cache_insert key: %p %s\n", key, key);
  key |= (strlen((char*)key) << (32+16));
  ioctl(g_jscache_dev, 55, (void*)key);

  return check_response(ctx);
  //return JS_UNDEFINED;
}

static JSValue kdump(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
  //fprintf(stderr, "kdump\n");
  ioctl(g_jscache_dev, 99, NULL);
  return JS_UNDEFINED;
}

static JSValue cache_insert(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
  if (argc < 2) {
    fprintf(stderr, "Error: cache_insert requires 2 arguments\n");
    return JS_UNDEFINED;
  }

  init_response();

  // Need to find the atom array to get the actual property value <:(

  unsigned long key = (unsigned long)JS_ToCString(ctx, argv[0]);

  //fprintf(stderr, "cache_insert fd: %d\n", g_jscache_dev);
  //fprintf(stderr, "cache_insert key: %p %s\n", key, key);
  key |= (strlen((char*)key) << (32+16));
  ioctl(g_jscache_dev, 33, (void*)key);

  JSValue obj = argv[1];
  unsigned long value = (unsigned long)&obj;

  ioctl(g_jscache_dev, 44, (void*)value);

  return check_response(ctx);
  //return JS_UNDEFINED;
}

JSValue qjs_sleep(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
  if (argc < 1) {
    fprintf(stderr, "Error: sleep requires 1 arguments\n");
    return JS_UNDEFINED;
  }

  unsigned long ms = (unsigned long)JS_ToCString(ctx, argv[0]);
  sleep(ms / 1000);
  return JS_UNDEFINED;
}

char flag[256] = {0};

char token_hex[128] = {0};

char* authenticity_token(char* data) {
  FILE* flag_p = fopen("/flag", "r");
  if (flag_p == NULL) {
    perror("fopen");
    exit(1);
  }
  fread(flag, 1, 256, flag_p);
  char* newline = strchr(flag, '\n');
  if (newline != NULL) {
    *newline = 0;
  }

  FILE* hash_data = fopen("/tmp/hash_data", "w");
  if (hash_data == NULL) {
    perror("fopen");
    exit(1);
  }
  fwrite(data, 1, strlen(data), hash_data);
  fwrite(flag, 1, strlen(flag), hash_data);
  fclose(hash_data);

  system("sha256sum /tmp/hash_data | cut -d ' ' -f 1 > /tmp/hash_hex");

  FILE* hash_hex = fopen("/tmp/hash_hex", "r");
  if (hash_hex == NULL) {
    perror("fopen");
    exit(1);
  }
  fread(token_hex, 1, 128, hash_hex);
  fclose(hash_hex);

  //fprintf(stderr, "authenticity_token: %s\n", token_hex);
  return token_hex;
}

static JSValue js_expand(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
  if (argc < 2) {
    fprintf(stderr, "Error: js_expand requires 2 arguments: <array> <length>\n");
    return JS_UNDEFINED;
  }
  // Input is a new length
  int32_t new_len;
  if (JS_ToInt32(ctx, &new_len, argv[1]))
      return JS_EXCEPTION;

  JSValue arr = argv[0];
  struct JSObject* obj = JS_VALUE_GET_OBJ(arr);

  uint64_t ptr = (uint64_t)obj;

  JSValue* prop_ptr = *(JSValue**)(ptr + 24 + 8);
  //fprintf(stderr, "prop_ptr: %p\n", prop_ptr);

  ptr += 48 + 8 + 8;
  //fprintf(stderr, "ptr: %p\n", ptr);
  uint64_t* len_ptr = (uint64_t*)ptr;
  //fprintf(stderr, "old_len: %d\n", *len_ptr);
  //fprintf(stderr, "new_len: %d\n", new_len);
  *len_ptr = new_len;

  //fprintf(stderr, "prop_ptr: %p\n", prop_ptr);
  //fprintf(stderr, "prop_ptr->u.int32: %d\n", prop_ptr->u.int32);
  prop_ptr->u.int32 = new_len;
  //fprintf(stderr, "prop_ptr->u.int32: %d\n", prop_ptr->u.int32);

  // Return new_len
  return JS_NewInt32(ctx, new_len);
}

static JSValue js_authenticity_token(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
  if (argc < 1) {
    fprintf(stderr, "Error: js_authenticity_token requires 1 arguments\n");
    return JS_UNDEFINED;
  }

  char* data = JS_ToCString(ctx, argv[0]);
  char* token = authenticity_token(data);
  return JS_NewString(ctx, token);
}

static JSValue js_dmesg(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
  system("dmesg");
  return JS_UNDEFINED;
}

static JSValue v_js_print(
  JSContext *ctx, JSValueConst this_val,
  int argc, JSValueConst *argv
) {
    int i;
    const char *str;

    for(i = 0; i < argc; i++) {
        if (i != 0)
            putchar(' ');
        str = JS_ToCString(ctx, argv[i]);
        if (!str)
            return JS_EXCEPTION;
        fputs(str, stdout);
        JS_FreeCString(ctx, str);
    }
    putchar('\n');
    return JS_UNDEFINED;
}

typedef struct engine {
  JSRuntime *rt;
  JSContext *ctx;
} eng;

eng* qjs_init(int argc, char **argv) {
  eng* e = calloc(1, sizeof(eng));
  e->rt = JS_NewRuntime();
  e->ctx = JS_NewContextRaw(e->rt);
  JS_AddIntrinsicBaseObjects(e->ctx);
  JS_AddIntrinsicEval(e->ctx);
  JS_AddIntrinsicJSON(e->ctx);
  JS_AddIntrinsicProxy(e->ctx);
  JS_AddIntrinsicTypedArrays(e->ctx);

  js_std_add_helpers(e->ctx, argc, argv);
  return e;
}

void qjs_run_until_done(eng* e) {
  js_std_loop(e->ctx);
  JS_FreeContext(e->ctx);
  JS_FreeRuntime(e->rt);
  free(e);
}

JSValue qjs_run_script(eng* e, char* script) {
    JSValue ret = JS_Eval(e->ctx, script, strlen(script), "<evalScript>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(ret)) {
        js_std_dump_error(e->ctx);
        exit(1);
    }
    // Print the ret
    char* str = JS_ToCString(e->ctx, ret);
    if (str) {
        printf("%s\n", str);
        fflush(stdout);
        JS_FreeCString(e->ctx, str);
    }
    JS_FreeValue(e->ctx, ret);

    return ret;
}

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int start_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    char* port = getenv("PORT");
    if (port == NULL) {
        port = "33";
    }
    serv_addr.sin_port = htons(atoi(port));

    // Set reuse address
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(sockfd, 5);

    while (1) {
        int in_fd = accept(sockfd, NULL, NULL);
        int child = fork();
        if (child == 0) {
            close(sockfd);
            return in_fd;
        } else {
            close(in_fd);
        }
    }
}

#include <sys/syscall.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


char* get_next_line(FILE* stream) {
    char* line = NULL;
    size_t max = 0x100;
    size_t num = getline(&line, &max, stream);
    if (num == -1) {
        return NULL;
    }
    // Find first \r\n
    char* end_of_line = strstr(line, "\r\n");
    if (end_of_line != NULL) {
        *end_of_line = '\0';
    }
    end_of_line = strstr(line, "\n");
    if (end_of_line != NULL) {
        *end_of_line = '\0';
    }
    return line;
}

void replace_stdio(int fd) {
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
}


int main(int argc, char **argv)
{
  fprintf(stdout, "KJSWorker Telnet Shell.\n");
  fflush(stdout);

  signal(SIGCHLD, SIG_IGN);
  g_jscache_dev = open("/dev/jscache", O_RDWR);
  eng* e = qjs_init(argc, argv);

  qjs_add_global_func(e->ctx, "jsCache", cache_insert, 2);
  qjs_add_global_func(e->ctx, "jsCacheInit", cache_init, 0);
  qjs_add_global_func(e->ctx, "jsCacheSelect", cache_select, 2);
  qjs_add_global_func(e->ctx, "kdump", kdump, 0);
  qjs_add_global_func(e->ctx, "sleep", qjs_sleep, 1);
  qjs_add_global_func(e->ctx, "token", js_authenticity_token, 1);
  qjs_add_global_func(e->ctx, "dmesg", js_dmesg, 0);
  qjs_add_global_func(e->ctx, "jsExpand", js_expand, 2);

  int fd = start_socket();
  replace_stdio(fd);

  //fprintf(stderr, "fd: %d\n", fd);

  FILE* f = fdopen(fd, "r+");
  while (1) {
    printf(stdout, "kworker> ");
    fflush(stdout);
    char* buf_ptr = get_next_line(f);
    if (buf_ptr == NULL) {
      break;
    }
    char* null_byte = strchr(buf_ptr, '\0');
    if (null_byte != NULL) {
      *null_byte = '\n';
    }
    if (strlen(buf_ptr) == 0) {
      break;
    }
    //fprintf(stderr, "buf_ptr: %s\n", buf_ptr);
    qjs_run_script(e, buf_ptr);
    sleep(1);
    free(buf_ptr);
  }


  

  //js_std_eval_binary(e->ctx, hello, hello_size, 0);
  //qjs_run_script(e, "jsCacheInit();jsCache('.hello.world',[1,{a:3},3]);");
  //qjs_run_script(e, "jsCacheInit();jsCache('.hello','sprint_symbol');");
  //qjs_run_script(e, "jsCacheInit();jsCache('.hello','adfsjblkasdkfbsdjkbfksjdbkfjbskdbjfsbdkfksbdkjf');");
  //sleep(1);
  //qjs_run_script(e, "kdump();jsCacheSelect('.hello[0]')");
  //qjs_run_script(e, "kdump();jsCache('.hello[0]',1234)");
  //qjs_run_script(e, "kdump();jsCacheSelect('.hello$symbol')");
  //qjs_run_script(e, "kdump();jsCacheSelect('.hello$authenticity')");
  //qjs_run_script(e, "kdump();jsCacheSelect('.hello')");
  //qjs_run_script(e, "jsCacheInit();jsCacheSelect('.hello');");
  //qjs_run_script(e, "jsCacheInit();jsCacheSelect('.hello');");

  qjs_run_until_done(e);

  e = NULL;
  return 0;
}
