/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef _SWF_H
#define _SWF_H

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <llvm/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/PassManagers.h> 
#include <llvm/LLVMContext.h> 
#include "tags.h"
#include "frame.h"
#include "logger.h"
#include <vector>
#include <deque>
#include <map>
#include <set>
#include "swf.h"

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

class s24
{
friend std::istream& operator>>(std::istream& in, s24& v);
private:
	int32_t val;
public:
	operator int32_t(){return val;}
};

class s32
{
friend std::istream& operator>>(std::istream& in, s32& v);
private:
	int32_t val;
public:
	operator int32_t(){return val;}
};

class u32
{
friend std::istream& operator>>(std::istream& in, u32& v);
private:
	uint32_t val;
public:
	operator uint32_t() const{return val;}
};

class u30
{
friend std::istream& operator>>(std::istream& in, u30& v);
private:
	uint32_t val;
public:
	u30():val(0){}
	operator uint32_t() const{return val;}
};

class u16
{
friend std::istream& operator>>(std::istream& in, u16& v);
private:
	uint32_t val;
public:
	operator uint32_t() const{return val;}
};

class u8
{
friend std::istream& operator>>(std::istream& in, u8& v);
private:
	uint32_t val;
public:
	operator uint32_t() const{return val;}
};

class d64
{
friend std::istream& operator>>(std::istream& in, d64& v);
private:
	double val;
public:
	operator double(){return val;}
};

class string_info
{
friend std::istream& operator>>(std::istream& in, string_info& v);
private:
	u30 size;
	std::string val;
public:
	operator std::string() const{return val;}
};

struct namespace_info
{
	u8 kind;
	u30 name;
};

struct ns_set_info
{
	u30 count;
	std::vector<u30> ns;
};

struct multiname_info
{
	u8 kind;
	u30 name;
	u30 ns;
	u30 ns_set;
	multiname* cached;
	multiname_info():cached(NULL){}
};

struct cpool_info
{
	u30 int_count;
	std::vector<s32> integer;
	u30 uint_count;
	std::vector<u32> uinteger;
	u30 double_count;
	std::vector<d64> doubles;
	u30 string_count;
	std::vector<string_info> strings;
	u30 namespace_count;
	std::vector<namespace_info> namespaces;
	u30 ns_set_count;
	std::vector<ns_set_info> ns_sets;
	u30 multiname_count;
	std::vector<multiname_info> multinames;
};

struct option_detail
{
	u30 val;
	u8 kind;
};

class method_body_info;
class ABCContext;
class ABCVm;

struct call_context
{
	struct
	{
		ISWFObject** locals;
		ISWFObject** stack;
		uint32_t stack_index;
	} __attribute__((packed));
	ABCContext* context;
	int locals_size;
	std::vector<ISWFObject*> scope_stack;
	void runtime_stack_push(ISWFObject* s);
	ISWFObject* runtime_stack_pop();
	ISWFObject* runtime_stack_peek();
	call_context(method_info* th);
	~call_context();
};

struct block_info
{
	llvm::BasicBlock* BB;
	std::vector<STACK_TYPE> locals;
	std::vector<STACK_TYPE> locals_start;
	std::vector<llvm::Value*> locals_start_obj;
	std::set<block_info*> preds;

	block_info():BB(NULL){}
};

class method_info
{
friend std::istream& operator>>(std::istream& in, method_info& v);
private:
	u30 param_count;
	u30 return_type;
	std::vector<u30> param_type;
	u30 name;
	u8 flags;

	u30 option_count;
	std::vector<option_detail> options;
//	param_info param_names

