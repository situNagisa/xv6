import os
import bootblock.make as bootblock_maker
import kernel.make as kernel_maker
import fs.make as fs_maker
import argparse
import time
import subprocess

def check_gcc_option(option):
    command = f"gcc -E -Werror=unknown-warning-option -Werror=unknown-pragmas -Werror=unknown-attributes -Werror=unknown-argument-warning -Werror=unknown-cpp-option -Werror=unknown-command-line-option -Werror=unknown-gnu-attribute -Werror=unknown-gnu-pragma -Werror=unknown-gnu-objective-c-attribute -Werror=unknown-gnu-statement-expression -Werror=unknown-language-option -Werror=unknown-pragma-parameter -Werror=unknown-warning-option-ignored -Werror=unknown-warning-option-unsupported -Werror=unknown-warning-option-without-flag -Werror=unknown-warning-option-without-value -Werror=unknown-warning-option-with-value {option} -x c /dev/null -o /dev/null"
    try:
        subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT)
        return True
    except subprocess.CalledProcessError as e:
        return False

project_root = os.path.relpath(os.path.dirname(os.path.abspath(__file__)), os.curdir)

class gcc_toolchain:
	def __init__(self, root:str):
		self._root:str = root
		self._dump_machine:str = ""
		for f in os.listdir(os.path.join(self._root, "bin")):
			if f.find("gcc") == -1:
				continue
			result = subprocess.run([f"{self._root}/bin/{f}", "-dumpmachine"], capture_output=True, text=True)
			self._dump_machine = result.stdout.strip()
	
	def root(self) -> str:
		return self._root
	def dump_machine(self) -> str:
		return self._dump_machine
	
	def prefix(self) -> str:
		return f"{self._dump_machine}-"
	
	def cc(self) -> str:
		return f"{self._root}/bin/{self.prefix()}gcc"
	def cxx(self) -> str:
		return f"{self._root}/bin/{self.prefix()}g++"
	def linker(self) -> str:
		return f"{self._root}/bin/{self.prefix()}ld"
	def objdump(self) -> str:
		return f"{self._root}/bin/{self.prefix()}objdump"
	def objcopy(self) -> str:
		return f"{self._root}/bin/{self.prefix()}objcopy"
	
prefix = os.environ.get("TOOLCHAIN_PREFIX") or ""
cc = f"{prefix}gcc"
cxx = f"{prefix}g++"
linker = f"{prefix}ld"
objdump = f"{prefix}objdump"
objcopy = f"{prefix}objcopy"

fast_io_root:str = os.environ.get("FAST_IO_ROOT")
nagisa_root:str = os.environ.get("NAGISA_ROOT")
vcpkg_root:str = os.environ.get("VCPKG_ROOT")

current_c_flags = f"-fno-strict-aliasing -m32 -Wno-error -ffreestanding"
current_cxx_flags = f"{current_c_flags} -std=c++26 -fno-exceptions -fno-rtti -I{fast_io_root}/include -I{nagisa_root}/include -I{vcpkg_root}/installed/x64-windows/include"
current_ld_flags = f"-m elf_i386 " + ('--no-warn-rwx-segments' if check_gcc_option('-Wl,--no-warn-rwx-segments') else '')

build_dir = f"{project_root}/build"

def make_bootblock() -> str:
	cur_build_dir = f"{build_dir}/bootblock"
	os.makedirs(cur_build_dir, exist_ok=True)
	return bootblock_maker.make_bootblock_copy(
		project_root=project_root, 
		compiler=cc, 
		linker=linker, 
		objcopy=objcopy, 
		c_flags=f"{current_c_flags}",
		ld_flags=current_ld_flags,
		output_dir=cur_build_dir
		)

def make_kernel() -> str:
	cur_build_dir = f"{build_dir}/kernel"
	os.makedirs(cur_build_dir, exist_ok=True)
	return kernel_maker.make_kernel(
		project_root=project_root, 
		compiler=cc, 
		linker=linker, 
		objcopy=objcopy, 
		c_flags=current_c_flags + " -MD -g3 -Og",
		cxx_flags=current_cxx_flags,
		ld_flags=current_ld_flags,
		output_dir=cur_build_dir
		)

