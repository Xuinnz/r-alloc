const myAllocator = require('./build/Release/my_allocator');

// Lifetime Policy Enum
const LIFETIME = {
    TRANSIENT: 0,
    INTERMEDIATE: 1,
    PERSISTENT: 2
};

// 1. Setup Base Heap
console.log("--- Initializing Base Heap ---");
myAllocator.init(); // This calls init_heap() in allocator.c

// 2. Create Two Distinct Arenas
console.log("\n--- Creating Two Arenas ---");
// Arena A: 512 bytes for Transient data
const arenaA = myAllocator.createArena(512, LIFETIME.TRANSIENT);
// Arena B: 256 bytes for Persistent data
const arenaB = myAllocator.createArena(256, LIFETIME.PERSISTENT);

console.log(`Arena A Handle (Pointer): 0x${arenaA.toString(16)}`);
console.log(`Arena B Handle (Pointer): 0x${arenaB.toString(16)}`);

// 3. Allocate in Arena A
console.log("\n--- Allocating in Arena A ---");
const aPtr1 = myAllocator.rArena(arenaA, 64);
const aPtr2 = myAllocator.rArena(arenaA, 64);
console.log(`A1: 0x${aPtr1.toString(16)}`);
console.log(`A2: 0x${aPtr2.toString(16)}`);

// 4. Allocate in Arena B (Should be a completely different range)
console.log("\n--- Allocating in Arena B ---");
const bPtr1 = myAllocator.rArena(arenaB, 64);
console.log(`B1: 0x${bPtr1.toString(16)}`);

// 5. Reset Arena A
console.log("\n--- Resetting Arena A ---");
myAllocator.rReset(arenaA);

// 6. Verify Reset (Should give back A1's address)
const aPtr3 = myAllocator.rArena(arenaA, 64);
console.log(`A3 (After Reset): 0x${aPtr3.toString(16)}`);

if (aPtr3 === aPtr1) {
    console.log("✅ Success: Arena A reset correctly.");
}

// 7. Destroy Arena B
console.log("\n--- Destroying Arena B ---");
myAllocator.rDestroy(arenaB);
console.log("Arena B memory returned to base allocator.");

// 8. Final Check: Arena A should still be valid!
const aPtr4 = myAllocator.rArena(arenaA, 64);
console.log(`A4 (Still working): 0x${aPtr4.toString(16)}`);

if (aPtr4 !== 0n) {
    console.log("✅ Success: Arena A survived Arena B's destruction.");
}