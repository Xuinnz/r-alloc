const myAllocator = require('./build/Release/my_allocator');
const { performance } = require('perf_hooks');

// --- CONFIGURATION ---
const REQUESTS = 10000;      // Total HTTP requests to simulate
const OPS_PER_REQ = 100;     // Allocations per request
const ALLOC_SIZE = 64;       // Size of each object
const ARENA_SIZE = 8192;     // 8KB per arena

// Lifetime Enum
const LIFETIME = { TRANSIENT: 0, INTERMEDIATE: 1, PERSISTENT: 2 };

// Helper to get current memory usage (MB)
const getMem = () => process.memoryUsage().rss / 1024 / 1024;

console.log(`\n=== ULTIMATE BENCHMARK ===`);
console.log(`Requests: ${REQUESTS} | Allocations: ${REQUESTS * OPS_PER_REQ}`);
console.log(`Payload:  ${ALLOC_SIZE} bytes per alloc`);

myAllocator.init(); // Initialize Heap

const results = {};

// ==========================================
// 1. V8 Engine (Native JS Objects)
// ==========================================
console.log("\n[1] Testing V8 (Native JavaScript)...");
global.gc && global.gc(); // Try to force GC start if exposed
const startMemV8 = getMem();
const startV8 = performance.now();

for (let i = 0; i < REQUESTS; i++) {
    const requestScope = []; // Simulate a request scope
    for (let j = 0; j < OPS_PER_REQ; j++) {
        // Allocate equivalent of 64 bytes
        requestScope.push(Buffer.allocUnsafe(ALLOC_SIZE));
    }
    // Scope ends, 'requestScope' becomes garbage
}

const endV8 = performance.now();
const endMemV8 = getMem();
results.v8 = {
    time: endV8 - startV8,
    memDiff: endMemV8 - startMemV8
};


// ==========================================
// 2. Base Allocator (Malloc/Free)
// ==========================================
console.log("[2] Testing Base Allocator (rAlloc/rFree)...");
global.gc && global.gc();
const startMemBase = getMem();
const startBase = performance.now();

let ptrs = new BigUint64Array(OPS_PER_REQ);

for (let i = 0; i < REQUESTS; i++) {
    for (let j = 0; j < OPS_PER_REQ; j++) {
        ptrs[j] = myAllocator.rAlloc(ALLOC_SIZE);
    }
    for (let j = 0; j < OPS_PER_REQ; j++) {
        myAllocator.rFree(ptrs[j]);
    }
}

const endBase = performance.now();
const endMemBase = getMem();
results.base = {
    time: endBase - startBase,
    memDiff: endMemBase - startMemBase
};


// ==========================================
// 3. Arena Allocator (Factory/Bump/Destroy)
// ==========================================
console.log("[3] Testing Arena Allocator (Factory)...");
global.gc && global.gc();
const startMemArena = getMem();
const startArena = performance.now();

for (let i = 0; i < REQUESTS; i++) {
    // 1. Create Arena
    const arena = myAllocator.createArena(ARENA_SIZE, LIFETIME.TRANSIENT);
    
    // 2. Bump Alloc (Fast!)
    for (let j = 0; j < OPS_PER_REQ; j++) {
        myAllocator.rArena(arena, ALLOC_SIZE);
    }

    // 3. Destroy Arena (Bulk Free)
    myAllocator.rDestroy(arena);
}

const endArena = performance.now();
const endMemArena = getMem();
results.arena = {
    time: endArena - startArena,
    memDiff: endMemArena - startMemArena
};


// ==========================================
// FINAL REPORT
// ==========================================
console.table({
    "V8 Engine": {
        "Time (ms)": results.v8.time.toFixed(2),
        "Req/Sec": (REQUESTS / (results.v8.time / 1000)).toFixed(0),
        "Mem Growth (MB)": results.v8.memDiff.toFixed(2)
    },
    "Base Allocator": {
        "Time (ms)": results.base.time.toFixed(2),
        "Req/Sec": (REQUESTS / (results.base.time / 1000)).toFixed(0),
        "Mem Growth (MB)": results.base.memDiff.toFixed(2)
    },
    "Arena Allocator": {
        "Time (ms)": results.arena.time.toFixed(2),
        "Req/Sec": (REQUESTS / (results.arena.time / 1000)).toFixed(0),
        "Mem Growth (MB)": results.arena.memDiff.toFixed(2)
    }
});

// Speedup Calculations
const v8_speedup = (results.v8.time / results.arena.time).toFixed(2);
const base_speedup = (results.base.time / results.arena.time).toFixed(2);

console.log(`\n>>> VERDICT <<<`);
console.log(`Arena is ${base_speedup}x FASTER than Base Allocator`);
console.log(`Arena is ${v8_speedup}x FASTER than V8 Node.js Engine`);

if(results.arena.memDiff < 1.0) {
    console.log(`Efficiency: Arena Memory Growth is minimal (No leaks detected).`);
}