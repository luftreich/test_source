import sys
import os
import threading
import time

class MyThread(threading.Thread):
      def __init__(self,id):
             threading.Thread.__init__(self)
             self.id=id
      def run(self):
             x=0
             time.sleep(10)
	     print 'hello thread'
             print self.id

def func():
      t.start()
      for i in range(5):
             print i

if __name__ == '__main__':
	print 'test system thread init'
	t=MyThread(0)
	func()
	t.join()
        print 'join thread t over'
