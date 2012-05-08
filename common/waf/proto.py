from waflib.TaskGen import extension, feature
from waflib import Task, Utils

def configure(conf):
    from os.path import join as pjoin
    
    conf.check_cfg(package="protobuf", args="--libs --cflags")
    conf.check_cfg(msg="Checking for protobuf exec_prefix",
                   package="protobuf", variables=["exec_prefix"])
    protoc_path = pjoin(conf.env.PROTOBUF_exec_prefix, "bin")
    conf.find_program("protoc", var="PROTOC", path_list=protoc_path)
    
@extension('.proto')
def add_proto(self, node):

    #print "HERE: ", node.
    
    #print dir(self)
    #print self.bld.path.abspath() #.bld_dir()

    proto_cc, proto_h, proto_py = proto_build = [node.change_ext('.pb.cc'), 
        node.change_ext('.pb.h'), node.change_ext('_pb2.py')]
        
    proto_task = self.create_task('BuildProto', node, proto_build)
    self.source.append(proto_cc)
    return proto_task

class BuildProto(Task.Task):
    
    run_str = ('${PROTOC} ${CPPPATH_ST:INCPATHS} -I${SRC[0].bld_dir()} '
               '--cpp_out    ${TGT[0].bld_dir()} '
               '--python_out ${TGT[0].bld_dir()} '
               '${SRC}')
    
    ext_in = ['.proto']
    ext_out = ['.pb.cc', '.pb.h', '.pb.py']

