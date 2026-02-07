#include <node_api.h>
#include "allocator.h"
#include "arena.h"
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
//wrapper for r_free
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
//wrapper for r_defrag
napi_value DefragWrapper(napi_env env, napi_callback_info info){
    r_defrag();
    return NULL;
}
//wrapper for init_arena
napi_value ArenaInitWrapper(napi_env env, napi_callback_info info) {
    init_arenas();
    return NULL;
}
//wrapper for r_arena
napi_value ArenaAllocWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    uint32_t size, lifetime;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    napi_get_value_uint32(env, args[0], &size);
    napi_get_value_uint32(env, args[1], &lifetime);

    void* ptr = r_arena(size, (lifetime_t)lifetime);
    
    napi_value output;
    napi_create_bigint_uint64(env, (uint64_t)ptr, &output);
    return output;
}
//wrapper for r_reset
napi_value ArenaResetWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    uint32_t lifetime;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    napi_get_value_uint32(env, args[0], &lifetime);

    r_reset((lifetime_t)lifetime);
    return NULL;
}

// initialization of function calls
napi_value Init(napi_env env, napi_value exports) {
    napi_value fn_init, fn_alloc, fn_free, fn_defrag, 
    fn_arena_init, fn_arena_alloc, fn_arena_reset;

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
    //export arena_init
    napi_create_function(env, NULL, 0, ArenaInitWrapper, NULL, &fn_arena_init);
    napi_set_named_property(env, exports, "initArenas", fn_arena_init);
    //export arena_alloc
    napi_create_function(env, NULL, 0, ArenaAllocWrapper, NULL, &fn_arena_alloc);
    napi_set_named_property(env, exports, "arenaAlloc", fn_arena_alloc);
    //export arena_reset
    napi_create_function(env, NULL, 0, ArenaResetWrapper, NULL, &fn_arena_reset);
    napi_set_named_property(env, exports, "arenaReset", fn_arena_reset);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)