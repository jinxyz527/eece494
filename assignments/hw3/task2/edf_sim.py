#!/usr/bin/python
"""
EDF Simulator

Usage:
    ./edf_sim.py [options] <Task Execution Time, Task Period> ...

Options:
    -h, --help
            Displays this help and exists
"""

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
    curTick = 0

    def __init__(self, taskList):
        self.taskList = taskList

    def start(self):
        while Edf.curTick < 24:
            curTask = self._nextTask()
            if curTask is None:
                Edf.curTick += 1
                continue

            print "%d : %s" % (Edf.curTick, curTask)
            for task in self.taskList:
                task.tick(curTask.execTime)
            Edf.curTick += curTask.execTime
            curTask.increaseDeadline(Edf.curTick)

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
