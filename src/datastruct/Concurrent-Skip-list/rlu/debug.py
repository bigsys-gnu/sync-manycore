import gdb
import gdb.printing
import json
import re


def isinlog(obj_ptr):
    start_addr = gdb.lookup_static_symbol("g_start_addr").value()
    end_addr = gdb.lookup_static_symbol("g_end_addr").value()
    return start_addr <= obj_ptr < end_addr


class CheckObjIsInLog(gdb.Function):
    "check obj is in the log."
    def __init__(self):
        super(CheckObjIsInLog, self).__init__("isinlog")

    def invoke(self, obj_ptr):
        return isinlog(obj_ptr)


class MVHDR(gdb.Function):
    "Return MVRLU struct"
    def __init__(self):
        super(MVHDR, self).__init__("get_mvrlu")
        self.cpy_hdr_type = gdb.lookup_type("struct mvrlu_cpy_hdr_struct")
        self.act_hdr_type = gdb.lookup_type("struct mvrlu_act_hdr_struct")

    def invoke(self, node_ptr):
        mvrlu_hdr = None
        if isinlog(node_ptr):
            mvrlu_hdr_ptr = int(node_ptr) - self.cpy_hdr_type.sizeof
            mvrlu_hdr = gdb.Value(mvrlu_hdr_ptr).cast(self.cpy_hdr_type.pointer())
        else:
            mvrlu_hdr_ptr = int(node_ptr) - self.act_hdr_type.sizeof
            mvrlu_hdr = gdb.Value(mvrlu_hdr_ptr).cast(self.act_hdr_type.pointer())
        return mvrlu_hdr.dereference()


class NodePrinter(object):
    "print node_t"
    def __init__(self, val):
        "docstring"
        self.val = val

    def to_string(self):
        return self.val["key_"]

    def display_hint(self):
        return None


def lookup_function(val):
    lookup_tag = val.type.tag
    if lookup_tag is None:
        return None
    regex = re.compile("^Node<.*>$")
    if regex.match(lookup_tag):
        return NodePrinter(val)
    return None


def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("mvrlu")
    pp.add_printer("Node", "^Node<.*>$", NodePrinter)
    return pp


if __name__ == '__main__':
    CheckObjIsInLog()
    MVHDR()

    gdb.printing.register_pretty_printer(
        gdb.current_objfile(),
        build_pretty_printer()
    )
