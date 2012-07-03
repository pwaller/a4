set -e -u

# Yes, including everything is overkill, what of it?
#echo "#define private public" > Dictionary.h
echo > Dictionary.h
find build -iname "*.pb.h" | cut -d'/' -f4- | sed -E 's/(.*)/#include <\1>/g' >> Dictionary.h
echo >> Dictionary.h
find protobuf/include -iname "*.h" | cut -d'/' -f3- | sed -E 's/(.*)/#include <\1>/g' >> Dictionary.h

cat > Dictionary.xml <<LCGDICT
<lcgdict>
    <!-- <class pattern="*Event" /> -->
    <class pattern="a4::atlas::ntup::photon::Event" />
    
    <class name="google::protobuf::Message" />
    <class name="google::protobuf::MessageLite" />
    
    <class pattern="google::protobuf::*" />
    <class name="std::string" />
    
<exclusion>    
    <class name="google::protobuf::FieldDescriptor" />
    <class pattern="*google::protobuf::compiler::*" />
    <class pattern="*google::protobuf::internal::*" />
    <class pattern="*google::protobuf::io::FileInputStream" />
    <class pattern="*google::protobuf::io::IstreamInputStream" />
    <class pattern="*google::protobuf::io::FileOutputStream" />
    <class pattern="*google::protobuf::io::OstreamOutputStream" />
    
    <class name="google::protobuf::io::CodedInputStream">
        <method pattern="Internal*" />
    </class>
    
    <class name="google::protobuf::UnknownField" />
</exclusion>
</lcgdict>
LCGDICT

genreflex Dictionary.h --selection=Dictionary.xml -Ibuild/a4{atlas,io,hist,store,root}/src -Iprotobuf/include --gccxmlopt='--gccxml-compiler g++-4.3.4' # --with-methptrgetter--debug=1

echo Compiling..
time g++ -fPIC -rdynamic -shared -Ibuild/a4{atlas,io,hist,store,root}/src \
    -I/usr/include/root -L/usr/lib/root -lReflex    \
    -Lbuild/a4atlas/ -Wl,-rpath,${PWD}/install/lib \
    -Lprotobuf/lib -Wl,-rpath,${PWD}/protobuf/lib \
    -la4atlas -oa4Dictionary.so Dictionary_rflx.cpp -lprotobuf
    
    
echo Done
