#include "types.h"
#include "defs.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"
#include <fast_io_dsal/array.h>
#include <fast_io_dsal/list.h>
#include "xv/page.h"
#include "xv6/page/page.h"
#include "xv/segment.h"
#include "./kernel_allocate.h"

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
extern "C" void seginit(void)
{
	constexpr auto global_descriptor_table = ::xv::segments::segment_descriptor_table{
		::xv::memmod::segment_data::create_from(
			0x00000000,
			0x00000000,
			{}
		),
		::xv::memmod::segment_code::create_from(
			0x00000000,
			0xffffffff,
			{
				.type = {
					.accessed = false,
					.readable = true,
					.conforming = false,
					.type = ::xv::memmod::descriptor_type_t::code_or_data,
					.privilege = ::xv::memmod::privilege_t::ring0,
					.present = true,
				},
				.available = false,
				.code_bit = ::xv::memmod::code_bit_t::compatibility,
				.operation_size = ::xv::memmod::bit_width_t::bits32,
				.enable_page_granularity = true,
			}
		),
		::xv::memmod::segment_data::create_from(
			0x00000000,
			0xffffffff,
			{
				.type = {
					.accessed = false,
					.writable = true,
					.expand_down = false,
					.type = ::xv::memmod::descriptor_type_t::code_or_data,
					.privilege = ::xv::memmod::privilege_t::ring0,
					.present = true,
				},
				.available = false,
				.code_bit = ::xv::memmod::code_bit_t::compatibility,
				.operation_size = ::xv::memmod::bit_width_t::bits32,
				.enable_page_granularity = true,
			}
		),
		::xv::memmod::segment_code::create_from(
			0x00000000,
			0xffffffff,
			{
				.type = {
					.accessed = false,
					.readable = true,
					.conforming = false,
					.type = ::xv::memmod::descriptor_type_t::code_or_data,
					.privilege = ::xv::memmod::privilege_t::ring3,
					.present = true,
				},
				.available = false,
				.code_bit = ::xv::memmod::code_bit_t::compatibility,
				.operation_size = ::xv::memmod::bit_width_t::bits32,
				.enable_page_granularity = true,
			}
		),
		::xv::memmod::segment_data::create_from(
			0x00000000,
			0xffffffff,
			{
				.type = {
					.accessed = false,
					.writable = true,
					.expand_down = false,
					.type = ::xv::memmod::descriptor_type_t::code_or_data,
					.privilege = ::xv::memmod::privilege_t::ring3,
					.present = true,
				},
				.available = false,
				.code_bit = ::xv::memmod::code_bit_t::compatibility,
				.operation_size = ::xv::memmod::bit_width_t::bits32,
				.enable_page_granularity = true,
			}
		),
		::xv::memmod::segment_data::create_from(
			0x00000000,
			0x00000000,
			{}
		),
	};
	static_assert(sizeof(global_descriptor_table) == (::xv::memmod::segment_descriptor_size * ::std::ranges::size(global_descriptor_table)));
	static_assert(::std::ranges::size(global_descriptor_table) == NSEGS);

	auto&& cpu = cpus[cpuid()];

	// Map "logical" addresses to virtual addresses using identity map.
	// Cannot share a CODE descriptor for both kernel and user
	// because it would have to have DPL_USR, but the CPU forbids
	// an interrupt from CPL=0 to DPL=3.
#ifdef __cpp_lib_freestanding_algorithm
	::std::ranges::copy(::std::bit_cast<::fast_io::array<segdesc, NSEGS>>(global_descriptor_table), ::std::ranges::begin(cpu.gdt));
#else
	for (auto&& [dst, src] : ::std::views::zip(cpu.gdt, global_descriptor_table))
	{
		dst = ::std::bit_cast<segdesc>(src);
	}
#endif
	::lgdt(::std::ranges::data(cpu.gdt), sizeof(cpu.gdt));
}


extern char data[];  // defined by kernel.ld
::xv6::pages::page_directory* kernel_page_directory;  // for use in scheduler()

namespace layout_factory
{
	inline constexpr auto ptv = ::xv6::pages::address_transformer_pair::physical_to_visual<void>;
	inline constexpr auto vtp = ::xv6::pages::address_transformer_pair::visual_to_physical<void>;

	inline constexpr auto physical_start = static_cast<::std::uintptr_t>(0);
	inline constexpr auto physical_end = ::xv6::pages::layouts::top_physical_memory;
	inline constexpr auto physical_device_start = ::xv6::pages::layouts::device_space;
	inline constexpr auto physical_device_end = physical_start;
}

struct kernel_visual_memory
{
	using directory_type = ::xv6::pages::page_directory;
	using directory_view_type = decltype(
		::std::declval<::std::views::all_t<directory_type&>>() | ::std::views::drop(0) | ::std::views::take(0)
		);
	using holder_type = ::xv6::pages::page_table_range_holder<directory_view_type>;

