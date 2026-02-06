#include <node_api.h>
#include "allocator.h"
#include <stdbool.h>

// Wrapper for init_heap
napi_value InitHeapWrapper(napi_env env, napi_callback_info info) {
    init_heap();
    return NULL; // Returns undefined to JS
}

// Wrapper for r_alloc
napi_value AllocWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    uint32_t size_requested;

    // 1. Get arguments from JS
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    // 2. Convert JS Number -> C Int
    napi_get_value_uint32(env, args[0], &size_requested);

    // 3. Call YOUR C function
    void* resultPtr = r_alloc(size_requested);

    // 4. Return the address as a BigInt (so JS can see the address)
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

    // Convert JS BigInt to C uint64 (The memory address)
    napi_get_value_bigint_uint64(env, args[0], &ptr_value, &lossless);

    // Cast it back to a void pointer
    void* ptr = (void*)ptr_value;

    // Call the engine
    r_free(ptr);

    return NULL;
}

// Module Initialization (Like 'module.exports')
napi_value Init(napi_env env, napi_value exports) {
    napi_value fn_init, fn_alloc, fn_free;

    // Export init_heap
    napi_create_function(env, NULL, 0, InitHeapWrapper, NULL, &fn_init);
    napi_set_named_property(env, exports, "init", fn_init);

    // Export r_alloc
    napi_create_function(env, NULL, 0, AllocWrapper, NULL, &fn_alloc);
    napi_set_named_property(env, exports, "alloc", fn_alloc);
    // Export r_free
    napi_create_function(env, NULL, 0, FreeWrapper, NULL, &fn_free);
    napi_set_named_property(env, exports, "free", fn_free);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)