#!/usr/bin/env python

boost_libs = "system filesystem program_options thread chrono"
a4_version = "0.1.0"

import waflib.Logs as msg

def go(ctx):
    from waflib.Options import commands, options
    from os import getcwd
    from os.path import join as pjoin
    options.prefix = pjoin(getcwd(), "install")
    commands += ["configure", "build", "install"]

def fetch_deps(ctx):
    import subprocess
    for script in "./get_miniboost.sh ./get_protobuf.sh".split():
        process = subprocess.Popen([script, "-j%i" % ctx.options.jobs])
        process.wait()

def onchange(ctx):
    """
    Detect changes to source files then run the following waf, e.g. "./waf onchange build"
    """
    
    from sys import argv
    argv = argv[1:]
    assert argv[0] == "onchange"
    argv = argv[1:]

    from pipes import quote
    args = " ".join(quote(arg) for arg in argv)

    from waflib.Options import commands
    commands[:] = []
    
    # System because
    from os import system
    system("common/autocompile.py ./waf %s" % args)
        
    raise SystemExit()

def options(opt):
    from os import getcwd
    from os.path import join as pjoin
    
    # Default the prefix to ${PWD}/install
    prefix_option = opt.parser.get_option("--prefix")
    old_default, new_default = prefix_option.default, pjoin(getcwd(), "install")
    opt.parser.set_default("prefix", new_default)
    prefix_option.help = prefix_option.help.replace(old_default, new_default)
    
    opt.load('compiler_c compiler_cxx python')
    opt.load('boost unittest_gtest libtool compiler_magic check_with platforms iwyu',
             tooldir="common/waf")
    opt.add_option('--with-protobuf', default=None,
        help="Also look for protobuf at the given path")
    opt.add_option('--with-cern-root-system', default=None,
        help="Also looks for the CERN Root System at the given path")
    opt.add_option('--with-snappy', default=None,
        help="Also look for snappy at the given path")
    opt.add_option('--with-boost', default=None,
        help="Also look for boost at the given path")
        
    opt.add_option('--enable-atlas-ntup', action="append", default=[],
        help="Build atlas ntup (e.g, photon, smwz)")
        
    opt.add_option('--disable-a4-python', action="store_true",
        help="Disable a4 python modules (they require python 2.6)")

