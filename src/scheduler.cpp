//
// Created by marko on 20.4.22..
//

#include "../h/scheduler.hpp"

List<TCB> Scheduler::readyThreadQueue;

TCB *Scheduler::get()
{
    return readyThreadQueue.removeFirst();
}

void Scheduler::put(TCB *ccb)
{
    readyThreadQueue.addLast(ccb);
}

List<TCB> SleepingQueue::sleepingThreadQueue;

TCB *SleepingQueue::get()
{
    return sleepingThreadQueue.removeFirst();
}

void SleepingQueue::put(TCB *ccb, int ticks)
{
    sleepingThreadQueue.addLast(ccb);
}

void SleepingQueue::decrement(){
if(totalTime==0) return;
totalTime--;
int left = 0;
while(!left){
if(!sleepingThreadQueue.peekFirst()) return; //no threads
left = sleepingThreadQueue.peekFirst()->decrement();
	if(!left){
unsleepFirst();
}
}
}