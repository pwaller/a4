#! /usr/bin/env python

import waflib
from waflib import Task
from waflib.TaskGen import after_method, before_method, feature

@feature("cxx")
@after_method("before_source")
def do_iwyu(self):
    from IPython import embed
    #embed()
     
    
    for source in self.source:
        if ".pb.cc" in source.abspath(): continue
        self.create_task('iwyu', source, source.change_ext(".iwyu").get_bld())


class iwyu(Task.Task):
    """
    Run iwyu
    """
    color = 'PINK'
    run_str = '${IWYU} ${CXXFLAGS} -I/usr/lib/gcc/x86_64-pc-linux-gnu/4.6.2/include -I/usr/include -I/usr/lib64/clang/3.1/include ${CPPPATH_ST:INCPATHS} ${SRC} &> ${TGT}; true'

def options(ctx):
    pass

def configure(ctx):
    ctx.find_program("include-what-you-use", var="IWYU", mandatory=False)