def configure(conf):
    import os
    from os.path import join as pjoin, exists
    conf.load('compiler_c compiler_cxx python')
    conf.load('boost unittest_gtest libtool compiler_magic check_with platforms iwyu',
              tooldir="common/waf")

    try:
        conf.check(features='cxx cxxprogram', cxxflags="-std=c++0x")
    except:
        try:
            version = conf.cmd_and_log(conf.env.CXX + ["--version"]).split("\n")[0]
        except:
            pass
        else:
            conf.msg("Compiler version:", version, color="RED")
        msg.error("Bad compiler. Require GCC >= 4.4 or recent Clang.")
        raise

    conf.cc_add_flags()
    min_python_version = None
    if not conf.options.disable_a4_python:
        conf.env.enable_a4_python = "1"
        min_python_version = (2, 6)
    conf.check_python_version(min_python_version)
    
    conf.find_program("doxygen", var="DOXYGEN", mandatory=False)

    # comment the following line for "production" run (not recommended)
    conf.env.append_value("CXXFLAGS", ["-g", "-Wall", "-Werror", "-ansi", "-fno-strict-aliasing"])
    conf.env.append_value("CXXFLAGS", ["-std=c++0x"])
    conf.env.append_value("RPATH", [conf.env.LIBDIR])
    conf.env.CXXFLAGS_OPTFAST = "-O2"
    conf.env.CXXFLAGS_OPTSIZE = "-Os"
    conf.env.A4_VERSION = a4_version

    # find useful libraries
    conf.check(features='cxx cxxprogram', lib="m", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="dl", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="rt", uselib_store="DEFLIB",
               mandatory=conf.is_linux())
    conf.check(features='cxx cxxprogram', lib="pthread", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="z", header_name="zlib.h", uselib_store="DEFLIB")

    check_cxx11_features(conf)

    # find root
    root_cfg = "root-config"
    if conf.options.with_cern_root_system:
        root_cfg = pjoin(conf.options.with_cern_root_system, "bin/root-config")
    conf.check_cfg(path=root_cfg, package="", uselib_store="CERN_ROOT_SYSTEM",
                   args='--libs --cflags', mandatory=False)

    # find protobuf
    def find_protoc(*args, **kwargs):
        if "check_path" in kwargs:
            kwargs["path_list"] = [pjoin(kwargs.pop("check_path"), "bin")]
        conf.find_program("protoc", **kwargs)

    try:
        conf.check_with(conf.check_cfg, "protobuf", package="protobuf",
                        atleast_version="2.4.0", args="--cflags --libs",
                        # Explicitly mention /usr here so that the "-I/usr/include"
                        # gets added to `protoc` execution.
                        extra_paths=["./protobuf", "/usr"])
        conf.check_with(find_protoc, "protobuf", extra_paths=["./protobuf"])
    except:
        msg.error("")
        msg.error("Protobuf appears to be unavailable or broken.")
        msg.error("You can get a known working good version in this directory by")
        msg.error("running ./get_protobuf.sh or specifying --with-protobuf=/path/")
        raise

    # find snappy
    conf.check_with(conf.check_cxx, "snappy", lib="snappy",
                    mandatory=False, extra_paths=["./snappy"],
                    define_name="HAVE_SNAPPY")
    
    # find boost
    def check_boost(*args, **kwargs):
        if "check_path" in kwargs:
            check_path = kwargs.pop("check_path")
            if "/miniboost" in check_path:
                kwargs["abi"] = "-a4"
            kwargs["includes"] = pjoin(check_path, "include")
            kwargs["libs"] = pjoin(check_path, "lib")
        conf.check_boost(*args, **kwargs)
    
    try:
        conf.check_with(check_boost, "boost", lib=boost_libs, mt=True,
                        extra_paths=["./miniboost"])
    except:
        msg.error("")
        msg.error("Boost appears to be unavailable or broken.")
        msg.error("You can get a known working good version in this directory by")
        msg.error("running ./get_miniboost.sh or specifying --with-boost=/path/")
        raise
    
    conf.env.enabled_atlas_ntup = conf.options.enable_atlas_ntup
    if conf.options.enable_atlas_ntup:
        conf.msg("Will build atlas ntup: ",
                 ", ".join(conf.options.enable_atlas_ntup),
                 color="WHITE")

    # We should test for these...
    conf.define("HAVE_CSTDINT", 1)
    #conf.define("HAVE_TR1_CSTDINT", 1)
    #conf.define("HAVE_STDINT_H", 1)
    conf.define("HAVE_CSTRING", 1)
    #conf.define("HAVE_TR1_CSTRING", 1)
    #conf.define("HAVE_STRING_H", 1)
    conf.define("HAVE_STD_SMART_PTR", 1)
    #conf.define("HAVE_STD_TR1_SMART_PTR", 1)
    
    conf.start_msg("Installation directory")
    conf.end_msg(conf.env.PREFIX, color="WHITE")
    
    if exists(conf.env.PREFIX) and not os.access(conf.env.PREFIX, os.W_OK):
        conf.msg("", "Installation directory not writable!", color="RED")
        conf.msg("", "'./waf install' as root or specify", color="YELLOW")
        conf.msg("", "an alternative with", color="YELLOW")
        conf.msg("", "'./waf configure --prefix=path'", color="YELLOW")

    conf.to_log("Final environment:")
    conf.to_log(conf.env)
    conf.write_config_header('a4io/src/a4/config.h')

