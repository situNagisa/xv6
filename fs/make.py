import os
import glob

current_c_flags = f"-fno-pic -static -fno-builtin -m32 -fno-omit-frame-pointer -fno-stack-protector -fno-stack-protector -fno-pie -no-pie "
current_cxx_flags = f"-x c++" 
current_ld_flags = f"-N -e main -Ttext 0"
current_dir = os.path.relpath(os.path.dirname(os.path.abspath(__file__)), os.curdir)

def make_mkfs_tool(project_root:str, compiler:str, c_flags:str, output_dir:str) -> str:
	output_file = f"{output_dir}/mkfs"
	return (f"gcc -iquote{project_root}/include -o {output_file} {current_dir}/tool/mkfs.c", output_file)

def soft_rule(project_root:str, compiler:str, linker:str, objdump:str, c_flags:str, cxx_flags:str, ld_flags:str, output_dir:str, source:str, libs:str) -> str:
	output_file,_ = os.path.splitext(os.path.basename(source))
	output_file = f"{output_dir}/_{output_file}"
	if source.endswith(".c"):
		cmd = f"{compiler} {c_flags} {current_c_flags} -iquote{project_root}/include -c {source} -o {output_file}.o"
	else:
		cmd = f"{compiler} {cxx_flags} {current_cxx_flags} -iquote{project_root}/include -c {source} -o {output_file}.o"
	cmd += f" && {linker} {ld_flags} {current_ld_flags} -o {output_file} {output_file}.o {libs}"
	cmd += f" && {objdump} -t {output_file} | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > {output_file}.sym"
	return (cmd, output_file)

def make_filesystem(project_root:str, compiler:str, linker:str, objdump:str, c_flags:str, cxx_flags:str, ld_flags:str, output_dir:str) -> str:
	mkfs_cmd, mkfs_file = make_mkfs_tool(project_root, compiler, c_flags, output_dir)
	output_file = f"{output_dir}/fs.img"

	cmd = mkfs_cmd
	exes = []

	sources = []
	sources += glob.glob(os.path.join(f"{current_dir}/general", '*.c'))
	sources += glob.glob(os.path.join(f"{current_dir}/general", '*.cpp'))

	ulib_file = f"{output_dir}/ulib.o"
	cmd += f" && {compiler} {c_flags} {current_c_flags} -iquote{project_root}/include -o {ulib_file} -c {current_dir}/ulib/ulib.c"
	printf_file = f"{output_dir}/printf.o"
	cmd += f" && {compiler} {c_flags} {current_c_flags} -iquote{project_root}/include -o {printf_file} -c {current_dir}/ulib/printf.c"
	umalloc_file = f"{output_dir}/umalloc.o"
	cmd += f" && {compiler} {c_flags} {current_c_flags} -iquote{project_root}/include -o {umalloc_file} -c {current_dir}/ulib/umalloc.c"
	usys_file = f"{output_dir}/usys.o"
	cmd += f" && {compiler} {c_flags} {current_c_flags} -gdwarf-2 -Wa,-divide -iquote{project_root}/include -o {usys_file} -c {current_dir}/ulib/usys.S"

	libs = f"{ulib_file} {printf_file} {umalloc_file} {usys_file}"

	for source in sources:
		exe_cmd, exe_file = soft_rule(
			project_root=project_root,
			compiler=compiler,
			linker=linker,
			objdump=objdump,
			c_flags=c_flags,
			cxx_flags=cxx_flags,
			ld_flags=ld_flags,
			output_dir=output_dir,
			source=source,
			libs=libs
		)
		cmd += f" && {exe_cmd}"
		exes.append(exe_file)

	forktest_cmd, forktest_file = soft_rule(
			project_root=project_root,
			compiler=compiler,
			linker=linker,
			objdump=objdump,
			c_flags=c_flags,
			cxx_flags=cxx_flags,
			ld_flags=ld_flags,
			output_dir=output_dir,
			source=f"{current_dir}/forktest.c",
			libs=f"{ulib_file} {usys_file}"
		)
	cmd += f" && {forktest_cmd}"
	exes.append(forktest_file)
	
	cmd += f" && cd {output_dir}"
	cmd += f" && {os.path.abspath(mkfs_file)} {os.path.relpath(output_file, output_dir)} {' '.join([ os.path.relpath(exe, output_dir) for exe in exes ])}"
	cmd += f" && cd -"
	return (cmd, output_file)
	

def make_clean(output_dir:str):
	return (f"rm -rf {output_dir}/*", None)