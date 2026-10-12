from __future__ import annotations
import argparse
import ctypes
from dataclasses import dataclass
from enum import IntEnum, IntFlag, unique
from typing import List, Tuple
from functools import cache

def read_cstr(obj: memoryview, offset: int) -> str:
    for end in range(offset, len(obj)):
        if obj[end] == 0: # \0
            return obj[offset:end+1].tobytes().decode("ascii")
    raise ValueError("Not a C string.")

def check_size(expected: int):
    def f(cls):
        actual = ctypes.sizeof(cls)
        assert actual == expected, (f"{cls.__name__}: expected {expected} bytes, got {actual}")
        return cls
    return f

def get_bytes(path: str) -> bytes:
    with open(path, 'rb') as f:
        return f.read()

ELF32_Half = ctypes.c_uint16
ELF32_Word = ctypes.c_uint32
ELF32_Sword = ctypes.c_int32
ELF32_Addr = ctypes.c_uint32
ELF32_Off = ctypes.c_uint32

ELF_MAGIC = b"\x7fELF"
EI_NIDENT = 16
EM_ARM = 40 # ARM

@unique
class ElfType(IntEnum):
    ET_NONE = 0
    ET_REL = 1
    ET_EXEC = 2
    ET_DYN = 3
    ET_CORE = 4
    ET_LOPROC = 0xFF00
    ET_HIPROC = 0xFFFF

    @staticmethod
    def CheckExecutable(value: int, path: str) -> None:
        try:
            elf_type = ElfType(value)
        except ValueError:
            raise ValueError(f"{path}: need ELF execuatble. Got unknown type {value:#x}.") from None
        if elf_type != ElfType.ET_EXEC:
            raise ValueError(f"{path}: need ELF execuatble. Got {elf_type.name} ({value:#x}).")

@unique
class ElfClass(IntEnum):
    ELFCLASSNONE = 0
    ELFCLASS32 = 1
    ELFCLASS64 = 2
    ELFCLASSNUM = 3

@unique
class ElfData(IntEnum):
    ELFDATANONE = 0
    ELFDATA2LSB = 1
    ELFDATA2MSB = 2

@unique
class ElfSectionNum(IntEnum):
    SHN_UNDEF = 0
    SHN_LORESERVE = 0xFF00 # also SHN_LOPROC
    SHN_HIPROC = 0xFF1F
    SHN_LIVEPATCH = 0xFF20
    SHN_ABS = 0xFFF1
    SHN_COMMON = 0xFFF2
    SHN_HIRESERVE = 0xFFFF

    @staticmethod
    def CheckSectionNum(value: int, path: str) -> None:
        try:
            ElfSectionNum(value)
        except ValueError:
            return
        raise ValueError(f"{path}: Got an invalid section number: {value}")

@unique
class ElfSectionFlags(IntFlag):
    SHF_WRITE = 0x00000001
    SHF_ALLOC = 0x00000002
    SHF_EXECINSTR = 0x00000004
    SHF_MERGE = 0x00000010
    SHF_STRINGS = 0x00000020
    SHF_INFO_LINK = 0x00000040
    SHF_LINK_ORDER = 0x00000080
    SHF_OS_NONCONFORMING = 0x00000100
    SHF_GROUP = 0x00000200
    SHF_TLS = 0x00000400
    SHF_RELA_LIVEPATCH = 0x00100000
    SHF_RO_AFTER_INIT = 0x00200000
    SHF_ORDERED = 0x04000000
    SHF_EXCLUDE = 0x08000000
    SHF_MASKOS = 0x0FF00000
    SHF_MASKPROC = 0xF0000000

