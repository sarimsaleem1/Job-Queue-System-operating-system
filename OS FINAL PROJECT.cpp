#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

mutex mtx; // Mutex for synchronizing access to shared resources
int jobCounter = 1; 


enum class JobStatus {
    QUEUED,
    RUNNING,
    COMPLETED
};


class Job {
public:
    int jobID;
    string jobName;
    int executionTime;
    JobStatus status;
    int priority; 

    Job(string name, int time, int prio)
        : jobID(jobCounter++), jobName(name), executionTime(time), status(JobStatus::QUEUED), priority(prio) {}

    void execute() {
        status = JobStatus::RUNNING;
        cout << "Job " << jobName << " (ID: " << jobID << ") is running for " << executionTime << " seconds..." << endl;
        this_thread::sleep_for(chrono::seconds(executionTime)); // Simulate job execution
        status = JobStatus::COMPLETED;
        cout << "Job " << jobName << " (ID: " << jobID << ") completed!" << endl;
    }

    
    void displayJob() const {
        string statusStr = (status == JobStatus::QUEUED) ? "Queued" : (status == JobStatus::RUNNING) ? "Running" : "Completed";
        cout << "Job ID: " << jobID << ", Name: " << jobName << ", Status: " << statusStr << ", Priority: " << priority << endl;
    }
};


struct JobComparator {
    bool operator()(const Job& job1, const Job& job2) {
        return job1.priority < job2.priority; 
    }
};


class JobQueue {
private:
    priority_queue<Job, vector<Job>, JobComparator> jobQueue; 
    vector<thread> threads; 

public:
    void submitJob(Job job) {
        lock_guard<mutex> lock(mtx); 
        jobQueue.push(job);
        cout << "Job " << job.jobName << " (ID: " << job.jobID << ") has been submitted!" << endl;
    }

    void executeJobs() {
        while (!jobQueue.empty()) {
            Job currentJob = jobQueue.top();
            jobQueue.pop();
            threads.push_back(thread(&Job::execute, currentJob)); 
        }
        for (auto& t : threads) {
            t.join();// threads complete before exiting
        }
    }

    void viewQueue() {
        lock_guard<mutex> lock(mtx); // Lock for thread safety
        cout << "\nCurrent Jobs in Queue:\n";
        priority_queue<Job, vector<Job>, JobComparator> tempQueue = jobQueue; 
        while (!tempQueue.empty()) {
            tempQueue.top().displayJob();
            tempQueue.pop();
        }
    }

    bool hasJobs() {
        return !jobQueue.empty();
    }
};


int main() {
    JobQueue queue;

    while (true) {
        cout << "\n\t\t\tJob Queue System Menu\n";
        cout << "1. Submit Job\n";
        cout << "2. View Queue\n";
        cout << "3. Execute Jobs\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        int choice;
        cin >> choice;

        if (choice == 1) {
            string jobName;
            int executionTime, priority;
            cout << "Enter Job Name: ";
            cin >> jobName;
            cout << "Enter Job Execution Time (in seconds): ";
            cin >> executionTime;
            cout << "Enter Job Priority (1 - High, 2 - Medium, 3 - Low): ";
            cin >> priority;
            queue.submitJob(Job(jobName, executionTime, priority));
        } else if (choice == 2) {
            queue.viewQueue();
        } else if (choice == 3) {
            queue.executeJobs();
        } else if (choice == 4) {
            break;
        } else {
            cout << "Invalid option! Please try again.\n";
        }
    }

    return 0;
}
