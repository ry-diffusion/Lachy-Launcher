import cppheaderparser
from pprint import pprint
import datetime
import re
import os
import argparse
from symbolless import generate_version_data

parser = argparse.ArgumentParser(description='Generates symbols.cpp')
parser.add_argument('--armhf', help='Sets the system abi as armhf', action='store_true')
args = parser.parse_args()

symbol_list = []
wrapper_name_counter = {}

out_file = None

def output(text):
    print(text)
    if out_file is not None:
        out_file.write(text)
        out_file.write("\n")

def get_method_path(method):
    if "path" in method:
        method_path = method["path"]
        if method_path.startswith("::"):
            method_path = method_path[2:]
        return method_path
    else:
        method_path = method["namespace"]
        if method_path.endswith("::"):
            method_path = method_path[:-2]
        return method_path

def get_method_wrapper_name(method):
    method_name = method["name"]
    if method["destructor"]:
        method_name = "destructor"
    if method_name.startswith("operator"):
        method_name = "operator"
    name = "_" + get_method_path(method).replace("::", "_") + "_" + method_name
    if name in wrapper_name_counter:
        wrapper_name_counter[name] += 1
        name = name + str(wrapper_name_counter[name])
    else:
        wrapper_name_counter[name] = 1
    return name

def get_mangled_class_name(class_name):
    sp = class_name.split("::")
    ret = ""
    for p in sp:
        ret += str(len(p)) + p
    return ret

PRIMITIVE_TYPES = {
        "char": "c",
        "signed char": "c",
        "unsigned char": "h",
        "short": "s",
        "signed short": "s",
        "unsigned short": "t",
        "int": "i",
        "signed int": "i",
        "unsigned int": "j",
        "long": "l",
        "signed long": "l",
        "unsigned long": "m",
        "long long": "x",
        "unsigned long long": "y",
        "bool": "b",
        "float": "f",
        "double": "d",
}

def expand_cpp_default_templates(type_name):
    ret = re.sub(r"(std::unique_ptr\s*)<([\w:]*)>", r"\1<\2,std::default_delete<\2>>", type_name)
    ret = re.sub(r"(std::vector\s*)<([\w:]*)>", r"\1<\2,std::allocator<\2>>", ret)
    return ret

def get_mangled_type_name(type_name, substitutions):
    type_name = expand_cpp_default_templates(type_name)
    # print(type_name)

    sp = re.findall(r"((unsigned\s*|signed\s*|long\s*|short\s*|[\w:]+)+|[*&<>,])", type_name)
    ret = []
    last_type_start = []
    last_type_start.append(0)
    for p in sp:
        if type(p) is tuple:
            p = p[0]
        if p == "<":
            ret.append("I")
            last_type_start.append(len(ret))
        elif p == ">":
            ret.append("E")
            last_type_start.pop()
        elif p == ",":
            last_type_start[-1] = len(ret)
        elif p == "const":
            ret.insert(last_type_start[-1], "K")
        elif p == "&" and ret[last_type_start[-1]] == "R":
            ret[last_type_start[-1]] = "O"
        elif p == "&":
            ret.insert(last_type_start[-1], "R")
        elif p == "*":
            ret.insert(last_type_start[-1], "P")
        else:
            if p in PRIMITIVE_TYPES:
                ret.append(PRIMITIVE_TYPES[p])
                continue
            if p in substitutions:
                subId = substitutions.index(p)
                if subId > 0:
                    ret.append("S" + str(subId - 1) + "_")
                else:
                    ret.append("S_")
                continue
            np = p.split("::")
            if (np[0] == "std" or np[0] == "mcpe") and np[1] == "string":
                ret.append("Ss")
                continue
            if np[0] == "std" and np[1] == "allocator":
                ret.append("Sa")
                continue
            substitutions.append(p)
            f = True
            for pp in np:
                if f:
                    f = False
                    if pp == "std":
                        ret.append("St")
                        continue
                ret.append(str(len(pp)) + pp)
    return ''.join(ret)

def get_mangled_method(method):
    ret = "_ZN"
    if method["const"]:
        ret += "K"
    ret += get_mangled_type_name(get_method_path(method), [])
    if method["constructor"]:
        ret += "C2"
    elif method["destructor"]:
        ret += "D2"
    else:
        ret += str(len(method["name"])) + method["name"]
    ret += "E"
    if len(method["parameters"]) == 0:
        ret += "v"
    else:
        substitutions = []
        substitutions.append(get_method_path(method))
        for param in method["parameters"]:
            ret += get_mangled_type_name(param["type"], substitutions)
    return ret

