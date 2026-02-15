#include <node_api.h>
#include "allocator.h"
#include "arena.h"
#include <stdbool.h>
#include <cstdio>

// wrapper for init_heap
napi_value InitHeapWrapper(napi_env env, napi_callback_info info) {
    init_heap();
    return NULL; // Returns undefined to JS
}

// wrapper for r_alloc
// JS Usage: rAlloc(size, site_id)
napi_value AllocWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 2; // We now expect 2 arguments
    napi_value args[2];
    uint32_t size_requested;
    uint32_t site_id = 0; // Default to 0 if not provided

    // 1. get arg from js
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    // 2. convert js number -> c number (Size)
    napi_get_value_uint32(env, args[0], &size_requested);

    // 3. Get Site ID (Argument 2)
    if (argc > 1) {
        napi_get_value_uint32(env, args[1], &site_id);
    }

    // 4. call the r_alloc function with site_id
    void* resultPtr = r_alloc(size_requested, site_id);

    // 5. return as big int
    napi_value output;
    napi_create_bigint_uint64(env, (uint64_t)resultPtr, &output);
    
    return output;
}

// wrapper for r_free
napi_value FreeWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    uint64_t ptr_value;
    bool lossless; 

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
    // r_defrag currently doesn't exist in the new C++ allocator.cpp 
    // unless you added it back. If it's missing, this is fine as a placeholder.
    // r_defrag(); 
    return NULL;
}

// Wrapper for create_arena
napi_value CreateArenaWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    uint32_t size, policy;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    napi_get_value_uint32(env, args[0], &size);
    napi_get_value_uint32(env, args[1], &policy);

    // Call YOUR function
    // Note: Cast policy to lifetime_t enum
    arena_t* arena = create_arena(size, (lifetime_t)policy);

    napi_value output;
    napi_create_bigint_uint64(env, (uint64_t)arena, &output);
    return output;
}

// Wrapper for r_arena
// JS Usage: rArena(arena_ptr, size, site_id)
napi_value ArenaAllocWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 3; // Now expects 3 arguments
    napi_value args[3];
    uint64_t arena_ptr_val;
    uint32_t size;
    uint32_t site_id = 0; // Default
    bool lossless;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    // 1. Get Arena
    napi_get_value_bigint_uint64(env, args[0], &arena_ptr_val, &lossless);
    arena_t* arena = (arena_t*)arena_ptr_val;

    // 2. Get Size
    napi_get_value_uint32(env, args[1], &size);

    // 3. Get Site ID
    if (argc > 2) {
        napi_get_value_uint32(env, args[2], &site_id);
    }

    // Call YOUR function (Ensure r_arena in arena.c/cpp accepts site_id!)
    // If r_arena doesn't take site_id yet, update arena.h to match r_alloc
    void* ptr = r_arena(arena, size, site_id);

    napi_value output;
    napi_create_bigint_uint64(env, (uint64_t)ptr, &output);
    return output;
}

// Wrapper for r_reset
napi_value ArenaResetWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    uint64_t arena_ptr_val;
    bool lossless;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    napi_get_value_bigint_uint64(env, args[0], &arena_ptr_val, &lossless);
    arena_t* arena = (arena_t*)arena_ptr_val;

    // Call YOUR function
    r_reset(arena);
    return NULL;
}

// Wrapper for r_destroy
napi_value DestroyArenaWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    uint64_t arena_ptr_val;
    bool lossless;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    napi_get_value_bigint_uint64(env, args[0], &arena_ptr_val, &lossless);
    arena_t* arena = (arena_t*)arena_ptr_val;

    // Call YOUR function
    r_destroy(arena);
    return NULL;
}

napi_value ArenaFreeWrapper(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    uint64_t arena_ptr_val;
    uint64_t item_ptr_val;
    uint32_t size;
    bool lossless;

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    // 1. Get Arena Handle
    napi_get_value_bigint_uint64(env, args[0], &arena_ptr_val, &lossless);
    arena_t* arena = (arena_t*)arena_ptr_val;

    // 2. Get Item Pointer to Free
    napi_get_value_bigint_uint64(env, args[1], &item_ptr_val, &lossless);
    void* ptr = (void*)item_ptr_val;

    // 3. Get Size (Crucial for Slab!)
    napi_get_value_uint32(env, args[2], &size);

    // Call the Free function
    r_arena_free(arena, ptr, size);

    return NULL;
}

// initialization of function calls
napi_value Init(napi_env env, napi_value exports) {
    napi_value fn_init, fn_alloc, fn_free, fn_defrag, 
    fn_arena_init, fn_arena_alloc, fn_arena_reset, fn_arena_destroy,
    fn_arena_free;

    // export init_heap
    napi_create_function(env, NULL, 0, InitHeapWrapper, NULL, &fn_init);
    napi_set_named_property(env, exports, "init", fn_init);

    // export r_alloc
    napi_create_function(env, NULL, 0, AllocWrapper, NULL, &fn_alloc);
    napi_set_named_property(env, exports, "rAlloc", fn_alloc);
    // export r_free
    napi_create_function(env, NULL, 0, FreeWrapper, NULL, &fn_free);
    napi_set_named_property(env, exports, "rFree", fn_free);
    //export r_defrag
    napi_create_function(env, NULL, 0, DefragWrapper, NULL, &fn_defrag);
    napi_set_named_property(env, exports, "rDefrag", fn_defrag);
    //export arena_init
    napi_create_function(env, NULL, 0, CreateArenaWrapper, NULL, &fn_arena_init);
    napi_set_named_property(env, exports, "createArena", fn_arena_init);
    //export arena_alloc
    napi_create_function(env, NULL, 0, ArenaAllocWrapper, NULL, &fn_arena_alloc);
    napi_set_named_property(env, exports, "rArena", fn_arena_alloc);
    //export arena_reset
    napi_create_function(env, NULL, 0, ArenaResetWrapper, NULL, &fn_arena_reset);
    napi_set_named_property(env, exports, "rReset", fn_arena_reset);

    napi_create_function(env, NULL, 0, DestroyArenaWrapper, NULL, &fn_arena_destroy);
    napi_set_named_property(env, exports, "rDestroy", fn_arena_destroy);

    napi_create_function(env, NULL, 0, ArenaFreeWrapper, NULL, &fn_arena_free);
    napi_set_named_property(env, exports, "rArenaFree", fn_arena_free);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)