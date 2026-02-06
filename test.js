const myHeap = require('./build/Release/my_allocator');

console.log("--- JS: Initializing Heap ---");
myHeap.init(); 

// 1. Allocate
console.log("\n--- JS: Allocating 100 bytes ---");
const ptr1 = myHeap.alloc(100);
// We use 0x${ptr1.toString(16)} to print BigInt as HEX
console.log(`Ptr1 Address: 0x${ptr1.toString(16)}`);

// 2. Free
console.log("\n--- JS: Freeing Ptr1 ---");
myHeap.free(ptr1);

// 3. Re-Allocate (Should get the same address!)
console.log("\n--- JS: Allocating 50 bytes (Should reuse Ptr1) ---");
const ptr2 = myHeap.alloc(50);
console.log(`Ptr2 Address: 0x${ptr2.toString(16)}`);

if (ptr1 === ptr2) {
    console.log("SUCCESS: JavaScript successfully reused C memory!");
} else {
    console.log("FAIL: Addresses are different.");
}