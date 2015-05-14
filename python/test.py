import os, sys, traceback, re

cwd = os.getcwd()
join = os.path.join
print "cwd"
print cwd
print join

def find_lib():
        return os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
wafdir = find_lib()
print wafdir
print sys.path

print __name__
