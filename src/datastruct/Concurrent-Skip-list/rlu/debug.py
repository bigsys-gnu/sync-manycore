import gdb
import gdb.printing
import json
import re


def gdb_print(msg):
    gdb.write(f"{msg}\n", gdb.STDOUT)


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


class NodeToMVRLU(gdb.Function):
    "Return MVRLU struct"
    def __init__(self):
        super(NodeToMVRLU, self).__init__("node_to_mvrlu")
        self.cpy_hdr_type = gdb.lookup_type("struct mvrlu_cpy_hdr_struct")
        self.act_hdr_type = gdb.lookup_type("struct mvrlu_act_hdr_struct")

    def invoke(self, node_ptr):
        mvrlu_hdr = None
        if isinlog(node_ptr):
            gdb_print("COPY HDR")
            mvrlu_hdr_ptr = int(node_ptr) - self.cpy_hdr_type.sizeof
            mvrlu_hdr = gdb.Value(mvrlu_hdr_ptr).cast(self.cpy_hdr_type.pointer())
        else:
            gdb_print("ACT HDR")
            mvrlu_hdr_ptr = int(node_ptr) - self.act_hdr_type.sizeof
            mvrlu_hdr = gdb.Value(mvrlu_hdr_ptr).cast(self.act_hdr_type.pointer())
        return mvrlu_hdr


class CastMVRLU(gdb.Function):
    "cast pointer to mvrlu hdr"
    def __init__(self):
        super(CastMVRLU, self).__init__("cast_mvrlu")
        self.cpy_hdr_type = gdb.lookup_type("struct mvrlu_cpy_hdr_struct")
        self.act_hdr_type = gdb.lookup_type("struct mvrlu_act_hdr_struct")

    def invoke(self, mvrlu_hdr_ptr):
        mvrlu_hdr = None
        if isinlog(mvrlu_hdr_ptr):
            gdb_print("COPY HDR")
            mvrlu_hdr = gdb.Value(mvrlu_hdr_ptr).cast(self.cpy_hdr_type.pointer())
        else:
            gdb_print("ACT HDR")
            mvrlu_hdr = gdb.Value(mvrlu_hdr_ptr).cast(self.act_hdr_type.pointer())
        return mvrlu_hdr


class NodePrinter(object):
    "print node_t"
    def __init__(self, node):
        self.val = node

    def to_string(self):
        msg = f'node height: {self.val["top_level_"]}\n'
        msg += f'key: {self.val["key_"]}\n'
        return msg

    def display_hint(self):
        return None

    def children(self):
        # _Nm is size
        # _M_elems is static array
        slots = self.val["next"]
        items = slots["__elems_"].cast(slots.type.template_argument(0).pointer())
        for i in range(int(self.val["top_level_"] + 1)):
            yield f"\n{i}", items.dereference()
            items += 1


def pretty_obj_hdr(obj_hdr):
    return json.dumps({
        "obj_size" : f"{obj_hdr['obj_size']}",
        "padding_size" : f"{obj_hdr['padding_size']}",
        "type" : f"{obj_hdr['type']}",
        "p_copy" : f"{int(obj_hdr['p_copy']):#x}",
        "obj[0]" : f"{int(obj_hdr['obj'].address):#x}",
    }, indent=2)


class CopyHdrPrinter(object):
    "Print MV-RLU Copy Header Struct"
    def __init__(self, hdr):
        self.cpy_hdr = hdr["cpy_hdr"]
        self.obj_hdr = hdr["obj_hdr"]

    def to_string(self):
        msg = "Copy Header\n"
        structure = {
            "p_wrt_clk" : f"{int(self.cpy_hdr['p_wrt_clk']):#x}",
            "wrt_clk_next" : f"{self.cpy_hdr['wrt_clk_next']}",
            "__wrt_clk" : f"{self.cpy_hdr['__wrt_clk']}",
            "p_act" : f"{self.cpy_hdr['p_act']}",
        }
        msg += f"cpy_hdr = {json.dumps(structure, indent=2)}\n"
        msg += f"obj_hdr = {pretty_obj_hdr(self.obj_hdr)}"
        return msg


class ActHdrPrinter(object):
    "Print MV-RLU Actual Header Struct"
    def __init__(self, hdr):
        self.act_hdr = hdr["act_hdr"]
        self.obj_hdr = hdr["obj_hdr"]

    def to_string(self):
        msg = "Actual Header\n"
        structure = {
            "p_lock" : f"{int(self.act_hdr['p_lock']):#x}",
        }
        msg += f"act_hdr = {json.dumps(structure, indent=2)}\n"
        msg += f"obj_hdr = {pretty_obj_hdr(self.obj_hdr)}"
        return msg

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("mvrlu")
    pp.add_printer("Node", "^Node<.*>$", NodePrinter)
    pp.add_printer("Copy Hdr", "^mvrlu_cpy_hdr_struct$", CopyHdrPrinter)
    pp.add_printer("Act Hdr", "^mvrlu_act_hdr_struct$", ActHdrPrinter)
    return pp


if __name__ == '__main__':
    CheckObjIsInLog()
    NodeToMVRLU()
    CastMVRLU()

    gdb.printing.register_pretty_printer(
        gdb.current_objfile(),
        build_pretty_printer()
    )
