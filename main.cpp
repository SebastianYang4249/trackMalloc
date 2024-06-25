#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <new>
#include <execinfo.h>
#include <string>
#include <cxxabi.h>

std::string addr_to_symbol( void *addr ) {
	// int nptrs = backtrace(&addr, 1);
	char **strings = backtrace_symbols(&addr, 1);
	if ( strings == nullptr ) return "unknown";
	std::string ret = strings[0];
	free(strings);
	size_t pos1 = ret.find('(');
	size_t pos2 = ret.find(')');
	if ( pos1 != std::string::npos ) {
		size_t pos2 = ret.find('+', pos1);
		if ( pos2 != std::string::npos ) {
			ret = ret.substr(pos1+1, pos2 - pos1 - 1);
			char *demangled = abi::__cxa_demangle(ret.data(), nullptr, nullptr, nullptr);
			if ( demangled ) {
				ret = demangled;
				free(demangled);
			}
		}
	}
	return ret;
}

struct AllocInfo {
	void *ptr;
	size_t size;
	void *caller;
};

std::map<void*, AllocInfo> allocated;
bool enable = false;

void* operator new (size_t size) {	
	bool was_enable = enable;
	
	enable = false;
	void* ptr = malloc(size);
	if ( was_enable ) {
		void *caller = __builtin_return_address(0);
		if ( ptr ) {
			printf("new : size : %zd, ptr : %p, caller : %p \n", size, ptr, caller);
			allocated[ptr] = {ptr, size, caller};
		}
	}
	enable = was_enable;
	if ( ptr == nullptr )
		throw std::bad_alloc();
	return ptr;
}

void operator delete(void* ptr) noexcept {
	bool was_enable = enable;

        enable = false;
	if ( was_enable ) {
		if ( ptr ) {
			printf("delete : ptr : %p\n", ptr);
			allocated.erase(ptr);
		}
	}
	free(ptr);
	enable = was_enable;
}

short *dummy(int x) {
	short *p3 = new short;
	return p3;
}

int main() {
	enable = true;
	int* p1 = new int;
	short* p2 = new short;

	dummy(3);

	delete p1;
	// delete p2;

	enable = false;

	for ( auto [ptr, info] : allocated ) 
		printf("Have Memory Leak! ptr = %p, size = %zd, caller = %s\n", info.ptr , info.size, addr_to_symbol(info.caller).c_str());
}
