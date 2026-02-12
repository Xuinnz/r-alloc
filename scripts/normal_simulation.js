// v8 in Backend Simulation
const { performance } = require('perf_hooks');

// CONFIG
const MAX_SESSIONS = 10000;
const TOTAL_OPS = 1000000;
const SESSION_SIZE_BYTES = 128; // Small enough to cause frag issues

// STATE
const sessions = new Map();
const persistentCache = [];

// 1. Setup Persistent State (Simulate specific DB Cache)
console.log("=== V8 (Standard) Simulation ===");
console.log("1. Filling Persistent Cache...");
for(let i=0; i<1000; i++) {
    persistentCache.push(new Array(100).fill("DATA")); // Long-lived junk
}

// 2. The Loop (Simulate Random Traffic)
console.log("2. Starting Traffic (Random Login/Logout/Request)...");

const start = performance.now();
let peakRSS = 0;

for (let i = 0; i < TOTAL_OPS; i++) {
    const r = Math.random();

    // 30% Chance: Login (Create Session)
    if (r < 0.3) {
        if (sessions.size < MAX_SESSIONS) {
            const id = i; // Unique ID
            // Allocate an object
            const session = { 
                id: id, 
                data: Buffer.alloc(SESSION_SIZE_BYTES), 
                loginTime: Date.now() 
            };
            sessions.set(id, session);
        }
    } 
    // 30% Chance: Logout (Destroy Session -> CREATE HOLE)
    else if (r < 0.6) {
        if (sessions.size > 0) {
            // Pick a random key to delete (Random Access is crucial for fragmentation)
            const keys = Array.from(sessions.keys());
            const randomKey = keys[Math.floor(Math.random() * keys.length)];
            sessions.delete(randomKey);
        }
    }
    // 40% Chance: Request (Transient Work)
    else {
        // Create garbage, then throw it away immediately
        const temp = { 
            reqId: i, 
            buffer: Buffer.alloc(4096) // 4KB Request
        };
        // 'temp' goes out of scope here (Garbage for GC)
    }

    // Monitor Memory
    if (i % 5000 === 0) {
        const rss = process.memoryUsage().rss / 1024 / 1024;
        if (rss > peakRSS) peakRSS = rss;
        process.stdout.write(`\rOps: ${i} | Sessions: ${sessions.size} | RSS: ${rss.toFixed(2)} MB`);
    }
}

console.log(`\n\n=== RESULTS (V8) ===`);
console.log(`Final RSS: ${(process.memoryUsage().rss / 1024 / 1024).toFixed(2)} MB`);
console.log(`Peak RSS:  ${peakRSS.toFixed(2)} MB`);