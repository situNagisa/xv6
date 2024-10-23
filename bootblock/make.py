import os

current_c_flags = f"-fno-pic -static -fno-builtin -Os -m32 -fno-omit-frame-pointer -fno-exceptions -fno-stack-protector -fno-pie -no-pie"
current_ld_flags = f""
current_dir = os.path.relpath(os.path.dirname(os.path.abspath(__file__)), os.curdir)

def bootasm(project_root:str, compiler:str, flags:str, output_dir:str) -> str:
	output_file = f"{output_dir}/bootasm"
	return (f"{compiler} {flags} {current_c_flags} -I{project_root}/include -c {current_dir}/bootasm.S -o {output_file}", output_file)

def bootmain(project_root:str, compiler:str, flags:str, output_dir:str) -> str:
	output_file = f"{output_dir}/bootmain"
	return (f"{compiler} {flags} {current_c_flags} -I{project_root}/include -c {current_dir}/bootmain.c -o {output_file}", output_file)

def bootblock_obj(project_root:str, compiler:str, linker:str, c_flags:str, ld_flags:str, output_dir:str) -> str:
	asm_cmd, bootasm_file = bootasm(project_root, compiler, c_flags, output_dir)
	main_cmd, bootmain_file = bootmain(project_root, compiler, c_flags, output_dir)
	output_file = f"{output_dir}/bootblock.o"
	cmd = f"{asm_cmd}"
	cmd += f" && {main_cmd}"
	cmd += f" && {linker} {ld_flags} {current_ld_flags} -N -e start -Ttext 0x7C00 -o {output_file} {bootasm_file} {bootmain_file}"
	
	return (cmd, output_file)

def make_bootblock_dump(project_root:str, compiler:str, linker:str, objdump:str, c_flags:str, ld_flags:str, output_dir:str) -> str:
	cmd, input_file = bootblock_obj(project_root, compiler, linker, c_flags, ld_flags, output_dir)
	output_file = f"{input_file}.asm"
	cmd += f" && {objdump} -S {input_file} > {output_file}"
	
	return (cmd, output_file)

def make_bootblock_copy(project_root:str, compiler:str, linker:str, objcopy:str, c_flags:str, ld_flags:str, output_dir:str) -> str:
	cmd, input_file = bootblock_obj(project_root, compiler, linker, c_flags, ld_flags, output_dir)
	output_file,_ = os.path.splitext(input_file)
	cmd += f" && {objcopy} -S -O binary -j .text {input_file} {output_file}"
	cmd += f" && {current_dir}/sign.pl {output_file}"
	
	return (cmd, output_file)

def make_clean(output_dir:str):
	return (f"rm -rf {output_dir}/*", None)