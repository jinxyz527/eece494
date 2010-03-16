#!/usr/bin/python
"""
Earliest Deadline First (EDF) Simulator

Usage:
    ./edf_sim.py [options] <Task Execution Time, Task Period> ...

Options:
    -h, --help
            Displays this help and exists
"""

MAX_TICKS = 25

class Task:
    totalTasks = 0

    def __init__(self, exec_time, period):
        self.taskId = self._getNextAvailableTaskId()
        self.execTime = int(exec_time)
        self.period = int(period)
        self.abs_deadline = self.period
        self.ready = 0

    def increaseDeadline(self, curTick):
        self.abs_deadline += self.period
        self.ready = self.period - (curTick % self.period)

    def tick(self, numTicks):
        self.ready -= numTicks
        if self.ready < 0:
            self.ready = 0

    def _getNextAvailableTaskId(self):
        Task.totalTasks += 1
        return Task.totalTasks

    def __str__(self):
        return "Task %d: (%d, %d, %d)" % (self.taskId, self.execTime, self.period, self.abs_deadline)

class Edf:
    def __init__(self, taskList):
        self.curTick = 0
        self.taskList = taskList

    def start(self):
        while self.curTick < MAX_TICKS:
            curTask = self._nextTask()
            if curTask is None:
                self._tick(1)
            else:
                print "%d \t %s" % (self.curTick, curTask)
                self._tick(curTask.execTime)
                curTask.increaseDeadline(self.curTick)

    def _tick(self, numTicks):
        for task in self.taskList:
            task.tick(numTicks)
        self.curTick += numTicks

    def _nextTask(self):
        edTask = None
        for task in self.taskList:
            if task.ready == 0:
                try:
                    # If the absolute deadlines are equal
                    # Always take the task with the lowest ID
                    if task.abs_deadline < edTask.abs_deadline:
                        edTask = task
                except:
                    edTask = task
        return edTask

def main():
    import getopt, sys
    try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["help"])
        for o, a in opts:
            if o in ("-h", "--help"):
                print __doc__
                sys.exit()

        taskList = []
        for arg in args:
            taskParams = arg.split(',')
            task = Task(taskParams[0], taskParams[1])
            taskList.append(task)

        simulate = Edf(taskList).start()

    except getopt.GetoptError, err:
        print str(err)
        sys.exit(2)
    except IndexError, err:
        print __doc__
        print "Error: Missing argument(s)"
        sys.exit(2)

if __name__ == '__main__':
    main()