	typedef std::pair<llvm::Value*, STACK_TYPE> stack_entry;
	static ISWFObject* argumentDumper(arguments* arg, uint32_t n);
	stack_entry static_stack_peek(llvm::IRBuilder<>& builder, std::vector<stack_entry>& static_stack,
			llvm::Value* dynamic_stack, llvm::Value* dynamic_stack_index);
	stack_entry static_stack_pop(llvm::IRBuilder<>& builder, std::vector<stack_entry>& static_stack,
			llvm::Value* dynamic_stack, llvm::Value* dynamic_stack_index);
	void static_stack_push(std::vector<stack_entry>& static_stack, const stack_entry& e);
	llvm::Value* llvm_stack_pop(llvm::IRBuilder<>& builder,llvm::Value* dynamic_stack,llvm::Value* dynamic_stack_index);
	llvm::Value* llvm_stack_peek(llvm::IRBuilder<>& builder,llvm::Value* dynamic_stack,llvm::Value* dynamic_stack_index);
	void llvm_stack_push(llvm::ExecutionEngine* ex, llvm::IRBuilder<>& builder, llvm::Value* val,
			llvm::Value* dynamic_stack,llvm::Value* dynamic_stack_index);
	void syncStacks(llvm::ExecutionEngine* ex, llvm::IRBuilder<>& builder, bool jitted,
			std::vector<stack_entry>& static_stack, 
			llvm::Value* dynamic_stack, llvm::Value* dynamic_stack_index);
	void syncLocals(llvm::ExecutionEngine* ex, llvm::IRBuilder<>& builder,
			const std::vector<stack_entry>& static_locals,llvm::Value* locals,
			const std::vector<STACK_TYPE>& expected,const block_info& dest_block);
	llvm::FunctionType* synt_method_prototype(llvm::ExecutionEngine* ex);
	llvm::Function* llvmf;

public:
	SyntheticFunction::synt_function synt_method();
	SyntheticFunction::synt_function f;
	ABCContext* context;
	method_body_info* body;
	method_info():body(NULL),f(NULL),context(NULL)
	{
	}
};

struct item_info
{
	u30 key;
	u30 value;
};

struct metadata_info
{
	u30 name;
	u30 item_count;
	std::vector<item_info> items;
};

struct traits_info
{
	enum { Slot=0,Method=1,Getter=2,Setter=3,Class=4,Function=5,Const=6};
	enum { Final=0x10, Override=0x20, Metadata=0x40};
	u30 name;
	u8 kind;

	u30 slot_id;
	u30 type_name;
	u30 vindex;
	u8 vkind;
	u30 classi;
	u30 function;
	u30 disp_id;
	u30 method;

	u30 metadata_count;
	std::vector<u30> metadata;
};

struct instance_info
{
	enum { ClassSealed=0x01,ClassFinal=0x02,ClassInterface=0x04,ClassProtectedNs=0x08};
	u30 name;
	u30 supername;
	u8 flags;
	u30 protectedNs;
	u30 interface_count;
	std::vector<u30> interfaces;
	u30 init;
	u30 trait_count;
	std::vector<traits_info> traits;
};

struct class_info
{
	u30 cinit;
	u30 trait_count;
	std::vector<traits_info> traits;
};

struct script_info
{
	u30 init;
	u30 trait_count;
	std::vector<traits_info> traits;
};

class exception_info
{
friend std::istream& operator>>(std::istream& in, exception_info& v);
private:
	u30 from;
	u30 to;
	u30 target;
	u30 exc_type;
	u30 var_name;
};

struct method_body_info
{
	u30 method;
	u30 max_stack;
	u30 local_count;
	u30 init_scope_depth;
	u30 max_scope_depth;
	u30 code_length;
	std::string code;
	u30 exception_count;
	std::vector<exception_info> exceptions;
	u30 trait_count;
	std::vector<traits_info> traits;
};

struct opcode_handler
{
	const char* name;
	void* addr;

};

enum ARGS_TYPE { ARGS_OBJ_OBJ=0, ARGS_OBJ_INT, ARGS_OBJ, ARGS_INT, ARGS_OBJ_OBJ_INT, ARGS_NUMBER, ARGS_OBJ_NUMBER, ARGS_BOOL, ARGS_INT_OBJ, ARGS_NONE };