def get_mangled_member(member, path):
    ret = "_ZN"
    ret += get_mangled_type_name(path, [])
    ret += str(len(member["name"])) + member["name"] + "E"
    return ret

def get_doxygen_properties(doxygen):
    properties = {}
    lines = doxygen.split('\n')
    for line in lines:
        if line.startswith("///"):
            line = line[3:]
        elif line.startswith("*"):
            line = line[1:]
        line = line.strip()
        if line.startswith("@"):
            key, _, val = line[1:].partition(" ")
            properties[key] = val

    return properties

def process_method(method, is_class, is_legacy):
    method_path = get_method_path(method)
    wrapper_name = get_method_wrapper_name(method)
    mangled_name = get_mangled_method(method)

    vtable_name = None
    if "doxygen" in method:
        props = get_doxygen_properties(method["doxygen"])
        if "symbol" in props:
            mangled_name = props["symbol"]
        if "vtable" in props:
            vtable_name, _, mangled_name = props["vtable"].partition(" ")

    params_str = ""
    params_with_names = ""
    params_for_call = ""
    param_no = 1
    #if not method["static"]:
    #    params_str = method_path + "*"
    #    if method["const"]:
    #        params_str = method_path + " const*"
    #    params_for_call = "this"
    for param in method["parameters"]:
        if len(params_str) > 0:
            params_str += ", "
            params_for_call += ", "
        if len(params_with_names) > 0:
            params_with_names += ", "
        if args.armhf and param["type"] == "float":
            params_str += "int"
            params_with_names += param["type"] + " p" + str(param_no)
            params_for_call += "(int&) p" + str(param_no)
            param_no += 1
            continue
        params_str += param["type"]
        params_with_names += param["type"] + " p" + str(param_no)
        if param["type"].startswith("std::unique_ptr") or param["type"].endswith("&&"):
            params_for_call += "std::move(p" + str(param_no) + ")"
        else:
            params_for_call += "p" + str(param_no)
        param_no += 1
    ret_type = method["rtnType"]
    if ret_type.startswith("static "):
        ret_type = ret_type[len("static "):]
    if vtable_name is not None:
        output("static int vti" + wrapper_name + " = -1;")
    elif method["static"] or not is_class:
        output("static " + ret_type + " (*" + wrapper_name + ")(" + params_str + ");")
    else:
        output("static " + ret_type + " (" + method_path + "::*" + wrapper_name + ")(" + params_str + ")" + (" const" if method["const"] else "") + ";")
    output((ret_type + " " if not method["constructor"] and not method["destructor"] else "") + method_path + "::" + ("~" if method["destructor"] else "") + method["name"] + "(" + params_with_names + ")" + (" const" if method["const"] else "") + " {")
    has_return = ret_type != "void" and ret_type != ""
    if vtable_name is not None:
        if "::" in ret_type:
            ret_type = "::" + ret_type
        output("    union { void* voidp; " + ret_type + " (" + method_path + "::*funcp)(" + params_str + "); } u;")
        output("    u.funcp = nullptr;")
        output("    u.voidp = vtable[vti" + wrapper_name + "];")
        output("    " + ("return " if has_return else "") + "(this->*u.funcp)(" + params_for_call + ");")
    elif method["static"] or not is_class:
        output("    " + ("return " if has_return else "") + wrapper_name + "(" + params_for_call + ");")
    else:
        output("    " + ("return " if has_return else "") + "(this->*" + wrapper_name + ")(" + params_for_call + ");")
    output("}")
    symbol_list.append({
        "name": wrapper_name,
        "symbol": mangled_name,
        "vtable_name": vtable_name,
        "is_legacy": is_legacy
    })


def process_header(file, is_legacy=False):
    print("Processing file " + file)
    cpp_header = cppheaderparser.CppHeader(file)

    for function in cpp_header.functions:
        process_method(function, False, is_legacy)

    for class_name in cpp_header.classes:
        print("Processing class " + class_name)
        class_data = cpp_header.classes[class_name]
        # pprint(class_data)

        class_name_with_namespace = class_data["namespace"] + "::" + class_data["name"]
        if class_name_with_namespace.startswith("::"):
            class_name_with_namespace = class_name_with_namespace[2:]

        # pprint(class_data)
        for member_vis in class_data["properties"]:
            for member in class_data["properties"][member_vis]:
                if not member["static"]:
                    continue
                mangled_name = get_mangled_member(member, class_name_with_namespace)
                if "doxygen" in member:
                    props = get_doxygen_properties(member["doxygen"])
                    if "symbol" in props:
                        mangled_name = props["symbol"]
                m_type = member["type"]
                if m_type.startswith("static "):
                    m_type = m_type[len("static "):]
                output(m_type + " " + class_name_with_namespace + "::" + member["name"] + ";")
                symbol_list.append({
                    "name": class_name_with_namespace + "::" + member["name"],
                    "symbol": mangled_name,
                    "is_legacy": is_legacy
                })

        for method_vis in class_data["methods"]:
            for method in class_data["methods"][method_vis]:
                if method["defined"] or method["pure_virtual"]:
                    continue
                process_method(method, True, is_legacy)

                
