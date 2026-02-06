#include <node_api.h>
#include "allocator.h"
#include <stdbool.h>

// wrapper for init_heap
napi_value InitHeapWrapper(napi_env env, napi_callback_info info) {
    init_heap();
    return NULL; // Returns undefined to JS
}

// wrapper for r_alloc
napi_value AllocWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    uint32_t size_requested;

    // 1. get arg from js
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    // 2. convert js number -> c number
    napi_get_value_uint32(env, args[0], &size_requested);

    // 3. call the r_alloc c function
    void* resultPtr = r_alloc(size_requested);

    // 4. return as big int
    napi_value output;
    napi_create_bigint_uint64(env, (uint64_t)resultPtr, &output);
    
    return output;
}

napi_value FreeWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    uint64_t ptr_value;
    bool lossless; // N-API requires this variable even if we don't use it

    // Get the arguments
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    // covert JS BigInt to C uint64 (The memory address)
    napi_get_value_bigint_uint64(env, args[0], &ptr_value, &lossless);

    // cast it back to a void pointer
    void* ptr = (void*)ptr_value;

    // call r_free
    r_free(ptr);

    return NULL;
}

napi_value DefragWrapper(napi_env env, napi_callback_info info){
    r_defrag();
    return NULL;
}

// initialization of function calls
napi_value Init(napi_env env, napi_value exports) {
    napi_value fn_init, fn_alloc, fn_free, fn_defrag;

    // export init_heap
    napi_create_function(env, NULL, 0, InitHeapWrapper, NULL, &fn_init);
    napi_set_named_property(env, exports, "init", fn_init);

    // export r_alloc
    napi_create_function(env, NULL, 0, AllocWrapper, NULL, &fn_alloc);
    napi_set_named_property(env, exports, "alloc", fn_alloc);
    // export r_free
    napi_create_function(env, NULL, 0, FreeWrapper, NULL, &fn_free);
    napi_set_named_property(env, exports, "free", fn_free);
    //export r_defrag
    napi_create_function(env, NULL, 0, DefragWrapper, NULL, &fn_defrag);
    napi_set_named_property(env, exports, "defrag", fn_defrag);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)