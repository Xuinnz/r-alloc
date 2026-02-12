// Express Server Testing

const express = require('express');
const myAllocator = require('./build/Release/my_allocator');

const app = express();
const PORT = 3000;

// CONFIG
const ALLOCS_PER_REQ = 1000; // Simulate "heavy work" (1000 allocations per user)
const ITEM_SIZE = 64;        // 64 bytes per item
const LIFETIME_TRANSIENT = 0;

// Initialize your Heap (64MB)
try {
    myAllocator.init(); 
    console.log("✅ Custom Allocator Initialized (64MB Heap)");
} catch (e) {
    console.error("Failed to init allocator:", e);
    process.exit(1);
}

// =========================================================
// MIDDLEWARE: The "Arena Lifecycle" Manager
// =========================================================
const arenaMiddleware = (req, res, next) => {
    // 1. Create a fresh Arena for this request (Scratchpad)
    // We give it 128KB, plenty for our test
    req.arena = myAllocator.createArena(128 * 1024, LIFETIME_TRANSIENT);

    // 2. Hook into the 'finish' event to clean up
    res.on('finish', () => {
        // AUTOMATIC CLEANUP: The request is done, destroy the arena.
        // This is O(1) bulk free. No GC pause.
        myAllocator.rDestroy(req.arena);
    });

    next();
};

// =========================================================
// ROUTE 1: The "Arena" Endpoint (Your Allocator)
// =========================================================
app.get('/arena', arenaMiddleware, (req, res) => {
    // Simulate "Work": Allocating 1000 temporary items
    for (let i = 0; i < ALLOCS_PER_REQ; i++) {
        // We just ask for memory. We don't even need to save the pointer 
        // because the Arena destroys it all at the end anyway.
        myAllocator.rArena(req.arena, ITEM_SIZE);
    }

    res.send('Arena Work Done');
});

// =========================================================
// ROUTE 2: The "Native" Endpoint (Standard V8 GC)
// =========================================================
app.get('/native', (req, res) => {
    // Simulate "Work": Allocating 1000 temporary JS objects
    const tempArray = [];
    for (let i = 0; i < ALLOCS_PER_REQ; i++) {
        // Allocate a generic object (~64 bytes + V8 overhead)
        tempArray.push({ a: 1, b: 2, c: 3, d: i });
    }
    
    // When this function ends, 'tempArray' becomes garbage.
    // V8 will have to pause later to clean this up.
    res.send('Native Work Done');
});

app.listen(PORT, () => {
    console.log(`🚀 Hybrid Server running on http://localhost:${PORT}`);
    console.log(`   /arena  -> Uses Custom C Allocator`);
    console.log(`   /native -> Uses V8 Garbage Collector`);
});