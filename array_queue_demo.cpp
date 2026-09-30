#include <iostream>
#include <string>

using namespace std;

// Maximum capacity for the circular queue
const int QUEUE_CAPACITY = 5;

// Structure representing an array-based Circular Queue
struct ArrayQueue {
    string data[QUEUE_CAPACITY];
    int front;
    int rear;
    int count;
};

// Initializes the circular queue to an empty state
// Time Complexity: O(1)
void initQueue(ArrayQueue& q) {
    q.front = 0;
    q.rear = -1;
    q.count = 0;
}

// Checks if queue is empty
// Time Complexity: O(1)
bool isEmpty(const ArrayQueue& q) {
    return q.count == 0;
}

// Checks if queue is full
// Time Complexity: O(1)
bool isFull(const ArrayQueue& q) {
    return q.count == QUEUE_CAPACITY;
}

// Inserts an element at the rear (enqueue)
// Time Complexity: O(1)
bool enqueue(ArrayQueue& q, const string& item) {
    if (isFull(q)) {
        cout << "[Queue Error] Cannot enqueue \"" << item << "\": Queue is FULL!\n";
        return false;
    }
    // Wrap around using modulo arithmetic
    q.rear = (q.rear + 1) % QUEUE_CAPACITY;
    q.data[q.rear] = item;
    q.count++;
    cout << "[Enqueue] Added: \"" << item << "\" (Count: " << q.count << ")\n";
    return true;
}

// Removes and returns the front element (dequeue)
// Time Complexity: O(1)
bool dequeue(ArrayQueue& q, string& outItem) {
    if (isEmpty(q)) {
        cout << "[Queue Error] Cannot dequeue: Queue is EMPTY!\n";
        return false;
    }
    outItem = q.data[q.front];
    // Wrap around front pointer
    q.front = (q.front + 1) % QUEUE_CAPACITY;
    q.count--;
    cout << "[Dequeue] Removed: \"" << outItem << "\" (Count: " << q.count << ")\n";
    return true;
}

// Inspects the front element without removing it
// Time Complexity: O(1)
bool peek(const ArrayQueue& q, string& outItem) {
    if (isEmpty(q)) {
        return false;
    }
    outItem = q.data[q.front];
    return true;
}

// Displays all elements currently in the queue
// Time Complexity: O(N) where N is number of elements
void displayQueue(const ArrayQueue& q) {
    if (isEmpty(q)) {
        cout << "Queue contents: [EMPTY]\n";
        return;
    }
    cout << "Queue contents (Front to Rear): ";
    for (int i = 0; i < q.count; i++) {
        int idx = (q.front + i) % QUEUE_CAPACITY;
        cout << "[" << q.data[idx] << "] ";
    }
    cout << "\n";
}

int main() {
    cout << "========================================================\n";
    cout << "  BONUS DEMO: Array-Based Circular Queue (Module IX)    \n";
    cout << "========================================================\n\n";

    ArrayQueue q;
    initQueue(q);

    // 1. Enqueue operations
    enqueue(q, "Passenger 1 (Alice)");
    enqueue(q, "Passenger 2 (Bob)");
    enqueue(q, "Passenger 3 (Charlie)");
    displayQueue(q);

    // 2. Dequeue operation
    string served;
    dequeue(q, served);
    cout << "Served passenger: " << served << "\n";
    displayQueue(q);

    // 3. Add more to demonstrate circular wrap-around
    enqueue(q, "Passenger 4 (Diana)");
    enqueue(q, "Passenger 5 (Evan)");
    enqueue(q, "Passenger 6 (Frank)");
    displayQueue(q);

    // 4. Try overflow
    enqueue(q, "Passenger 7 (Overflow)");

    cout << "\n========================================================\n";
    cout << "  Circular Queue demonstration completed successfully!  \n";
    cout << "========================================================\n";

    return 0;
}