@unique
class ElfSectionType(IntEnum):
    SHT_NULL = 0
    SHT_PROGBITS = 1
    SHT_SYMTAB = 2
    SHT_STRTAB = 3
    SHT_RELA = 4
    SHT_HASH = 5
    SHT_DYNAMIC = 6
    SHT_NOTE = 7
    SHT_NOBITS = 8
    SHT_REL = 9
    SHT_SHLIB = 10
    SHT_DYNSYM = 11
    SHT_NUM = 12
    SHT_LOPROC = 0x70000000
    SHT_HIPROC = 0x7FFFFFFF
    SHT_LOUSER = 0x80000000
    SHT_HIUSER = 0xFFFFFFFF

@unique
class ElfSegmentType(IntEnum):
    PT_NULL = 0
    PT_LOAD = 1
    PT_DYNAMIC = 2
    PT_INTERP = 3
    PT_NOTE = 4
    PT_SHLIB = 5
    PT_PHDR = 6
    PT_TLS = 7
    PT_LOOS = 0x60000000
    PT_HIOS = 0x6FFFFFFF
    PT_LOPROC = 0x70000000
    PT_HIPROC = 0x7FFFFFFF
    PT_GNU_EH_FRAME = 0x6474E550
    PT_GNU_STACK = 0x6474E551
    PT_GNU_RELRO = 0x6474E552
    PT_GNU_PROPERTY = 0x6474E553

@unique
class ElfSegmentFlags(IntFlag):
    PF_X = 0x1
    PF_W = 0x2
    PF_R = 0x4

class Size:
    def __get__(self, _, owner):
        return ctypes.sizeof(owner)

class ELF32Struct(ctypes.LittleEndianStructure):
    size = Size()

@check_size(EI_NIDENT)
class ELF32_E_Ident(ELF32Struct):
    _fields_ = [
        ("ei_mag0", ctypes.c_ubyte),
        ("ei_mag1", ctypes.c_ubyte),
        ("ei_mag2", ctypes.c_ubyte),
        ("ei_mag3", ctypes.c_ubyte),
        ("ei_class", ctypes.c_ubyte),
        ("ei_data", ctypes.c_ubyte),
        ("ei_version", ctypes.c_ubyte),
        ("ei_osabi", ctypes.c_ubyte),
        ("ei_abiversion", ctypes.c_ubyte),
        ("ei_pad", ctypes.c_ubyte * 7),
    ]

    def CheckElfMagic(self, path):
        b = bytes([self.ei_mag0, self.ei_mag1, self.ei_mag2, self.ei_mag3])
        if b != ELF_MAGIC:
            raise ValueError(f"{path}: Not an ELF file (no ELF Magic)")

    def Check32bLE(self, path):
        if self.ei_class != ElfClass.ELFCLASS32 or self.ei_data != ElfData.ELFDATA2LSB:
            raise ValueError(f"{path}: Need a 32-bit little-endian ELF")

# https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h#L217-L232
class ELF32_Ehdr(ELF32Struct):
    _fields_ = [
        ("e_ident", ELF32_E_Ident),
        ("e_type", ELF32_Half),
        ("e_machine", ELF32_Half),
        ("e_version", ELF32_Word),
        ("e_entry", ELF32_Addr),
        ("e_phoff", ELF32_Off),
        ("e_shoff", ELF32_Off),
        ("e_flags", ELF32_Word),
        ("e_ehsize", ELF32_Half),
        ("e_phentsize", ELF32_Half),
        ("e_phnum", ELF32_Half),
        ("e_shentsize", ELF32_Half),
        ("e_shnum", ELF32_Half),
        ("e_shstrndx", ELF32_Half),
    ]

    def CheckAggregatable(self, path):
        e_ident: ELF32_E_Ident = self.e_ident
        e_ident.CheckElfMagic(path)
        e_ident.Check32bLE(path)
        ElfType.CheckExecutable(self.e_type, path)
        ElfSectionNum.CheckSectionNum(self.e_shnum, path)

        assert self.e_machine == EM_ARM
        assert self.e_phentsize == ELF32_Phdr.size
        assert self.e_shentsize == ELF32_Shdr.size

