// Arena in Backend Simulation
const myAllocator = require('./build/Release/my_allocator');
const { performance } = require('perf_hooks');

// CONFIG
const MAX_SESSIONS = 10000;
const TOTAL_OPS = 1000000;
const SESSION_SIZE_BYTES = 128; // Slab Size (e.g. 128 class)

const LIFETIME = { TRANSIENT: 0, INTERMEDIATE: 1, PERSISTENT: 2 };

// INIT
myAllocator.init();
const sessionArena = myAllocator.createArena(10 * 1024 * 1024, LIFETIME.INTERMEDIATE); // 10MB Slab
const persistentArena = myAllocator.createArena(5 * 1024 * 1024, LIFETIME.PERSISTENT);

// STATE
const sessions = new Map(); // Maps ID -> Pointer (BigInt)

console.log("=== ARENA (Custom) Simulation ===");
console.log("1. Filling Persistent Cache...");
for(let i=0; i<1000; i++) {
    myAllocator.rArena(persistentArena, 100); // Allocate 100 bytes persistent
}

console.log("2. Starting Traffic (Random Login/Logout/Request)...");

const start = performance.now();
let peakRSS = 0;

for (let i = 0; i < TOTAL_OPS; i++) {
    const r = Math.random();

    // 30% Chance: Login (Alloc from Slab)
    if (r < 0.3) {
        if (sessions.size < MAX_SESSIONS) {
            const id = i;
            // O(1) Allocation from Free List
            const ptr = myAllocator.rArena(sessionArena, SESSION_SIZE_BYTES);
            if (ptr) sessions.set(id, ptr);
        }
    } 
    // 30% Chance: Logout (Return to Slab -> FILL HOLE)
    else if (r < 0.6) {
        if (sessions.size > 0) {
            const keys = Array.from(sessions.keys());
            const randomKey = keys[Math.floor(Math.random() * keys.length)];
            const ptr = sessions.get(randomKey);
            
            // O(1) Free (Pushes back to stack)
            myAllocator.rArenaFree(sessionArena, ptr, SESSION_SIZE_BYTES);
            sessions.delete(randomKey);
        }
    }
    // 40% Chance: Request (Transient Bump)
    else {
        const reqArena = myAllocator.createArena(4096, LIFETIME.TRANSIENT);
        myAllocator.rArena(reqArena, 2048); // Do work
        myAllocator.rDestroy(reqArena); // O(1) Wipe
    }

    if (i % 5000 === 0) {
        const rss = process.memoryUsage().rss / 1024 / 1024;
        if (rss > peakRSS) peakRSS = rss;
        process.stdout.write(`\rOps: ${i} | Sessions: ${sessions.size} | RSS: ${rss.toFixed(2)} MB`);
    }
}

console.log(`\n\n=== RESULTS (ARENA) ===`);
console.log(`Final RSS: ${(process.memoryUsage().rss / 1024 / 1024).toFixed(2)} MB`);
console.log(`Peak RSS:  ${peakRSS.toFixed(2)} MB`);

// Cleanup
myAllocator.rDestroy(sessionArena);
myAllocator.rDestroy(persistentArena);