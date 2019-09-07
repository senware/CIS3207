class Process
{
private:
    // increments whenever a process is created
    static int IDcounter;

    //process id
    int procID;
    // not sure if these will still be here when im done
    int cpu_time;  //--
    int disk_time; //--

public:
    // constructor
    Process();

    // returns number of processes created
    static int getIDCounter();

    // returns process identifier
    int get_ID();
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

int Process::get_ID()
{
    return this->procID;
}