def check_cxx11_features(conf):
    
    conf.check_cxx(
        msg="Checking for C++11 auto keyword",
        fragment="""
            int main(int argc, char* argv[]) {
                auto i = 10;
                return i;
            }""",
        mandatory=True)
    
    conf.check_cxx(
        msg="Checking for C++11 std::atomic",
        fragment="""
            #include <atomic>
            int main(int argc, char* argv[]) {
                std::atomic<int> a;
                volatile int x = 1;
                a += x;
                return a;
            }
        """,
        define_name="HAVE_ATOMIC",
        mandatory=False)
        
    conf.check_cxx(
        msg="Checking for C++11 lambda syntax",
        fragment="""int main(int argc, char* argv[]) {
                        volatile int a = 0;
                        auto x = [&]() { return a; };
                        return x();
                    }""",
        define_name="HAVE_LAMBDA",
        mandatory=False)
        
    conf.check_cxx(
        msg="Checking for C++11 noexcept keyword",
        fragment="""int blarg() noexcept { return 2; }
                    int main(int argc, char* argv[]) {
                        return blarg();
                    }""",
        define_name="HAVE_NOEXCEPT",
        mandatory=False)
        
    conf.check_cxx(
        msg="Checking for C++11 initializer lists",
        fragment="""
            #include <vector>
            int myfunc(const std::initializer_list<int>& x) {
                std::vector<int> v(x);
                return v.front();
            }
        
            int main(int argc, char* argv[]) {
                return myfunc({0, 1, 2});
            }""",
        define_name="HAVE_INITIALIZER_LISTS",
        mandatory=False)

def build(bld):
    from os.path import join as pjoin
    packs = ["a4io", "a4store", "a4process", "a4hist", "a4atlas", "a4root", "a4plot"]

    if bld.options.enable_atlas_ntup:
        raise RuntimeError("--enable-atlas-ntup is a configure time option")

    if bld.cmd == 'doxygen':
        doc_packs(bld, packs)
        return

    libsrc =  list(add_pack(bld, "a4io", [], ["SNAPPY"]))
    libsrc += add_pack(bld, "a4store", ["a4io"], ["CERN_ROOT_SYSTEM"])
    libsrc += add_pack(bld, "a4process", ["a4io", "a4store"])
    libsrc += add_pack(bld, "a4hist",
        ["a4io", "a4store", "a4process"], ["CERN_ROOT_SYSTEM"])
    if bld.env.LIB_CERN_ROOT_SYSTEM:
        libsrc += add_pack(bld, "a4root",
            ["a4io", "a4store", "a4process", "a4hist"], ["CERN_ROOT_SYSTEM"])
    libsrc += add_pack(bld, "a4atlas",
        ["a4io", "a4store", "a4process", "a4hist", "a4root"])
    #bld(features="cxx cxxstlib", target="a4", name="a4static",
    #    vnum=a4_version, use=libsrc)
    #bld(features="cxx cxxshlib", target="a4", vnum=a4_version, use=libsrc)
    
    # Install configuration header
    ch = bld.path.find_resource("a4io/src/a4/config.h")
    bld.install_files("${PREFIX}/include/a4", ch)

    # Install binaries
    binaries = bld.path.ant_glob("a4*/bin/*")
    if not bld.env.enable_a4_python:
        # filter out files ending in .py
        binaries = [b for b in binaries if not b.name.endswith(".py")]
    bld.install_files("${BINDIR}", binaries, chmod=int('0755', 8))

    if bld.env.enable_a4_python:
        # Install python modules
        for pack in packs:
            cwd = bld.path.find_node("%s/python" % pack)
            if not cwd: continue
            files = cwd.ant_glob('**/*.py')
            if not files: continue
            bld.install_files('${PYTHONDIR}', files, cwd=cwd, relative_trick=True)

    # Create this_a4.sh and packageconfig files, symlink python
    bld(rule=write_this_a4, target="this_a4.sh", install_path=bld.env.BINDIR)
    bld(rule=write_pkgcfg, target="a4.pc", install_path=pjoin(bld.env.LIBDIR, "pkgconfig"), always=True)
    #bld.symlink_as(pjoin(bld.env.BINDIR, "python"), bld.env.PYTHON[0])

    # Add post-install checks
    if bld.is_install:
        do_installcheck(bld)

