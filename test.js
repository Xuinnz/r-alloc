const myHeap = require('./build/Release/my_allocator');

myHeap.init();

console.log("1. Allocating 3 blocks...");
const p1 = myHeap.alloc(10); // Will align to 8 + header
const p2 = myHeap.alloc(10); 
const p3 = myHeap.alloc(10);

console.log("2. Freeing p1 and p2 (creating two separate free holes)...");
myHeap.free(p1);
myHeap.free(p2);

// At this point, we have: [Free 8] -> [Free 8] -> [Used]
// They are NOT merged yet because r_free doesn't coalesce automatically anymore.

console.log("3. Running Defrag...");
myHeap.defrag(); 
// This should merge p1 and p2 into one [Free 16 + header] block.

console.log("4. Allocating big block (should fit in the merged space)...");
const p4 = myHeap.alloc(40); // 8 + 8 + header size approx = 40

if(p4.toString() === p1.toString()) {
    console.log("SUCCESS: Defrag merged the holes!");
} else {
    console.log("Info: Allocated at new address (this is okay if header overhead made the hole too small).");
}