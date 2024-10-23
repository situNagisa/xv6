import os
import glob
from typing import List

current_c_flags = f"-fno-pic -static -fno-builtin -m32 -fno-omit-frame-pointer -fno-exceptions -fno-stack-protector -fno-pie -no-pie "
current_cxx_flags = f"{current_c_flags} -x c++" 
current_ld_flags = f""
current_dir = os.path.relpath(os.path.dirname(os.path.abspath(__file__)), os.curdir)

sources = []
sources += glob.glob(os.path.join(current_dir, '*.c'))
sources += [ f"{current_dir}/{source}" for source in "swtch.S trapasm.S entry.S".split(" ") ]
if os.environ.get('USE_CPP'):
	sources.remove(os.path.join(current_dir, 'kalloc.c'))
	sources += glob.glob(os.path.join(current_dir, '*.cpp'))
	sources.remove(os.path.join(current_dir, 'visual_memory.cpp'))


def entryother(project_root:str, compiler:str, linker:str, objcopy:str, c_flags:str, ld_flags:str, output_dir:str) -> str:
	input_file = f"{current_dir}/entryother.S"
	output_file = f"{output_dir}/entryother"
	cmd = f"{compiler} {c_flags} {current_c_flags} -iquote{project_root}/include -c {input_file} -o {output_file}.o"
	cmd += f" && {linker} {ld_flags} {current_ld_flags} -N -e start -Ttext 0x7000 -o {output_file}.out {output_file}.o"
	cmd += f" && {objcopy} -S -O binary -j .text {output_file}.out {output_file}"

	return (cmd, output_file)

def initcode(project_root:str, compiler:str, linker:str, objcopy:str, c_flags:str, ld_flags:str, output_dir:str) -> str:
	input_file = f"{current_dir}/initcode.S"
	output_file = f"{output_dir}/initcode"
	cmd = f"{compiler} {c_flags} {current_c_flags} -iquote{project_root}/include -c {input_file} -o {output_file}.o"
	cmd += f" && {linker} {ld_flags} {current_ld_flags} -N -e start -Ttext 0 -o {output_file}.out {output_file}.o"
	cmd += f" && {objcopy} -S -O binary {output_file}.out {output_file}"

	return (cmd, output_file)

def vector_asm(output_dir:str) -> str:
	output_file = f"{output_dir}/vectors.S"
	return (f"{current_dir}/vectors.pl > {output_file}", output_file)

def kernel_c_rule(project_root:str, compiler:str, c_flags:str, output_dir:str, source:str) -> str:
	output_file,_ = os.path.splitext(os.path.basename(source))
	output_file = f"{output_dir}/{output_file}.o"
	return (f"{compiler} {c_flags} {current_c_flags} -iquote{project_root}/include -c {source} -o {output_file}", output_file)

def kernel_cxx_rule(project_root:str, compiler:str, cxx_flags:str, output_dir:str, source:str) -> str:
	output_file,_ = os.path.splitext(os.path.basename(source))
	output_file = f"{output_dir}/{output_file}.o"
	return (f"{compiler} {cxx_flags} {current_cxx_flags} -iquote{project_root}/include -c {source} -o {output_file}", output_file)

def kernel_asm_rule(project_root:str, compiler:str, c_flags:str, output_dir:str, source:str) -> str:
	output_file,_ = os.path.splitext(os.path.basename(source))
	output_file = f"{output_dir}/{output_file}.o"
	return (f"{compiler} {c_flags} {current_c_flags} -iquote{project_root}/include -c {source} -o {output_file}", output_file)

def make_kernel(project_root:str, compiler:str, linker:str, objcopy:str, c_flags:str, cxx_flags:str, ld_flags:str, output_dir:str) -> str:
	sources_c = filter(lambda x: x.endswith(".c"), sources)
	sources_cxx = filter(lambda x: x.endswith(".cpp"), sources)
	sources_asm = list(filter(lambda x: x.endswith(".S"), sources))
	output_file = f"{output_dir}/kernel"
	cmd = ": "
	objs = []

	vector_cmd, vector_file = vector_asm(output_dir)
	cmd += f" && {vector_cmd}"
	sources_asm.append(vector_file)

	for source in sources_c:
		obj_cmd, file = kernel_c_rule(project_root, compiler, c_flags, output_dir, source)
		cmd += f" && {obj_cmd}"
		objs.append(file)
	for source in sources_cxx:
		obj_cmd, file = kernel_cxx_rule(project_root, f"x86_64-elf-{compiler}", f"{c_flags} {cxx_flags}", output_dir, source)
		cmd += f" && {obj_cmd}"
		objs.append(file)
	for source in sources_asm:
		obj_cmd, file = kernel_asm_rule(project_root, compiler, c_flags, output_dir, source)
		cmd += f" && {obj_cmd}"
		objs.append(file)
	entryother_cmd, entryother_file = entryother(project_root, compiler, linker, objcopy, c_flags, ld_flags, output_dir)
	cmd += f" && {entryother_cmd}"
	initcode_cmd, initcode_file = initcode(project_root, compiler, linker, objcopy, c_flags, ld_flags, output_dir)
	cmd += f" && {initcode_cmd}"
	cmd += f" && cd {output_dir}"
	cur_dir = os.path.join(os.path.abspath(os.path.curdir), output_dir)
	cmd += f" && {linker} {ld_flags} {current_ld_flags} -T {os.path.relpath(os.path.abspath(current_dir), cur_dir)}/kernel.ld -o {os.path.relpath(os.path.abspath(output_file), cur_dir)} {' '.join([os.path.relpath(os.path.abspath(obj), cur_dir) for obj in objs])} -b binary {os.path.basename(initcode_file)} {os.path.basename(entryother_file)}"
	cmd += f" && cd -"
	return (cmd, output_file)

def make_kernel_symbol(project_root:str, compiler:str, linker:str, objdump:str, objcopy:str, c_flags:str, cxx_flags:str, ld_flags:str, output_dir:str) -> str:
	cmd, kernel_file = make_kernel(project_root, compiler, linker, objcopy, c_flags, cxx_flags, ld_flags, output_dir)
	output_file = os.path.splitext(kernel_file)[0] + ".sym"
	cmd += f" && {objdump} -t {kernel_file} | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > {output_file}"
	return (cmd, output_file)

def make_clean(output_dir:str):
	return (f"rm -rf {output_dir}/*", None)