# https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h#L257-L266
class ELF32_Phdr(ELF32Struct):
    _fields_ = [
        ("p_type", ELF32_Word),
        ("p_offset", ELF32_Off),
        ("p_vaddr", ELF32_Addr),
        ("p_paddr", ELF32_Addr),
        ("p_filesz", ELF32_Word),
        ("p_memsz", ELF32_Word),
        ("p_flags", ELF32_Word),
        ("p_align", ELF32_Word),
    ]

class ELF32_ProgramHeaders():
    phdrs: List[ELF32_Phdr]

    def __init__(self, obj: memoryview, ehdr: ELF32_Ehdr):
        self.phdrs = [ELF32_Phdr.from_buffer_copy(obj, ehdr.e_phoff + (i * ehdr.e_phentsize)) for i in range(ehdr.e_phnum)]

    @cache
    def PtLoadProgramTables(self) -> List[ELF32_Phdr]:
        return [p for p in self.phdrs if p.p_type == ElfSegmentType.PT_LOAD]
    

# https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h#L326-L337
class ELF32_Shdr(ELF32Struct):
    _fields_ = [
        ("sh_name", ELF32_Word),
        ("sh_type", ELF32_Word),
        ("sh_flags", ELF32_Word),
        ("sh_addr", ELF32_Addr),
        ("sh_offset", ELF32_Off),
        ("sh_size", ELF32_Word),
        ("sh_link", ELF32_Word),
        ("sh_info", ELF32_Word),
        ("sh_addralign", ELF32_Word),
        ("sh_entsize", ELF32_Word),
    ]

class ELF32_SectionHeaders():
    shdrs: List[ELF32_Shdr]
    shstrtab: ELF32_Shdr
    obj: memoryview

    def __init__(self, obj: memoryview, ehdr: ELF32_Ehdr):
        self.shdrs = [ELF32_Shdr.from_buffer_copy(obj, ehdr.e_shoff + (i * ehdr.e_shentsize)) for i in range(ehdr.e_shnum)]
        self.shstrtab: ELF32_Shdr = self.shdrs[ehdr.e_shstrndx]
        self.obj = obj

    @cache
    def AllocatedSections(self) -> List[ELF32_Shdr]:
        """List of all allocated sections (drops: zero-filled .bss/.heap/.stack)"""
        allocated_sections = []
        for s in self.shdrs:
            if s.sh_flags & ElfSectionFlags.SHF_ALLOC and s.sh_type != ElfSectionType.SHT_NOBITS and s.sh_size:
                allocated_sections.append(s)
        return allocated_sections

    @cache
    def SectionNames(self) -> List[str]:
        return [read_cstr(self.obj, self.shstrtab.sh_offset + s.sh_name) for s in self.shdrs]


# https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h#L196-L203
class ELF32_Sym(ELF32Struct):
    _fields_ = [
        ("st_name", ELF32_Word),
        ("st_value", ELF32_Addr),
        ("st_size", ELF32_Word),
        ("st_info", ctypes.c_ubyte),
        ("st_other", ctypes.c_ubyte),
        ("st_shndx", ELF32_Half),
    ]


@dataclass
class ELF:
    path: str
    data: bytes
    ehdr: ELF32_Ehdr
    phdrs: ELF32_ProgramHeaders
    shdrs: ELF32_SectionHeaders

    def __init__(self, path:str):
        """ELF ARM 32 LittleEndian Execuables"""
        obj_bytes = get_bytes(path)
        obj = memoryview(obj_bytes)
        assert len(obj) >= ELF32_Ehdr.size, f"{path}: Too short to be an ELF file."
        self.path = path
        self.data = obj_bytes

        self.ehdr = ELF32_Ehdr.from_buffer_copy(obj, 0)
        self.ehdr.CheckAggregatable(path)
        self.phdrs = ELF32_ProgramHeaders(obj, self.ehdr)
        self.shdrs = ELF32_SectionHeaders(obj, self.ehdr)


# Nothing maps this file into virtual memory page by page, so segments only need word alignment
# instead of the linker's 64 KiB page alignment. p_offset and p_vaddr must still be congruent modulo it.
OUT_ALIGN = 4