def get_git_version():
    try:
        from commands import getstatusoutput
    except ImportError: # py3
        from subprocess import getstatusoutput
    status, output = getstatusoutput("git describe --dirty")
    if status: return "unknown"
    return output

def write_pkgcfg(task):
    def libstr(use):
        s = []
        if task.env["LIBPATH_"+use]:
            s.extend("-L%s"%l for l in task.env["LIBPATH_"+use])
            s.extend("-l%s"%l for l in task.env["LIB_"+use])
        return " ".join(s)

    def cppstr(use):
        s = []
        s.extend(task.env.get_flat("CPPFLAGS_"+use).split())
        s.extend("-I"+i for i in task.env.get_flat("INCLUDES_"+use).split())
        return " ".join(s)

    lines = []
    from textwrap import dedent
    lines.append(dedent("""
    prefix=%(PREFIX)s
    exec_prefix=${prefix}
    includedir=${prefix}/include
    libdir=%(LIBDIR)s
    CXX=%(CXX)s
    PROTOC=%(PROTOC)s
    git_describe=%(GIT_VERSION)s

    Name: A4
    Description: An Analysis Tool for High-Energy Physics
    URL: https://github.com/JohannesEbke/a4
    Version: %(A4_VERSION)s
    Cflags: -std=c++0x -I%(PREFIX)s/include %(CPPFLAGS_PROTOBUF)s %(CPPFLAGS_BOOST)s %(CPPFLAGS_SNAPPY)s
    Libs: -L${libdir} -la4hist -la4process -la4store -la4io %(protobuflibs)s %(boostlibs)s %(snappylibs)s
    Requires: protobuf >= 2.4
    """ % dict(
        PREFIX=task.env.PREFIX, 
        LIBDIR=task.env.LIBDIR, 
        CXX=task.env.CXX[0], 
        PROTOC=task.env.PROTOC, 
        A4_VERSION=task.env.A4_VERSION, 
        CPPFLAGS_PROTOBUF=cppstr("PROTOBUF"),
        CPPFLAGS_BOOST=cppstr("BOOST"),
        CPPFLAGS_SNAPPY=cppstr("SNAPPY"),
        protobuflibs=libstr("PROTOBUF"),
        boostlibs=libstr("BOOST"),
        snappylibs=libstr("SNAPPY"),
        GIT_VERSION=get_git_version(),
    )))

    task.outputs[0].write("\n".join(lines))
    return 0

def write_header_test(header, cwd):
    include = header.path_from(cwd).lstrip("./")
    def writer(task):
        lines = []
        assert len(task.inputs) == 1
        header = task.inputs[0]
        lines.append("// Check if header is self-sufficient")
        lines.append("#include <a4/%s>" % include)
        lines.append("int main() { return 0; }")
        task.outputs[0].write("\n".join(lines))
    return writer

def add_header_test(bld, cwd, header, opts):
    test_cxx = header.change_ext('_standalone_test.cpp')
    test_exe = header.change_ext('_standalone_test')
    bld(rule=write_header_test(header, cwd), source=[header], target=test_cxx)
    bld.program(features="testt", source=[test_cxx], target=test_exe, **opts)

