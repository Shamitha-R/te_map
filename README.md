**Prototype**

# TE HashMap

## Overview

C++ Hashmap using Linear probing and Robinhood hashing

* Uses linear probing instead of chaining for higher CPU cache efficiency
* Robin hood hashing to improve the average probing distance
* Faster than std::unordered_map
* Header only

## Usage

```c++
 #include "te_map.h"

int main() {
    te_hashed::te_hashmap<int, int> map;
    
    map.insert(2,3);
    
    auto r = map.find(2);
    
    return 0;
}
```