# An output segment: its program header, the bytes it stores, and the sections in it (name, section header)

@dataclass
class Segment:

    @dataclass
    class Named_ELF32_Shdr():
        name: bytes
        shdr: ELF32_Shdr

        def __post_init__(self):
            assert self.name[len(self.name)] == b"\0", "Section namemust be a C-string"

    phdr: ELF32_Phdr
    content: bytes
    shdrs: List[Named_ELF32_Shdr]

    def AppendSectionHeader(self, shdr: ELF32_Shdr, sname: str) ->None:
        if self.phdr.p_offset <= shdr.sh_offset < (self.phdr.p_offset + self.phdr.p_filesz):
            if not sname.endswith("\0"):
                sname += "\0"
            self.shdrs.append(Segment.Named_ELF32_Shdr(sname.encode("ascii"), shdr))

    def HasSections(self) -> bool:
        return len(self.shdrs) > 0

    def Range(self) -> Tuple[int, int]:
        return (self.phdr.p_paddr, self.phdr.p_paddr + len(self.content))


def stored_segments(elf: ELF, section_name_prefix: str = ".") -> List[Segment]:
    """
    The PT_LOAD segments that store at least one allocated section, with those sections. This drops segments
    that store nothing (zero-filled .bss/.heap/.stack) and the segment the linker may use to load the ELF
    headers themselves, which would overwrite the start of CODE_SRAM.
    """
    segments: List[Segment] = []

    alloacated_sections = elf.shdrs.AllocatedSections()
    if not alloacated_sections:
        return segments
    
    section_names = elf.shdrs.SectionNames()
    load_program_tables = elf.phdrs.PtLoadProgramTables()

    for p in load_program_tables:
        content = elf.data[p.p_offset:p.p_offset + p.p_filesz]
        segment = Segment(p, content, [])
        for s, name in zip(elf.shdrs.shdrs, section_names):
            segment.AppendSectionHeader(s, section_name_prefix+name.replace(".", ""))
        if segment.HasSections():
            segments.append(segment)
    return segments


def fill_segment(address: int, data: bytes) -> Segment:
    """A segment and section storing data that no ELF segment stores, such as padding and the image check"""
    phdr = ELF32_Phdr(ElfSegmentType.PT_LOAD, 0, address, address, len(data), len(data), ElfSegmentFlags.PF_R, OUT_ALIGN)
    shdr = ELF32_Shdr(0, ElfSectionType.SHT_PROGBITS, ElfSectionFlags.SHF_ALLOC, address, 0, len(data), 0, 0, 1, 0)
    return Segment(phdr, data, [Segment.Named_ELF32_Shdr(b".image_fill\0", shdr)])


def align_up(value: int, align: int) -> int:
    return (value + align - 1) & ~(align - 1)