def write_this_a4(task):
    import os
    lines = []
    if task.env.LIBPATH_PROTOBUF:
        lines.append("# Setup protobuf since it is not installed")
        lines.append("export PKG_CONFIG_PATH=${PKG_CONFIG_PATH:+$PKG_CONFIG_PATH:}%s/pkgconfig"
                     % task.env.LIBPATH_PROTOBUF[0])
        pb_root = os.sep.join(task.env.LIBPATH_PROTOBUF[0].split(os.sep)[:-1])
    if task.env.LIBPATH_SNAPPY:
        lines.append("# Setup snappy since it is not installed")
        lines.append("export LD_LIBRARY_PATH=${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}%s"
                     % task.env.LIBPATH_SNAPPY[0])
    lines.append("# Setup a4 python libaries")
    lines.append("export PYTHONPATH=%s${PYTHONPATH:+:$PYTHONPATH}"
                 % os.path.join(pb_root, "python"))

    from textwrap import dedent
    lines.append(dedent("""
    export PKG_CONFIG_PATH=${PKG_CONFIG_PATH:+$PKG_CONFIG_PATH:}%(LIBDIR)s/pkgconfig
    export PYTHONPATH=%(PYTHONDIR)s${PYTHONPATH:+:$PYTHONPATH}
    export PATH=%(BINDIR)s${PATH:+:$PATH}

    # Get the used compiler with $(pkg-config a4 --variable=CXX)
    # Get the used protoc compiler with $(pkg-config a4 --variable=PROTOC)

    # In Makefiles, do not forget that the syntax is:
    # CXX=$(shell pkg-config a4 --variable=CXX)
    #""" % dict(
        PYTHONDIR=task.env.PYTHONDIR,
        LIBDIR=task.env.LIBDIR,
        BINDIR=task.env.BINDIR)))

    task.outputs[0].write("\n".join(lines))
    return 0

def doc_packs(bld, packs):
    if not bld.env.DOXYGEN:
        bld.fatal("No doxygen executable found! Install doxygen and repeat ./waf configure.")
    srcs = [bld.path.find_node("doc/doxygen")]
    for p in packs:
        s = bld.path.find_or_declare("%s/src" % p)
        if not s:
            continue
        if s.get_src():
            srcs.append(s.get_src())
        if s.get_bld():
            srcs.append(s.get_bld())
    sourcedirs = " ".join(s.abspath() for s in srcs)
    udx = bld.path.find_or_declare("doc/user.doxygen.conf")
    ddx = bld.path.find_or_declare("doc/dev.doxygen.conf")
    bld(rule="cat ${SRC} | sed -e 's|__sourcedirs__|%s|' > ${TGT}" % sourcedirs,
        source="doc/user.doxygen", target=udx)
    bld(rule="cat ${SRC} | sed -e 's|__sourcedirs__|%s|' > ${TGT}" % sourcedirs,
        source="doc/dev.doxygen", target=ddx)
    bld(rule="${DOXYGEN} ${SRC}", source=udx, target="doc/user/html/index.html")
    bld(rule="${DOXYGEN} ${SRC}", source=ddx, target="doc/dev/html/index.html")

def filter_a4atlas(bld, proto_sources):
    # Don't ever build "*_flat.proto" files
    proto_sources = [s for s in proto_sources
                     if not ("_flat" in s.srcpath() and
                              "ntup" in s.srcpath())]

    ntup_dir = bld.srcnode.find_dir("a4atlas/proto/a4/atlas/ntup")

    protos_by_ntup = {}
    final_proto_sources = []
    
    # Build a dict of protos by ntup type, keeping protos which don't belong
    # to the /ntup/ directory
    for proto in proto_sources:
        if proto.is_child_of(ntup_dir):
            ntup_type = proto.path_from(ntup_dir).split("/")[0]
            protos_by_ntup.setdefault(ntup_type, []).append(proto)
        else:
            final_proto_sources.append(proto)
    
    for ntup in bld.env.enabled_atlas_ntup:
        if ntup not in protos_by_ntup:
            raise RuntimeError(
                "Invalid tuple type '%s', valid ones are %s"
                % (ntup, sorted(protos_by_ntup)))
        final_proto_sources.extend(protos_by_ntup[ntup])
    
    return final_proto_sources

