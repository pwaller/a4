#! /data/pwaller/Projects/External/pypy-dev/pypy/translator/goal/pypy-c

import cppyy as C

def pythonize_message(c):
    print c, [x for x in dir(c) if x.startswith("set_")]
    pass
    
    
#C.add_pythonization("google::protobuf::Message", pythonize_message)
#C.add_pythonization("a4::atlas::ntup::photon::Event", pythonize_message)

C.load_reflection_info("/home/pwaller/Projects/a4/a4Dictionary.so")

print "Loaded"

E = C.gbl.a4.atlas.ntup.photon.Event
#d = E.descriptor()
#print d
#def x(self): return "<Hello {0!r}>".format(self)
#C.gbl.google.protobuf.FieldDescriptor.__repr__ = x
#print d.field_count(), d.field(0)
#print d.field(2).full_name()
Ph = C.gbl.a4.atlas.ntup.photon.Photon
s = C.gbl.std.string()

#print dir(C.gbl.a4.atlas.ntup.photon)
#print type(E).__bases__[0].__bases__[0].__bases__
#print C.gbl.a4.atlas.ntup.photon._cpp_proxy.get_method_names()

E._run_number = E.run_number
E.run_number = property(E._run_number, E.set_run_number)


e = E()
#d = e.GetDescriptor()
#print d, type(d.name()), d.name()
ph = e.add_photons()
ph.set_pt(10)

print e.DebugString()

for ph in e.photons():
    print "Have a photon:", ph.DebugString()

raise SystemExit

for i in xrange(1000000):
    #e.set_run_number(i)
    e.run_number = i
    e.SerializeToString(s)
    e.ParseFromString(s)
    
    if i == 100:
        print e.run_number
        