def aggregate_elf(bootloader_path: str, application_path: str, application_bin_path: str, output_path: str):
    """
    Combine a stub bootloader ELF and the application ELF it boots into one loadable ELF. The application's
    stamped bin supplies the bytes of its region that no application segment stores (padding, image check),
    so that loading the ELF writes the same bytes as flashing the combined bin.
    """
    bootloader = ELF(bootloader_path)
    application = ELF(application_path)
    if bootloader.ehdr.e_flags != application.ehdr.e_flags:
        raise ValueError(f"{bootloader_path} and {application_path} were built for different ARM ABIs (e_flags)")
    
    app_segments = stored_segments(application)
    bootloader_segments = stored_segments(bootloader, ".bootloader_") # bootloader section names need to have this prefix

    app_bin = get_bytes(application_bin_path)

    # TODO: MAKE THIS SIMPLER
    #############################################################################################################
    # The stamped bin starts at the application's lowest stored address and runs to the end of its region.
    # Fill every gap between the application's segments, then the rest of the bin after the last one.
    sorted_segment_content = sorted((s.phdr.p_paddr, s.content) for s in app_segments)
    elf_bin_start = sorted_segment_content[0][0]
    sorted_segment_content.append((elf_bin_start + len(app_bin), b"")) # add end addr because we need to fill all the way
    fills = []
    cursor = elf_bin_start

    for addr, data in sorted_segment_content[1:]:
        # check that the ELF correctly addresses segments as in the application binary
        app_start = addr - elf_bin_start
        if app_bin[app_start: app_start + len(data)] != data:
            raise ValueError(f"{application_bin_path} does not match {application_path} at {addr:#x}")
        if addr > cursor:
            fills.append(fill_segment(cursor, app_bin[cursor - elf_bin_start: app_start]))
        cursor = max(cursor, addr + len(data))
    #############################################################################################################


    segments = bootloader_segments + app_segments + fills

    # Stored bytes must not overlap. Compare storage (physical) addresses: both programs run in the same RAM, so their run (virtual) addresses legitimately overlap.
    ranges = sorted(s.Range() for s in segments)
    if any(start < end for (_, end), (start, _) in zip(ranges, ranges[1:])):
        raise ValueError("Segments overlap: does the bootloader extend past the application's start?")



    # File layout: ELF header, program headers, segment bytes, section names, section headers. The headers
    # are written last, once every offset is known. Sections keep their offset within their segment.
    headers_size = ELF32_Ehdr.size + (len(segments) * ELF32_Phdr.size)
    out = bytearray(headers_size)
    phdrs = []
    shdrs = [ELF32_Shdr()]  # Section 0 is always the null section
    names = bytearray(b"\0")
    for segment in segments:
        out.extend(bytes(align_up(len(out), OUT_ALIGN) + segment.phdr.p_vaddr % OUT_ALIGN - len(out)))
        phdr = ELF32_Phdr.from_buffer_copy(bytes(segment.phdr))
        phdr.p_offset, phdr.p_align = len(out), OUT_ALIGN
        phdrs.append(phdr)
        for named_shdr in segment.shdrs:
            shdr = ELF32_Shdr.from_buffer_copy(bytes(named_shdr.shdr))
            shdr.sh_name, shdr.sh_offset = len(names), len(out) + named_shdr.shdr.sh_offset - segment.phdr.p_offset
            shdr.sh_link = shdr.sh_info = 0  # Section indices change, and loaded sections don't link to others here
            names.extend(named_shdr.name)
            shdrs.append(shdr)
        out.extend(data)

    shdrs.append(ELF32_Shdr(len(names), ElfSectionType.SHT_STRTAB, 0, 0, len(out), 0, 0, 0, 1, 0))
    names.extend(b".shstrtab\0")
    shdrs[-1].sh_size = len(names)
    out.extend(names)
    out.extend(bytes(align_up(len(out), OUT_ALIGN) - len(out)))

    ehdr = ELF32_Ehdr.from_buffer_copy(bytes(application.ehdr))
    ehdr.e_entry = bootloader.ehdr.e_entry  # Execution starts in the bootloader
    ehdr.e_phoff, ehdr.e_phnum = ELF32_Ehdr.size, len(phdrs)
    ehdr.e_shoff, ehdr.e_shnum, ehdr.e_shstrndx = len(out), len(shdrs), len(shdrs) - 1
    out.extend(b"".join(bytes(s) for s in shdrs))
    out[:headers_size] = bytes(ehdr) + b"".join(bytes(p) for p in phdrs)

    with open(output_path, "wb") as f:
        f.write(out)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Combine a stub bootloader ELF and the application ELF it boots into one ELF")
    parser.add_argument("bootloader", help="Stub bootloader ELF")
    parser.add_argument("application", help="Application ELF")
    parser.add_argument("application_bin", help="The application's stamped bin")
    parser.add_argument("output", help="Combined ELF to write")
    args = parser.parse_args()
    aggregate_elf(args.bootloader, args.application, args.application_bin, args.output)