def add_pack(bld, pack, other_packs=[], use=[]):
    from os import listdir, readlink
    from os.path import islink, dirname, join as pjoin

    # Add protoc rules
    proto_sources = bld.path.ant_glob("%s/proto/**/*.proto" % pack)
    if pack == "a4atlas":
        proto_sources = filter_a4atlas(bld, proto_sources)
        
    proto_targets = []
    proto_includes = ["%s/proto" % p for p in [pack] + other_packs]
    for protof in proto_sources:
        proto_targets.extend(add_proto(bld, pack, protof, proto_includes))

    # Find all source files to be compiled
    proto_cc = [f for f in proto_targets if f.suffix() == ".cc"]
    proto_h = [f for f in proto_targets if f.suffix() == ".h"]
    proto_py = [f for f in proto_targets if f.suffix() == ".py"]
    lib_cppfiles = bld.path.ant_glob("%s/src/*.cpp" % pack)

    # Find applications and tests to be built
    appdir = "%s/src/apps" % pack
    apps = listdir(appdir)
    app_cppfiles = {}
    for app in apps:
        if islink("%s/%s" % (appdir, app)) and not \
                "/" in readlink("%s/%s" % (appdir, app)):
            target = readlink("%s/%s" % (appdir, app))
            if target.endswith(".cpp"):
                target = target[:-4]
            bld.symlink_as(pjoin(bld.env.BINDIR, app), target)
        elif app.endswith(".cpp"):
            app_cppfiles[app[:-4]] = ["%s/%s" % (appdir, app)]
        else:
            if app == "root2a4" and not bld.env.LIB_CERN_ROOT_SYSTEM:
                # Temporary workaround to prevent root2a4 from being built
                # if ROOT isn't available
                continue
            fls = bld.path.ant_glob("%s/%s/*.cpp" % (appdir, app))
            app_cppfiles[app] = fls
    test_cppfiles = bld.path.ant_glob("%s/src/tests/*.cpp" % pack)
    test_scripts = bld.path.ant_glob("%s/src/tests/*.sh" % pack)
    gtest_cppfiles = bld.path.ant_glob("%s/src/gtests/*.cpp" % pack)

    # Add compilation rules
    to_use = ["DEFLIB", "PROTOBUF", "BOOST"] + use
    to_use += [pjoin(p,p) for p in other_packs]
    incs = ["%s/src" % p for p in [pack] + other_packs]
    libnm = pjoin(pack, pack)

    # Build objects
    objs = []
    if lib_cppfiles:
        bld.objects(source=lib_cppfiles, target=libnm+"_obj", cxxflags="-fPIC",
            use=to_use+["OPTFAST"], includes=incs)
        objs.append(libnm+"_obj")
    if proto_cc:
        bld.objects(source=proto_cc, target=libnm+"_pbobj", cxxflags="-fPIC -Os",
            use=to_use, includes=incs)
        objs.append(libnm+"_pbobj")

    # Build libraries
    if objs:
        bld(features="cxx cxxstlib", target=libnm, vnum=a4_version,
            install_path="${LIBDIR}", use=to_use + objs)
        bld(features="libtool cxx cxxshlib", target=libnm, vnum=a4_version,
            install_path="${LIBDIR}", use=to_use + objs)

    # Set app and test options
    opts = {}
    # link dynamically against shared sublibraries
    opts["use"] = to_use + [pjoin(pack, pack)]
    opts["use"] += [p.upper() for p in [pack] + other_packs]
    # link against liba4.so
    #opts["use"] = to_use + ["a4"]
    # link statically against liba4
    #opts["use"] = to_use + ["a4static"]

    # Build applications
    opts["includes"] = incs
    for app, fls in app_cppfiles.items():
        if fls:
            bld.program(source=fls, target=pjoin(pack,app), **opts)

    opts["install_path"] = None
    # Build test applications
    testapps = []
    for app in test_cppfiles:
        t = pjoin(pack, "tests", str(app.change_ext("")))
        t = bld.path.find_or_declare(t)
        if str(app).startswith("test_"):
            is_test = "testt"
        else:
            is_test = ""
        bld.program(features=is_test, source=[app], target=t, **opts)
        testapps.append(t)

    # Run test scripts
    for testscript in test_scripts:
        if str(testscript).startswith("test_"):
            t = pjoin(pack, "tests", str(testscript))
            t = bld.path.find_or_declare(t)
            tsk = bld(features="testsc", rule="cp ${SRC} ${TGT} && chmod +x ${TGT}",
                source=testscript, target=t,
                use=testapps)

    # Build and run gtests
    if gtest_cppfiles:
        t = pjoin(pack, "tests", "gtest")
        bld.program(features="gtest", source=gtest_cppfiles, target=t, **opts)

    # install headers
    cwd = bld.path.find_node("%s/src/a4" % pack)
    if cwd:
        headers = cwd.ant_glob('**/*.h')
        if headers:
            bld.install_files('${PREFIX}/include/a4', cwd.ant_glob('**/*.h'),
                cwd=cwd, relative_trick=True)
            for h in headers:
                add_header_test(bld, cwd, h, opts)


    if proto_sources:
        cwd = bld.path.find_or_declare("%s/proto/a4" % pack).get_src()
        bld.install_files('${PREFIX}/include/a4', proto_sources, cwd=cwd,
            relative_trick=True)
    
    if proto_h:
        cwd = bld.path.find_or_declare("%s/src/a4" % pack).get_bld()
        bld.install_files('${PREFIX}/include/a4', proto_h, cwd=cwd,
            relative_trick=True)

    if bld.env.enable_a4_python and proto_py:
        cwd = bld.path.find_or_declare("%s/python" % pack).get_bld()
        paths = set(dirname(n.path_from(bld.path.get_bld())) for n in proto_py)
        initfiles = [bld.path.find_or_declare(pjoin(p,"__init__.py")) for p in paths]
        bld(rule="touch ${TGT}", target=initfiles)
        bld.install_files('${PYTHONDIR}', proto_py+initfiles, cwd=cwd,
            relative_trick=True)


    return objs

