// Dummy implementation of the functions in libwork.so
extern "C" {
    int work_init(int seed) {
        return 3;  // Return a dummy number of threads (e.g., 3)
    }

    void work_start_monitoring() {
        // Do nothing (this simulates starting monitoring)
    }

    void work_run() {
        // Simulate running the threads (dummy)
    }