struct typed_opcode_handler
{
	const char* name;
	void* addr;
	ARGS_TYPE type;
};

class ABCContext
{
friend class ABCVm;
friend class method_info;
private:
	u16 minor;
	u16 major;
	cpool_info constant_pool;
	u30 method_count;
	std::vector<method_info> methods;
	u30 metadata_count;
	std::vector<metadata_info> metadata;
	u30 class_count;
	std::vector<instance_info> instances;
	std::vector<class_info> classes;
	u30 script_count;
	std::vector<script_info> scripts;
	u30 method_body_count;
	std::vector<method_body_info> method_body;
	method_info* get_method(unsigned int m);
	std::string getString(unsigned int s) const;
	Qname getQname(unsigned int m, call_context* th=NULL) const;
	void buildTrait(ISWFObject* obj, const traits_info* t, IFunction* deferred_initialization=NULL);
	ISWFObject* buildNamedClass(const std::string& n, ASObject*, arguments* a);
	multiname getMultiname(unsigned int m, call_context* th=NULL) const;
	static multiname* s_getMultiname(call_context*, ISWFObject*, int m);
	static multiname* s_getMultiname_i(call_context*, uintptr_t i , int m);
	int getMultinameRTData(int n) const;
	ABCVm* vm;
	ASObject* Global;
public:
	ABCContext(ABCVm* vm,std::istream& in);
	void exec();
};

class ABCVm
{
friend class ABCContext;
friend class method_info;
friend int main(int argc, char* argv[]);
private:
	SystemState* m_sys;
	pthread_t t;
	ASObject Global;
	//std::vector<ISWFObject*> stack;
	llvm::Module* module;

	void registerClasses();

