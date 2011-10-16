from google.protobuf.message import Message
from google.protobuf.descriptor import Descriptor, FileDescriptor, FieldDescriptor, EnumDescriptor, EnumValueDescriptor
from google.protobuf.descriptor_pb2 import FileDescriptorProto
from google.protobuf.reflection import GeneratedProtocolMessageType

class TYPE:
    DOUBLE=1
    FLOAT=2
    INT64=3
    UINT64=4
    INT32=5
    FIXED64=6
    FIXED32=7
    BOOL=8
    STRING=9
    GROUP=10
    MESSAGE=11
    BYTES=12
    UINT32=13
    ENUM=14
    SFIXED32=15
    SFIXED64=16
    SINT32=17
    SINT64=18

class CPPTYPE:
    INT32=1
    INT64=2
    UINT32=3
    UINT64=4
    DOUBLE=5
    FLOAT=6
    BOOL=7
    ENUM=8
    STRING=9
    MESSAGE=10

type_to_cpptype = {
    TYPE.DOUBLE: CPPTYPE.DOUBLE ,
    TYPE.FLOAT: CPPTYPE.FLOAT ,
    TYPE.INT64: CPPTYPE.INT64 ,
    TYPE.UINT64: CPPTYPE.UINT64 ,
    TYPE.INT32: CPPTYPE.INT32 ,
    TYPE.FIXED64: CPPTYPE.INT64 ,
    TYPE.FIXED32: CPPTYPE.INT32 ,
    TYPE.BOOL: CPPTYPE.BOOL ,
    TYPE.STRING: CPPTYPE.STRING ,
    TYPE.GROUP: CPPTYPE.MESSAGE ,
    TYPE.MESSAGE: CPPTYPE.MESSAGE ,
    TYPE.BYTES: CPPTYPE.STRING ,
    TYPE.UINT32: CPPTYPE.UINT32 ,
    TYPE.ENUM: CPPTYPE.ENUM ,
    TYPE.SFIXED32: CPPTYPE.INT32 ,
    TYPE.SFIXED64: CPPTYPE.INT64 ,
    TYPE.SINT32: CPPTYPE.INT32 ,
    TYPE.SINT64: CPPTYPE.INT64,
    }
    
default_from_type = {
    CPPTYPE.INT32: 0,
    CPPTYPE.INT64: 0,
    CPPTYPE.UINT32: 0,
    CPPTYPE.UINT64: 0,
    CPPTYPE.DOUBLE: 0.0,
    CPPTYPE.FLOAT: 0.0,
    CPPTYPE.BOOL: False,
    CPPTYPE.ENUM: 0,
    CPPTYPE.STRING: "",
    CPPTYPE.MESSAGE: None
}


class ProtoClassPool(object):
    def __init__(self):
        self.files = []
        self.classes = {}
        self.classes["google.protobuf.FileDescriptorProto"] = FileDescriptorProto.DESCRIPTOR
        self.enums = {}
        self.clean = True
        self._class_ids = {}

    def field_from_proto(self, f, index, package, super_name):
        f_full_name = super_name + "." + f.name
        options = f.options
        has_default = f.HasField("default_value")
        if has_default:
            if type_to_cpptype[f.type] == CPPTYPE.BOOL:
                default = not (str(f.default_value).lower() in ("false", "0"))
            elif type_to_cpptype[f.type] == CPPTYPE.STRING:
                default = f.default_value
            else:
                default = f.default_value
        elif f.label == 3: # 3 == REPEATED
            default = []
        else:
            default = default_from_type[type_to_cpptype[f.type]]
        message_type = None
        enum_type = None
        fqtn = f.type_name.strip(".")
        message_type = fqtn if f.type in (TYPE.MESSAGE, TYPE.GROUP) else None
        enum_type = fqtn if f.type == TYPE.ENUM else None
        return FieldDescriptor(f.name, f_full_name, index, f.number, f.type, 
            type_to_cpptype[f.type], f.label, default, message_type, 
            enum_type=enum_type, containing_type=None,
            is_extension=False, extension_scope=None, options=options)

    def enum_from_proto(self, e, package, filename, super_name=""):
        full_name = ".".join((super_name, e.name)).strip(".")
        values = [EnumValueDescriptor(evd.name, index, evd.number, options=evd.options) 
                  for index, evd in enumerate(e.value)]
        ed = EnumDescriptor(e.name, full_name, filename, values, containing_type=None, 
                options=e.options, file=None, serialized_start=None, serialized_end=None)
        fqn = ".".join((package, super_name, e.name)).replace("..",".")
        self.enums[fqn] = ed
        return ed

    def type_from_proto(self, m, package, filename, super_name=""):
        full_name = ".".join((super_name, m.name)).strip(".")
        containing_type = None # Set by containing type constructor
        fields = [self.field_from_proto(f, index, package, full_name) for index, f in enumerate(m.field)]
        nested_types = [self.type_from_proto(e, package, filename, full_name) for e in m.nested_type]
        enum_types = [self.enum_from_proto(e, package, filename, full_name) for e in m.enum_type]
        extensions = [] #TODO m.extension
        options = m.options
        is_extendable = len(m.extension_range) != 0
        ext_ranges = m.extension_range
        # Note tht we cannot serialize this back if serialized_start/end is not set
        d = Descriptor(m.name, full_name, filename, containing_type, fields,
                       nested_types, enum_types, extensions, options=options,
                       is_extendable=is_extendable, extension_ranges=ext_ranges,
                       file=None, serialized_start=None, serialized_end=None)
        fqn = ".".join((package, super_name, m.name)).replace("..",".")
        self.classes[fqn] = d
        return d

    def add_file_descriptor(self, fdp):
        if any(str(fdp.name) == fd.name for fd in self.files):
            return
        self.clean = False
        fd = FileDescriptor(str(fdp.name), str(fdp.package), serialized_pb=fdp.SerializeToString())
        for m in fdp.message_type:
            fd.message_types_by_name[m.name] = self.type_from_proto(m, str(fdp.package), fdp.name)
        self.files.append(fd)

    @property
    def class_ids(self):
        if self.clean:
            return self._class_ids
        # First fixup message_type and enum_type
        for d in self.classes.itervalues():
            for f in d.fields:
                if isinstance(f.message_type, basestring):
                    f.message_type = self.classes[f.message_type]
                if isinstance(f.enum_type, basestring):
                    f.enum_type = self.enums[f.enum_type]
        # Then recreate class_ids
        self._class_ids = {}
        for fd in self.files:
            for name, d in fd.message_types_by_name.iteritems():
                if not "CLASS_ID" in d.fields_by_name:
                    continue # Doesn't have a CLASS_ID, ignore
                class_id = d.fields_by_name["CLASS_ID"].number
                protoclass = GeneratedProtocolMessageType(str(d.name), (Message,), {"DESCRIPTOR" : d})
                self._class_ids[class_id] = protoclass
        self.clean = True
        return self._class_ids