	kernel_visual_memory(directory_type& directory) noexcept
		: _directory(directory)
	{
		auto visual_start = ::std::bit_cast<void const*>(::xv6::pages::layouts::kernel_base);
		auto visual_code = visual_start + ::xv6::pages::layouts::extended_memory;
		auto visual_data = static_cast<void const*>(data);
		auto visual_end = ::layout_factory::ptv(::xv6::pages::layouts::top_physical_memory);
		auto visual_device = ::std::bit_cast<void const*>(::xv6::pages::layouts::device_space);
		auto visual_device_end = ::std::bit_cast<void const*>(::layout_factory::physical_start);

		static_assert(::layout_factory::physical_end <= ::layout_factory::physical_device_start);

		constexpr auto page_size = ::xv::pages::page_trait<::xv::pages::page_mode::secondary>::size;
		auto flattened_directory = ::xv::pages::flatten(directory, ::xv6::pages::directory_entry_to_table_transformer);
		static_assert(::xv::pages::page_table<decltype(flattened_directory)>);
		for (
			auto start = visual_start; 
			auto&& [index, entry] : ::xv::pages::address_range(flattened_directory, start, visual_code) | ::std::views::enumerate)
		{
			entry.map(::layout_factory::vtp(start) + index * page_size);
			entry.writable = true;
		}
		for (
			auto start = visual_code;
			auto && [index, entry] : ::xv::pages::address_range(flattened_directory, start, visual_data) | ::std::views::enumerate)
		{
			entry.map(::layout_factory::vtp(start) + index * page_size);
			entry.writable = false;
		}
		for (
			auto start = visual_data;
			auto && [index, entry] : ::xv::pages::address_range(flattened_directory, start, visual_end) | ::std::views::enumerate)
		{
			entry.map(::layout_factory::vtp(start) + index * page_size);
			entry.writable = true;
		}
		for (
			auto start = visual_device;
			auto && [index, entry] : ::xv::pages::address_range(flattened_directory, start, visual_device_end) | ::std::views::enumerate)
		{
			entry.map(::std::bit_cast<::std::uintptr_t>(start) + index * page_size);
			entry.writable = true;
		}
	}

	directory_type& _directory;
	struct
	{
		::xv6::pages::page_table_range_holder<decltype(::xv6::pages::kernel_pages(_directory))>
			kernel{ ::xv6::pages::kernel_pages(_directory), kernel_memory, { .writable = true, .user_accessible = true } };
		::xv6::pages::page_table_range_holder<decltype(::xv6::pages::device_pages(_directory))>
			user{ ::xv6::pages::device_pages(_directory), kernel_memory, {.writable = true, .user_accessible = true } };
	}_holders{};
};

namespace {

	auto setup_kernel_visual_memory(void)
	{
		if (kernel_memory.empty())
			::fast_io::fast_terminate();
		auto&& page = kernel_memory.allocate();
		auto&& page_directory = *new(::std::ranges::data(page)) ::xv6::pages::page_directory{};

		return kernel_visual_memory(page_directory);
	}
}