	void registerFunctions();
	//Interpreted AS instructions
	static ISWFObject* hasNext2(call_context* th, int n, int m); 
	static void callPropVoid(call_context* th, int n, int m); 
	static void callSuperVoid(call_context* th, int n, int m); 
	static void callSuper(call_context* th, int n, int m); 
	static void callProperty(call_context* th, int n, int m); 
	static void constructProp(call_context* th, int n, int m); 
	static void getLocal(call_context* th, int n); 
	static void newObject(call_context* th, int n); 
	static ISWFObject* newCatch(call_context* th, int n); 
	static void jump(call_context* th, int offset); 
	static bool ifEq(ISWFObject*, ISWFObject*, int offset); 
	static bool ifStrictEq(ISWFObject*, ISWFObject*, int offset); 
	static bool ifNE(ISWFObject*, ISWFObject*); 
	static bool ifNE_oi(ISWFObject*, intptr_t); 
	static bool ifLT(ISWFObject*, ISWFObject*); 
	static bool ifLT_oi(ISWFObject*, intptr_t); 
	static bool ifLT_io(intptr_t, ISWFObject*); 
	static bool ifNLT(ISWFObject*, ISWFObject*, int offset); 
	static bool ifNGT(ISWFObject*, ISWFObject*, int offset); 
	static bool ifGT(ISWFObject*, ISWFObject*, int offset); 
	static bool ifNGE(ISWFObject*, ISWFObject*, int offset); 
	static bool ifGE(ISWFObject*, ISWFObject*, int offset); 
	static bool ifNLE(ISWFObject*, ISWFObject*, int offset); 
	static bool ifStrictNE(ISWFObject*, ISWFObject*, int offset); 
	static bool ifFalse(ISWFObject*, int offset); 
	static bool ifTrue(ISWFObject*, int offset); 
	static ISWFObject* getSlot(ISWFObject* th, int n); 
	static void setLocal(call_context* th, int n); 
	static void kill(call_context* th, int n); 
	static void setSlot(ISWFObject*, ISWFObject*, int n); 
	static ISWFObject* pushString(call_context* th, int n); 
	static void getLex(call_context* th, int n); 
	static ISWFObject* getScopeObject(call_context* th, int n); 
	static void deleteProperty(call_context* th, int n); 
	static void initProperty(call_context* th, int n); 
	static void newClass(call_context* th, int n); 
	static void newArray(call_context* th, int n); 
	static void findPropStrict(call_context* th, int n);
	static void findProperty(call_context* th, int n);
	static intptr_t pushByte(intptr_t n);
	static intptr_t pushShort(intptr_t n);
	static ISWFObject* pushInt(call_context* th, int n);
	static ISWFObject* pushDouble(call_context* th, int n);
	static void incLocal_i(call_context* th, int n);
	static void coerce(call_context* th, int n);
	static ISWFObject* getProperty(ISWFObject* obj, multiname* name);
	static void setProperty(ISWFObject* value,ISWFObject* obj, multiname* name);
	static void setProperty_i(intptr_t value,ISWFObject* obj, multiname* name);
	static void call(call_context* th, int n);
	static void constructSuper(call_context* th, int n);
	static void construct(call_context* th, int n);
	static ISWFObject* newFunction(call_context* th, int n);
	static void setSuper(call_context* th, int n);
	static void getSuper(call_context* th, int n);
	static void pushScope(call_context* th);
	static void pushWith(call_context* th);
	static ISWFObject* pushNull(call_context* th);
	static ISWFObject* pushUndefined(call_context* th);
	static ISWFObject* pushNaN(call_context* th);
	static bool pushFalse();
	static bool pushTrue();
	static void dup(call_context* th);
	static ISWFObject* in(ISWFObject*,ISWFObject*);
	static ISWFObject* strictEquals(ISWFObject*,ISWFObject*);
	static bool _not(ISWFObject*);
	static bool equals(ISWFObject*,ISWFObject*);
	static ISWFObject* negate(ISWFObject*);
	static void pop(call_context* th);
	static ISWFObject* typeOf(ISWFObject*);
	static void _throw(call_context* th);
	static void asTypelate(call_context* th);
	static void isTypelate(call_context* th);
	static void swap(call_context* th);
	static ISWFObject* add(ISWFObject*,ISWFObject*);
	static ISWFObject* add_oi(ISWFObject*,intptr_t);
	static ISWFObject* add_od(ISWFObject*,number_t);
	static uintptr_t bitAnd(ISWFObject*,ISWFObject*);
	static uintptr_t bitAnd_oi(ISWFObject* val1, intptr_t val2);
	static uintptr_t bitOr(ISWFObject*,ISWFObject*);
	static uintptr_t bitOr_oi(ISWFObject*,uintptr_t);
	static uintptr_t bitXor(ISWFObject*,ISWFObject*);
	static uintptr_t urShift(ISWFObject*,ISWFObject*);
	static uintptr_t urShift_io(uintptr_t,ISWFObject*);
	static uintptr_t lShift(ISWFObject*,ISWFObject*);
	static uintptr_t lShift_io(uintptr_t,ISWFObject*);
	static number_t multiply(ISWFObject*,ISWFObject*);
	static number_t multiply_oi(ISWFObject*, intptr_t);
	static number_t divide(ISWFObject*,ISWFObject*);
	static intptr_t modulo(ISWFObject*,ISWFObject*);
	static number_t subtract(ISWFObject*,ISWFObject*);
	static void popScope(call_context* th);
	static ISWFObject* newActivation(call_context* th, method_info*);
	static ISWFObject* coerce_s(ISWFObject*);
	static void coerce_a();
	static ISWFObject* convert_i(ISWFObject*);
	static ISWFObject* convert_b(ISWFObject*);
	static ISWFObject* convert_d(ISWFObject*);
	static ISWFObject* greaterThan(ISWFObject*,ISWFObject*);
	static ISWFObject* lessThan(ISWFObject*,ISWFObject*);
	static ISWFObject* nextName(ISWFObject* index, ISWFObject* obj);
	static ISWFObject* nextValue(ISWFObject* index, ISWFObject* obj);
	static ISWFObject* increment_i(ISWFObject*);
	static uintptr_t increment(ISWFObject*);
	static uintptr_t decrement(ISWFObject*);
	static ISWFObject* decrement_i(ISWFObject*);
	static ISWFObject* getGlobalScope(call_context* th);
	//Utility
	static void not_impl(int p);
	ASFUNCTION(print);

