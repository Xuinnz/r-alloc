const myAllocator = require('./build/Release/my_allocator');

// Lifetimes Enum (Must match C enum)
const LIFETIME = {
    TRANSIENT: 0,
    INTERMEDIATE: 1,
    PERSISTENT: 2
};

console.log("=== TEST: Arena Allocator Strategy ===");

// 1. Initialize
console.log("\n[1] Initializing Arenas...");
myAllocator.initArenas();

// 2. Allocating in Transient (Bump Pointer Check)
console.log("\n[2] Allocating 3 objects in Transient Arena...");
const t1 = myAllocator.arenaAlloc(10, LIFETIME.TRANSIENT);
const t2 = myAllocator.arenaAlloc(10, LIFETIME.TRANSIENT);
const t3 = myAllocator.arenaAlloc(10, LIFETIME.TRANSIENT);

console.log(`    T1 Address: 0x${t1.toString(16)}`);
console.log(`    T2 Address: 0x${t2.toString(16)}`);
console.log(`    T3 Address: 0x${t3.toString(16)}`);

// Calculate distance between pointers (Should be 16 bytes: 10 rounded to 8, plus header? No header in arena!)
// Wait, your arena logic DOES align to 8. So 10 -> 16 bytes.
const diff = BigInt(t2) - BigInt(t1);
console.log(`    -> Bump Distance: ${diff} bytes (Expected ~16)`);


// 3. Allocating in Persistent (Isolation Check)
console.log("\n[3] Allocating 1 object in Persistent Arena...");
const p1 = myAllocator.arenaAlloc(50, LIFETIME.PERSISTENT);
console.log(`    P1 Address: 0x${p1.toString(16)} (Should be far from T1)`);


// 4. The Reset Test (The "Thesis Moment")
console.log("\n[4] Resetting Transient Arena (Wiping Clean)...");
myAllocator.arenaReset(LIFETIME.TRANSIENT);


// 5. Re-Allocating (Reuse Check)
console.log("\n[5] Allocating New Transient Object...");
const new_t1 = myAllocator.arenaAlloc(10, LIFETIME.TRANSIENT);
console.log(`    New T1 Address: 0x${new_t1.toString(16)}`);

if (t1 === new_t1) {
    console.log("\n[SUCCESS] The allocator reused the exact same memory address!");
    console.log("          This proves O(1) bulk deallocation works.");
} else {
    console.log("\n[FAIL] Address mismatch. The pointer did not reset correctly.");
}