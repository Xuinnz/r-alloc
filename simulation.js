const myAllocator = require('./build/Release/my_allocator');
const { performance } = require('perf_hooks');

// CONFIGURATION
const TOTAL_REQUESTS = 5000;   // How many "HTTP Requests" to simulate
const OPS_PER_REQUEST = 100;    // How many variables/objects per request
const DATA_SIZE = 64;           // Size of each object (bytes)

// Lifetime Enum
const LIFETIME = { TRANSIENT: 0, INTERMEDIATE: 1, PERSISTENT: 2 };

console.log(`\n=== BACKEND SIMULATION: ${TOTAL_REQUESTS} Requests, ${OPS_PER_REQUEST} allocs/req ===`);

// Initialize Heap
myAllocator.init();

// ==========================================
// TEST 1: Base Allocator (Malloc/Free)
// ==========================================
console.log("\n[1] Running Base Allocator Test (malloc/free)...");
let ptrs = new BigUint64Array(OPS_PER_REQUEST); // Store pointers to free them later

const startBase = performance.now();

for (let i = 0; i < TOTAL_REQUESTS; i++) {
    // 1. Simulate "Handling Request" -> Allocate memory
    for (let j = 0; j < OPS_PER_REQUEST; j++) {
        ptrs[j] = myAllocator.rAlloc(DATA_SIZE);
    }

    // 2. Simulate "Request Done" -> Free everything
    for (let j = 0; j < OPS_PER_REQUEST; j++) {
        myAllocator.rFree(ptrs[j]);
    }
}

const endBase = performance.now();
const timeBase = endBase - startBase;
console.log(`    -> Time: ${timeBase.toFixed(2)} ms`);
console.log(`    -> Throughput: ${(TOTAL_REQUESTS / (timeBase / 1000)).toFixed(0)} req/sec`);


// ==========================================
// TEST 2: Arena Factory (Create/Bump/Destroy)
// ==========================================
console.log("\n[2] Running Arena Factory Test (create/bump/destroy)...");

const startArena = performance.now();

for (let i = 0; i < TOTAL_REQUESTS; i++) {
    // 1. Create a "Scope" (Arena) for this request
    // Size needed: 100 items * 64 bytes = 6400 bytes. Let's give it 8KB (8192).
    const reqArena = myAllocator.createArena(8192, LIFETIME.TRANSIENT);

    // 2. Simulate "Handling Request" -> Bump Allocate
    for (let j = 0; j < OPS_PER_REQUEST; j++) {
        myAllocator.rArena(reqArena, DATA_SIZE); 
        // Notice: We don't need to save pointers because we don't free them individually!
    }

    // 3. Simulate "Request Done" -> Destroy the whole scope
    myAllocator.rDestroy(reqArena);
}

const endArena = performance.now();
const timeArena = endArena - startArena;
console.log(`    -> Time: ${timeArena.toFixed(2)} ms`);
console.log(`    -> Throughput: ${(TOTAL_REQUESTS / (timeArena / 1000)).toFixed(0)} req/sec`);


// ==========================================
// RESULTS
// ==========================================
console.log("\n=== FINAL VERDICT ===");
const improvement = (timeBase / timeArena).toFixed(2);
console.log(`Speedup: ${improvement}x FASTER`);

if (timeArena < timeBase) {
    console.log("Conclusion: Arena Strategy wins. O(1) bulk cleanup beats O(N) free.");
} else {
    console.log("Conclusion: Base Allocator wins (Unexpected). Check overhead.");
}