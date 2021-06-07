#ifndef CRASHHANDLER_SIGNALHANDLER_H
#define CRASHHANDLER_SIGNALHANDLER_H

class CrashHandler
{
public:
	/*
	 * It is enough to add it about function main(). And the constructor will register the signals
	 */
    CrashHandler();
    CrashHandler(const CrashHandler&) = delete;
    virtual ~CrashHandler() = default;

private:
    enum
    {
        MAX_FRAMES = 64,
    };
    static void SignalHandler(int Signal);
    static void WriteStackTrace();
};

#endif