	//Internal utilities
	static void method_reset(method_info* th);

	//Opcode tables
	void register_table(const llvm::Type* ret_type,typed_opcode_handler* table, int table_len);
	static opcode_handler opcode_table_args0[];
	static opcode_handler opcode_table_args0_lazy[];
	static opcode_handler opcode_table_args1[];
	static opcode_handler opcode_table_args1_lazy[];
	static opcode_handler opcode_table_args1_pointers[];
	static opcode_handler opcode_table_args1_branches[];
	static opcode_handler opcode_table_args1_pointers_int[];
	static opcode_handler opcode_table_args2_pointers[];
	static opcode_handler opcode_table_args2_branches[];
	static opcode_handler opcode_table_args2_pointers_int[];
	static opcode_handler opcode_table_args_pointer_2int[];
	static opcode_handler opcode_table_args3_pointers[];
	static typed_opcode_handler opcode_table_uintptr_t[];
	static typed_opcode_handler opcode_table_number_t[];
	static typed_opcode_handler opcode_table_void[];
	static typed_opcode_handler opcode_table_voidptr[];
	static typed_opcode_handler opcode_table_bool_t[];

	//Synchronization
	sem_t mutex;
	sem_t sem_event_count;

	//Event handling
	bool shutdown;
	std::deque<std::pair<EventDispatcher*,Event*> > events_queue;
	void handleEvent();
	ABCContext* last_context;

	Manager* int_manager;
	Manager* number_manager;
public:
	llvm::ExecutionEngine* ex;
	llvm::FunctionPassManager* FPM;
	llvm::LLVMContext llvm_context;
	ABCVm(SystemState* s);
	~ABCVm();
	static void Run(ABCVm* th);
	void addEvent(EventDispatcher*,Event*);
	void wait();
};

class DoABCTag: public ControlTag
{
private:
	UI32 Flags;
	STRING Name;
	ABCContext* context;
	pthread_t thread;
public:
	DoABCTag(RECORDHEADER h, std::istream& in);
	void execute( );
};

class SymbolClassTag: public ControlTag
{
private:
	UI16 NumSymbols;
	std::vector<UI16> Tags;
	std::vector<STRING> Names;
public:
	SymbolClassTag(RECORDHEADER h, std::istream& in);
	void execute( );
};


bool Boolean_concrete(ISWFObject* obj);
ISWFObject* parseInt(ISWFObject* obj,arguments* args);

std::istream& operator>>(std::istream& in, u8& v);
std::istream& operator>>(std::istream& in, u16& v);
std::istream& operator>>(std::istream& in, u30& v);
std::istream& operator>>(std::istream& in, u32& v);
std::istream& operator>>(std::istream& in, s32& v);
std::istream& operator>>(std::istream& in, d64& v);
std::istream& operator>>(std::istream& in, string_info& v);
std::istream& operator>>(std::istream& in, namespace_info& v);
std::istream& operator>>(std::istream& in, ns_set_info& v);
std::istream& operator>>(std::istream& in, multiname_info& v);
std::istream& operator>>(std::istream& in, cpool_info& v);
std::istream& operator>>(std::istream& in, method_info& v);
std::istream& operator>>(std::istream& in, method_body_info& v);
std::istream& operator>>(std::istream& in, instance_info& v);
std::istream& operator>>(std::istream& in, traits_info& v);
std::istream& operator>>(std::istream& in, script_info& v);
std::istream& operator>>(std::istream& in, metadata_info& v);
std::istream& operator>>(std::istream& in, class_info& v);

#endif