def generate_init_func():
    vtables = {}

    output("void minecraft_symbols_init(void* handle) {")
    output("    set_current_version_data(find_version_data(version_data, sizeof(version_data) / sizeof(VersionData), handle));")
    for symbol in symbol_list:
        if "vtable_name" in symbol and symbol["vtable_name"] is not None:
            vt_var_name = "vt_" + symbol["vtable_name"].replace("::", "_")
            if symbol["vtable_name"] not in vtables:
                vt_sym_name = get_mangled_type_name(symbol["vtable_name"], [])
                if "::" in symbol["vtable_name"]:
                    vt_sym_name = "N" + vt_sym_name + "E"
                output("    void** " + vt_var_name + " = (void**) minecraft_dlsym(handle, \"_ZTV" + vt_sym_name + "\") + 2;")
                vtables[symbol["vtable_name"]] = True
            output("    if (vti" + symbol["name"] + " == -1) vti" + symbol["name"] + " = resolve_vtable_func(" + vt_var_name + ", minecraft_dlsym(handle, \"" + symbol["symbol"] + "\"));")
 #           if not symbol["is_legacy"]:
 #               output("    if (vti" + symbol["name"] + " == -1) Log::error(\"MinecraftSymbols\", \"Unresolved vtable symbol: %s\", \"" + symbol["symbol"] + "\");")
            continue
        output("    ((void*&) " + symbol["name"] + ") = minecraft_dlsym(handle, \"" + symbol["symbol"] + "\");")
#        if not symbol["is_legacy"]:
#            output("    if (" + symbol["name"] + " == nullptr) Log::error(\"MinecraftSymbols\", \"Unresolved symbol: %s\", \"" + symbol["symbol"] + "\");")
    output("}")

out_file = open("../src/symbols.cpp", "w")
output("// This file was automatically generated using tools/process_headers.py")
output("// Generated on " + datetime.datetime.utcnow().strftime("%a %b %d %Y %H:%M:%S UTC"))
output("")
output("#include <hybris/dlfcn.h>")
output("#include <log.h>")
output("#include \"symbols_internal.h\"")
output("")
header_dir = "../include/minecraft/"
for file in sorted(os.listdir(header_dir)):
    file_path = os.path.join(header_dir, file)
    if not os.path.isfile(file_path) or not file.endswith(".h"):
        continue
    if file == "symbols.h":
        continue
    output("#include <minecraft/" + file + ">")
    process_header(file_path)
    output("")
legacy_header_dir = header_dir + "legacy/"
for file in sorted(os.listdir(legacy_header_dir)):
    file_path = os.path.join(legacy_header_dir, file)
    if not os.path.isfile(file_path) or not file.endswith(".h"):
        continue
    output("#include <minecraft/legacy/" + file + ">")
    process_header(file_path, True)
    output("")

vd_gen_vt_data = {}
for symbol in symbol_list:
    if "vtable_name" in symbol and symbol["vtable_name"] is not None:
        vt_var_name = "vt_" + symbol["vtable_name"].replace("::", "_")
        if symbol["vtable_name"] not in vd_gen_vt_data:
            vd_gen_vt_data[symbol["vtable_name"]] = {}
        vd_gen_vt_data[symbol["vtable_name"]][symbol["symbol"]] = "vti" + symbol["name"]
        continue
generate_version_data(output, vd_gen_vt_data)

output("")
output("static int resolve_vtable_func(void** vtable, void* what) {")
output("    if (vtable - 2 == nullptr)")
output("        return -1;")
output("    for (int i = 0; ; i++) {")
output("        if (vtable[i] == nullptr)")
output("            return -1;")
output("        if (vtable[i] == what)")
output("            return i;")
output("    }")
output("}")

generate_init_func()
out_file.close()

