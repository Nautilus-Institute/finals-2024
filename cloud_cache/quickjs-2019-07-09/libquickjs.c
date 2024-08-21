/* File generated automatically by the QuickJS compiler. */

#include "libquickjs.h"

//int main(int argc, char **argv)
EXPORT int qjs_run_bytecode(uint8_t* bytecode, size_t bytecode_size, int argc, char **argv)
{
  JSRuntime *rt;
  JSContext *ctx;
  rt = JS_NewRuntime();
  ctx = JS_NewContextRaw(rt);

  JS_AddIntrinsicBaseObjects(ctx);
  js_std_add_helpers(ctx, argc, argv);
  js_std_eval_binary(ctx, bytecode, bytecode_size, 0);
  js_std_loop(ctx);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
