/*
Require: LLVM 16
Usage:
make && \
	opt -load-pass-plugin ./CppIdentifier.so -passes cpp-identifier -disable-output test.bc
*/
#include <string>
#include <unordered_set>
#include <iostream>

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

class CppIdentifier : public PassInfoMixin<CppIdentifier>
{
private:
	static std::string getFilename(const Instruction *I)
	{
		std::string Filename;
		if (DILocation *Loc = I->getDebugLoc())
		{
			Filename = Loc->getFilename().str();

			if (Filename.empty())
			{
				DILocation *oDILoc = Loc->getInlinedAt();
				if (oDILoc)
				{
					Filename = oDILoc->getFilename().str();
				}
			}
		}
		return Filename;
	}

	static bool ends_with(
		const std::string& str, const char* suffix, unsigned suffixLen)
	{
		return str.size() >= suffixLen &&
			str.compare(
				str.size()-suffixLen, suffixLen, suffix, suffixLen) == 0;
	}
	static bool ends_with(const std::string& str, const char* suffix)
	{
		return ends_with(str, suffix, std::string::traits_type::length(suffix));
	}
public:
	PreservedAnalyses run(Module &M, ModuleAnalysisManager &)
	{
		std::unordered_set<std::string> c_files, cpp_files;
		for (Function& F : M)
		{
			for (BasicBlock& BB : F)
			{
				for (Instruction& I : BB)
				{
					std::string src = getFilename(&I);
					if (ends_with(src, ".cpp") || ends_with(src, ".cxx") ||
						ends_with(src, ".cc"))
					{
						cpp_files.insert(src);
					}
					else if (ends_with(src, ".c"))
					{
						c_files.insert(src);
					}
				}
			}
		}
		std::cout << "Number of C files = " << c_files.size() << std::endl;
		std::cout << "Number of C++ files = " << cpp_files.size() << std::endl;
		return PreservedAnalyses::all();
	}
};

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo()
{
	return {LLVM_PLUGIN_API_VERSION, "CppIdentifier", "v0.1",
			/* lambda to insert our pass into the pass pipeline. */
			[](PassBuilder &PB)
			{
				PB.registerPipelineParsingCallback(
				[](StringRef Name, ModulePassManager &MPM,
					ArrayRef<PassBuilder::PipelineElement>)
				{
					if (Name == "cpp-identifier")
					{
						MPM.addPass(CppIdentifier());
						return true;
					}
					return false;
				});
			}};
}