extern "C" pde_t* setupkvm(void)
{
	return reinterpret_cast<pde_t*>(setup_kernel_visual_memory());
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
extern "C" void kvmalloc(void)
{
	kernel_page_directory = ::setup_kernel_visual_memory();
	switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
extern "C" void switchkvm(void)
{
	lcr3(::xv6::memmod::address_transformer_pair::visual_to_physical(kernel_page_directory));   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
extern "C" void switchuvm(struct proc* p)
{
	if (p == 0)
		panic("switchuvm: no process");
	if (p->kstack == 0)
		panic("switchuvm: no kstack");
	if (p->pgdir == 0)
		panic("switchuvm: no pgdir");

	pushcli();
	mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
		sizeof(mycpu()->ts) - 1, 0);
	mycpu()->gdt[SEG_TSS].s = 0;
	mycpu()->ts.ss0 = SEG_KDATA << 3;
	mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
	// setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
	// forbids I/O instructions (e.g., inb and outb) from user space
	mycpu()->ts.iomb = (ushort)0xFFFF;
	ltr(SEG_TSS << 3);
	lcr3(V2P(p->pgdir));  // switch to process's address space
	popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
extern "C" void inituvm(pde_t* pgdir, char* init, uint sz)
{
	char* mem;

	if (sz >= PGSIZE)
		panic("inituvm: more than a page");
	mem = kalloc();
	memset(mem, 0, PGSIZE);
	mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W | PTE_U);
	memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
extern "C" int loaduvm(pde_t* pgdir, char* addr, struct inode* ip, uint offset, uint sz)
{
	uint i, pa, n;
	pte_t* pte;

	if ((uint)addr % PGSIZE != 0)
		panic("loaduvm: addr must be page aligned");
	for (i = 0; i < sz; i += PGSIZE) {
		if ((pte = walkpgdir(pgdir, addr + i, 0)) == 0)
			panic("loaduvm: address should exist");
		pa = PTE_ADDR(*pte);
		if (sz - i < PGSIZE)
			n = sz - i;
		else
			n = PGSIZE;
		if (readi(ip, P2V(pa), offset + i, n) != n)
			return -1;
	}
	return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
extern "C" int allocuvm(pde_t* pgdir, uint oldsz, uint newsz)
{
	char* mem;
	uint a;

	if (newsz >= KERNBASE)
		return 0;
	if (newsz < oldsz)
		return oldsz;

	a = PGROUNDUP(oldsz);
	for (; a < newsz; a += PGSIZE) {
		mem = kalloc();
		if (mem == 0) {
			cprintf("allocuvm out of memory\n");
			deallocuvm(pgdir, newsz, oldsz);
			return 0;
		}
		memset(mem, 0, PGSIZE);
		if (mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
			cprintf("allocuvm out of memory (2)\n");
			deallocuvm(pgdir, newsz, oldsz);
			kfree(mem);
			return 0;
		}
	}
	return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
extern "C" int deallocuvm(pde_t* pgdir, uint oldsz, uint newsz)
{
	pte_t* pte;
	uint a, pa;

	if (newsz >= oldsz)
		return oldsz;

	a = PGROUNDUP(newsz);
	for (; a < oldsz; a += PGSIZE) {
		pte = walkpgdir(pgdir, (char*)a, 0);
		if (!pte)
			a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
		else if ((*pte & PTE_P) != 0) {
			pa = PTE_ADDR(*pte);
			if (pa == 0)
				panic("kfree");
			char* v = P2V(pa);
			kfree(v);
			*pte = 0;
		}
	}
	return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
extern "C" void freevm(pde_t* pgdir)
{
	uint i;

	if (pgdir == 0)
		panic("freevm: no pgdir");
	deallocuvm(pgdir, KERNBASE, 0);
	for (i = 0; i < NPDENTRIES; i++) {
		if (pgdir[i] & PTE_P) {
			char* v = P2V(PTE_ADDR(pgdir[i]));
			kfree(v);
		}
	}
	kfree((char*)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
extern "C" void clearpteu(pde_t* pgdir, char* uva)
{
	pte_t* pte;

	pte = walkpgdir(pgdir, uva, 0);
	if (pte == 0)
		panic("clearpteu");
	*pte &= ~PTE_U;
}

// Given a parent process's page table, create a copy
// of it for a child.
extern "C" pde_t*copyuvm(pde_t* pgdir, uint sz)
{
	pde_t* d;
	pte_t* pte;
	uint pa, i, flags;
	char* mem;

	if ((d = setupkvm()) == 0)
		return 0;
	for (i = 0; i < sz; i += PGSIZE) {
		if ((pte = walkpgdir(pgdir, (void*)i, 0)) == 0)
			panic("copyuvm: pte should exist");
		if (!(*pte & PTE_P))
			panic("copyuvm: page not present");
		pa = PTE_ADDR(*pte);
		flags = PTE_FLAGS(*pte);
		if ((mem = kalloc()) == 0)
			goto bad;
		memmove(mem, (char*)P2V(pa), PGSIZE);
		if (mappages(d, (void*)i, PGSIZE, V2P(mem), flags) < 0) {
			kfree(mem);
			goto bad;
		}
	}
	return d;

bad:
	freevm(d);
	return 0;
}

//PAGEBREAK!
// Map user virtual address to kernel address.
extern "C" char* uva2ka(pde_t* pgdir, char* uva)
{
	pte_t* pte;

	pte = walkpgdir(pgdir, uva, 0);
	if ((*pte & PTE_P) == 0)
		return 0;
	if ((*pte & PTE_U) == 0)
		return 0;
	return (char*)P2V(PTE_ADDR(*pte));
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
extern "C" int copyout(pde_t* pgdir, uint va, void* p, uint len)
{
	char* buf, * pa0;
	uint n, va0;

	buf = (char*)p;
	while (len > 0) {
		va0 = (uint)PGROUNDDOWN(va);
		pa0 = uva2ka(pgdir, (char*)va0);
		if (pa0 == 0)
			return -1;
		n = PGSIZE - (va - va0);
		if (n > len)
			n = len;
		memmove(pa0 + (va - va0), buf, n);
		len -= n;
		buf += n;
		va = va0 + PGSIZE;
	}
	return 0;
}
