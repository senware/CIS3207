class Process
{
private:
    // increments whenever a process is created
    static int IDcounter;

    //process id
    int procID;
    // not sure if these will still be here when im done
    int cpu_time;    //--
    int disk_time; //--

public:
    //constructor
    Process();
    // returns number of processes created
    static int getIDCounter();
};

// initializes process counter to 0
int Process::IDcounter = 0;

Process::Process()
{
    this->procID = IDcounter;
    IDcounter++;
}

int Process::getIDCounter()
{
    return IDcounter;
}