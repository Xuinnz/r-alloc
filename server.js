const myAllocator = require('./build/Release/my_allocator');
const { performance } = require('perf_hooks');

// CONFIGURATION
const TOTAL_REQUESTS = 100000;  // Total traffic to simulate
const CONCURRENT_USERS = 100;   // Active sessions at any time
const SESSION_SIZE = 64;        // Size of a User Session object (Slab)
const REQUEST_TEMP_SIZE = 1024; // Temp data per request (Bump)

// LIFETIME ENUMS
const LIFETIME = {
    TRANSIENT: 0,    // Bump Pointer (Requests)
    INTERMEDIATE: 1, // Slab Allocator (Sessions)
    PERSISTENT: 2    // Malloc (Config)
};

// GLOBAL STATE
// This acts like a database or session store
const activeSessions = new Map(); // Map<UserID, Pointer>
let sessionArena = null; // One big arena for all sessions

// METRICS
let totalAllocatedBytes = 0;
let totalFreedBytes = 0;

// ==========================================
// 1. INITIALIZATION
// ==========================================
console.log("=== STARTING BACKEND SIMULATION ===");
console.log(`Target: ${TOTAL_REQUESTS} requests`);
console.log(`Mode:   Hybrid (Slab for Sessions, Bump for Requests)`);

try {
    myAllocator.init();
    // Create one large INTERMEDIATE arena to hold all user sessions (Slabs)
    // 1MB should be enough for our test
    sessionArena = myAllocator.createArena(1024 * 1024, LIFETIME.INTERMEDIATE);
    console.log("✅ Server Heap Initialized.");
    console.log("✅ Session Store (Slab Allocator) Ready.\n");
} catch (e) {
    console.error("❌ Initialization Failed:", e);
    process.exit(1);
}

// ==========================================
// 2. HELPER FUNCTIONS
// ==========================================

// Simulate User Login (Slab Alloc)
function createSession(userId) {
    if (activeSessions.has(userId)) return;

    // Allocate 64 bytes from the Slab Allocator
    // This is O(1) and reuses freed blocks!
    const ptr = myAllocator.rArena(sessionArena, SESSION_SIZE);
    
    if (ptr === 0n || ptr === 0) {
        throw new Error("Session Store OOM (Out of Memory)!");
    }
    
    activeSessions.set(userId, ptr);
}

// Simulate User Logout (Slab Free)
function destroySession(userId) {
    const ptr = activeSessions.get(userId);
    if (!ptr) return;

    // Return the block to the Slab Allocator (Recycle!)
    myAllocator.rArenaFree(sessionArena, ptr, SESSION_SIZE);
    
    activeSessions.delete(userId);
}

// Simulate an HTTP Request (Bump Alloc)
function handleRequest(userId) {
    // 1. Setup Request Scope (Transient Arena)
    // Every request gets a fresh scratchpad.
    const reqArena = myAllocator.createArena(4096, LIFETIME.TRANSIENT);
    
    // 2. Simulate Work (Allocating temp strings, JSON, etc.)
    // We allocate 10 small chunks
    for(let i=0; i<10; i++) {
        myAllocator.rArena(reqArena, REQUEST_TEMP_SIZE / 10);
    }

    // 3. Access Session Data (Read from Slab)
    // In C, we would read memory at activeSessions.get(userId)
    // Here we just verify the pointer exists
    const sessionPtr = activeSessions.get(userId);
    if (!sessionPtr) {
        // User not logged in, maybe create a temp guest session
    }

    // 4. Cleanup (Bulk Free)
    // Destroy the entire request scratchpad in O(1)
    myAllocator.rDestroy(reqArena);
}

// ==========================================
// 3. MAIN LOOP
// ==========================================
const start = performance.now();
let peakSessions = 0;

for (let i = 0; i < TOTAL_REQUESTS; i++) {
    // Randomly choose a user ID (0 to CONCURRENT_USERS)
    const userId = Math.floor(Math.random() * CONCURRENT_USERS);
    const action = Math.random();

    // 10% Chance: Login/Logout (Session Churn)
    if (action < 0.05) {
        // User logs in
        createSession(userId);
    } else if (action < 0.10) {
        // User logs out
        destroySession(userId);
    }

    // 90% Chance: Normal Request (View Page, API Call)
    // If logged in, handle request. If not, maybe just a landing page.
    handleRequest(userId);

    // Track Peak
    if (activeSessions.size > peakSessions) peakSessions = activeSessions.size;
    
    // Logging progress
    if (i % 10000 === 0) {
        const memMB = (process.memoryUsage().rss / 1024 / 1024).toFixed(2);
        process.stdout.write(`\rProgress: ${i}/${TOTAL_REQUESTS} | Active Sessions: ${activeSessions.size} | RSS: ${memMB} MB`);
    }
}

const end = performance.now();
console.log(`\n\n=== SIMULATION COMPLETE ===`);
console.log(`Time:        ${(end - start).toFixed(2)} ms`);
console.log(`Throughput:  ${(TOTAL_REQUESTS / ((end - start)/1000)).toFixed(0)} req/sec`);
console.log(`Peak Users:  ${peakSessions}`);
console.log(`Final Heap:  ${(process.memoryUsage().rss / 1024 / 1024).toFixed(2)} MB`);

// Cleanup Global Session Arena
myAllocator.rDestroy(sessionArena);
console.log("✅ Server Shutdown Cleanly.");