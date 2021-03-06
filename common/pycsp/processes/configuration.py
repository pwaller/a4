"""
Configuration module

Copyright (c) 2009 John Markus Bjoerndalen <jmb@cs.uit.no>,
      Brian Vinter <vinter@diku.dk>, Rune M. Friborg <runef@diku.dk>.
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
  
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.  THE
SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

# Constants
PROCESSES_SHARED_LOCKS            = 0
PROCESSES_SHARED_CONDITIONS       = 1

PROCESSES_ALLOC_MSG_BUFFER        = 2
PROCESSES_MSG_BUFFER_BLOCKSIZE    = 3
                
PROCESSES_ALLOC_QUEUE_PER_CHANNEL = 4
PROCESSES_ALLOC_CHANNELS          = 5
PROCESSES_ALLOC_CHANNELENDS       = 6

# Classes
class Configuration(object):
    """
    Configuration is a singleton class.
    
    >>> A = Configuration()
    >>> B = Configuration()
    >>> A == B
    True

    Retrieve value ( default is 100 Mb)
    >>> Configuration().get(PROCESSES_ALLOC_MSG_BUFFER)
    104857600

    Set value to 64 Kb
    >>> Configuration().set(PROCESSES_ALLOC_MSG_BUFFER, 64*1024) 
    >>> Configuration().get(PROCESSES_ALLOC_MSG_BUFFER)
    65536

    >>> Configuration().set(PROCESSES_ALLOC_MSG_BUFFER, 100*1024*1024)
    """

    __instance = None  # the unique instance
    __conf = {}

    def __new__(cls, *args, **kargs):
        return cls.getInstance(cls, *args, **kargs)

    def __init__(self):
        pass
    
    def getInstance(cls, *args, **kwargs):
        '''Static method to have a reference to **THE UNIQUE** instance'''
        if cls.__instance is None:
            # Initialize **the unique** instance
            cls.__instance = object.__new__(cls)
            
            cls.__conf = {
                PROCESSES_SHARED_LOCKS:20,
                PROCESSES_SHARED_CONDITIONS:20,
                
                PROCESSES_ALLOC_MSG_BUFFER:100*1024*1024,
                PROCESSES_MSG_BUFFER_BLOCKSIZE:10000,
                
                PROCESSES_ALLOC_QUEUE_PER_CHANNEL:1000,
                PROCESSES_ALLOC_CHANNELS:5000,
                PROCESSES_ALLOC_CHANNELENDS:10000
                }
            
        return cls.__instance
    getInstance = classmethod(getInstance)

    def get(self, conf_id):
        return self.__conf[conf_id]

    def set(self, conf_id, value):
        self.__conf[conf_id] = value

# Run tests
if __name__ == '__main__':
    import doctest
    doctest.testmod()
    