def make_image() -> str:
	bootblock_cmd, bootblock_file = make_bootblock()
	kernel_cmd, kernel_file = make_kernel()
	output_file = f"{build_dir}/image"
	cmd = f"{bootblock_cmd} && {kernel_cmd}"
	cmd += f" && dd if=/dev/zero of={output_file} count=10000"
	cmd += f" && dd if={bootblock_file} of={output_file} conv=notrunc"
	cmd += f" && dd if={kernel_file} of={output_file} seek=1 conv=notrunc"
	return (cmd, output_file)

def make_filesystem() -> str:
	cur_build_dir = f"{build_dir}/fs"
	os.makedirs(cur_build_dir, exist_ok=True)
	return fs_maker.make_filesystem(
		project_root=project_root, 
		compiler=cc, 
		linker=linker, 
		objdump=objdump, 
		c_flags=current_c_flags + " -O3",
		cxx_flags=current_cxx_flags + " -O3",
		ld_flags=current_ld_flags,
		output_dir=cur_build_dir
		)

def make_clean() -> str:
	cmd = bootblock_maker.make_clean(f"{build_dir}/bootblock")[0]
	cmd += f" && {kernel_maker.make_clean(f'{build_dir}/kernel')[0]}"
	cmd += f" && {fs_maker.make_clean(f'{build_dir}/fs')[0]}"
	cmd += f" && rm -rf {build_dir}/*"
	return cmd

def make_build() -> str:
	os.makedirs(build_dir, exist_ok=True)
	fs_cmd, fs_file = make_filesystem()
	image_cmd, image_file = make_image()
	cmd = f"{fs_cmd} && {image_cmd}"
	return cmd

def make_install(prefix:str) -> str:
	fs_cmd, fs_file = make_filesystem()
	image_cmd, image_file = make_image()
	os.makedirs(prefix, exist_ok=True)
	cmd = f"{fs_cmd} && {image_cmd}"
	cmd += f" && cp {image_file} {fs_file} {prefix}"
	return cmd

def qemu(simulator:str, cpus:int, flags='') -> str:
	fs_cmd, fs_file = make_filesystem()
	image_cmd, image_file = make_image()
	cmd = ":"
	# cmd += f"{fs_cmd} && {image_cmd}"
	cmd += f" && {simulator} -serial mon:stdio -drive file={fs_file},index=1,media=disk,format=raw -drive file={image_file},index=0,media=disk,format=raw -smp {cpus} -m 512 {flags}"
	return cmd

# 创建参数解析器
parser = argparse.ArgumentParser(description='描述你的程序')

# 添加参数
parser.add_argument('--clean', action='store_true', help='清理编译文件')
parser.add_argument('-i', '--install', action='store_true', help='安装程序')
parser.add_argument('-q', '--qemu',  action='store_true', help='运行程序')
parser.add_argument('-g', '--gdb',  action='store_true', help='调试程序')
parser.add_argument('--simulator', type=str, default=f"qemu-system-i386", help='模拟器')
parser.add_argument('--cpus', type=int, default=2, help='CPU数量')
parser.add_argument('--prefix', type=str, default=f"{project_root}/install", help='安装路径')
parser.add_argument('--toolchains', type=str, default=None, help='工具链路径')

# 解析参数
args = parser.parse_args()

if args.clean:
	cmd = make_clean()
	print(cmd.replace("&&", "\n"))
	os.system(cmd)
	exit(0)

if args.install:
	cmd = make_install(args.prefix)
	print(cmd.replace("&&", "\n"))
	os.system(cmd)
	exit(0)

if args.qemu:
	flags = "-S -gdb tcp::26000" if args.gdb else ""
	cmd = qemu(args.simulator, args.cpus, flags)
	print(cmd.replace("&&", "\n"))
	time.sleep(1)
	os.system(cmd)
	exit(0)

cmd = make_build()
print(cmd.replace("&&", "\n"))
os.system(cmd)