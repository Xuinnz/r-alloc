const myAllocator = require('./build/Release/my_allocator');

// Lifetime Enum
const LIFETIME = {
    TRANSIENT: 0,
    INTERMEDIATE: 1, // <--- We are testing this one!
    PERSISTENT: 2
};

console.log("=== TEST: Slab Allocator (Intermediate) ===");

// 1. Initialize
myAllocator.init();
const arena = myAllocator.createArena(4096, LIFETIME.INTERMEDIATE); // 4KB Arena

// 2. Allocate two objects (Size 40 -> Rounds up to 64-byte Slab)
console.log("\n[1] Allocating Objects A and B...");
const ptrA = myAllocator.rArena(arena, 40);
const ptrB = myAllocator.rArena(arena, 40);

console.log(`    Object A: 0x${ptrA.toString(16)}`);
console.log(`    Object B: 0x${ptrB.toString(16)}`);

// 3. Free Object A (This should go back onto the Free List)
console.log("\n[2] Freeing Object A...");
// Note: We must tell it the size (40) so it knows which Slab Class to put it in!
myAllocator.rArenaFree(arena, ptrA, 40);

// 4. Allocate Object C (Should reuse A's spot!)
console.log("\n[3] Allocating Object C...");
const ptrC = myAllocator.rArena(arena, 40);
console.log(`    Object C: 0x${ptrC.toString(16)}`);

// 5. The Verdict
if (ptrC === ptrA) {
    console.log("\n✅ SUCCESS: The allocator recycled Object A's memory for Object C!");
    console.log("             (Zero Fragmentation confirmed)");
} else {
    console.log("\n❌ FAIL: The allocator allocated new memory instead of recycling.");
    console.log("         (Slab logic is broken)");
}

// 6. Cleanup
myAllocator.rDestroy(arena);