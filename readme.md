# RwLock
A C++ implementation of a Read / Write lock with Writing priority.

### RwLock
A way of reading and writing to a part of memory which is threadsafe.<br>
An RwLock supports **one writer**, or **multiple readers** at once.

### Write Priority
When a write lock is requested, all subsequent read locks will be blocked until the write lock succeeds.

### Usage
```cpp
struct Data {
    int number;
    bool boolean;
};

int main() {

    {
        RwLock<Data> lock;

        /* Reading data */
        int number = lock.read()->number;

        /* Writing data */
        lock.write()->number = 42;
    }
    
    {
        RwLock<int> lock;

        /* Reading data directly */
        int number = lock.read().data();

        /* Writing data directly */
        lock.write().data() = 42;
    }
    
    return 0;
}
```

### Sources
[Kl4rry's RwLock](https://github.com/Kl4rry/RwLock)