def add_proto(bld, pack, pf_node, includes):
    from os.path import basename, dirname, join as pjoin
    spack = pack.replace("a4", "a4/")
    pfd, pfn = pf_node.path_from(bld.path.get_src()), str(pf_node)

    if pfd[:len(pjoin(pack, "proto", spack))] != pjoin(pack, "proto", spack):
        bld.fatal("Unexpected file: %s" % pfd)
    pfd = dirname(pfd[len(pjoin(pack, "proto", spack))+1:])

    co = pjoin(pack, "src")
    po = pjoin(pack, "python")
    bld.path.find_or_declare(co)
    bld.path.find_or_declare(po)

    cpptargets = [pjoin(co, spack, pfd, pfn.replace(".proto", e))
        for e in (".pb.cc", ".pb.h")]
    pytarget = [pjoin(po, spack, pfd, pfn.replace(".proto", e))
        for e in ("_pb2.py",)]

    inc_paths = ["-I%s"%i for i in bld.env["INCLUDES_PROTOBUF"]]
    for i in includes:
        res = bld.path.find_node(i)
        if res:
            rel_path = res.path_from(bld.path.get_bld())
            inc_paths.append("-I" + rel_path)
    incs = " ".join(inc_paths)

    targets = [bld.path.find_or_declare(n) for n in cpptargets+pytarget]
    if bld.env.PROTOC:
        pc = bld.env.PROTOC
    else:
        pc = "protoc"
    rule = "%s %s --python_out %s --cpp_out %s ${SRC}" % (pc, incs, po, co)
    bld(rule=rule, source=pf_node, target=targets)
    return targets

def do_installcheck(bld):
    import os
    # set test env
    keys = bld.env.get_merged_dict().keys()

    import os
    os.environ["BINDIR"] = bld.env.BINDIR
    os.environ["SRCDIR"] = bld.path.get_src().abspath()

    test_scripts = bld.path.ant_glob("*/src/tests/install_*.sh")
    for script in test_scripts:
        pack = script.path_from(bld.path).split(os.sep)[0]
        t = os.path.join(pack, "tests", str(script))
        t = bld.path.find_or_declare(t)
        bld(features="testsc", rule="cp ${SRC} ${TGT} && chmod +x ${TGT}",
            source=script.get_src(), target=t, env=bld.env)


from waflib import Build
class doxygen(Build.BuildContext):
    """generate doxygen documentation"""
    fun = 'build'
    cmd = 'doxygen'

