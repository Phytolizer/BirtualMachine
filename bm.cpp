#include <array>
#include <iostream>
#include <span>

#define TRAPS_X                                                                                                        \
	X(Ok)                                                                                                              \
	X(StackOverflow)                                                                                                   \
	X(StackUnderflow)                                                                                                  \
	X(IllegalInstruction)

struct IllegalTrapException final : std::runtime_error
{
	IllegalTrapException() : runtime_error("Encountered an illegal Trap value.")
	{
	}
};

enum struct Trap
{
#define X(x) x,
	TRAPS_X
#undef X
};

static constexpr const char* trapAsCstr(const Trap trap)
{
	switch (trap)
	{
#define X(x)                                                                                                           \
	case Trap::x:                                                                                                      \
		return "Trap::" #x;
		TRAPS_X
#undef X
		default:
			throw IllegalTrapException{};
	}
}

using Word = std::int64_t;

constexpr std::size_t STACK_CAPACITY = 1024;

enum struct InstructionType
{
	Push,
	Add,
	Subtract,
	Multiply,
	Divide,
};

struct Instruction
{
	InstructionType type;
	Word operand;

	static constexpr Instruction push(Word operand)
	{
		return Instruction{
		    .type = InstructionType::Push,
		    .operand = operand,
		};
	}

	static constexpr Instruction add()
	{
		return Instruction{
		    .type = InstructionType::Add,
		};
	}

	static constexpr Instruction subtract()
	{
		return Instruction{
		    .type = InstructionType::Subtract,
		};
	}

	static constexpr Instruction multiply()
	{
		return Instruction{
		    .type = InstructionType::Multiply,
		};
	}

	static constexpr Instruction divide()
	{
		return Instruction{
		    .type = InstructionType::Divide,
		};
	}
};

struct Bm
{
	std::array<Word, STACK_CAPACITY> stack;
	std::size_t stackSize;

	Trap executeInstruction(const Instruction inst)
	{
		switch (inst.type)
		{
			case InstructionType::Push:
				if (stackSize >= STACK_CAPACITY)
				{
					return Trap::StackOverflow;
				}
				stack[stackSize++] = inst.operand;
				break;
			case InstructionType::Add:
				if (stackSize < 2)
				{
					return Trap::StackUnderflow;
				}
				stack[stackSize - 2] += stack[stackSize - 1];
				stackSize -= 1;
				break;
			case InstructionType::Subtract:
				if (stackSize < 2)
				{
					return Trap::StackUnderflow;
				}
				stack[stackSize - 2] -= stack[stackSize - 1];
				stackSize -= 1;
				break;
			case InstructionType::Multiply:
				if (stackSize < 2)
				{
					return Trap::StackUnderflow;
				}
				stack[stackSize - 2] *= stack[stackSize - 1];
				stackSize -= 1;
				break;
			case InstructionType::Divide:
				if (stackSize < 2)
				{
					return Trap::StackUnderflow;
				}
				stack[stackSize - 2] /= stack[stackSize - 1];
				stackSize -= 1;
				break;
			default:
				return Trap::IllegalInstruction;
		}

		return Trap::Ok;
	}

	void dump(std::ostream& os) const
	{
		os << "Stack:\n";
		if (stackSize > 0)
		{
			for (std::size_t i = 0; i < stackSize; ++i)
			{
				os << "  " << stack[i] << "\n";
			}
		}
		else
		{
			os << "  [empty]\n";
		}
	}
};

Bm bm{};
constexpr std::array PROGRAM = {
    Instruction::push(69),
    Instruction::push(420),
    Instruction::add(),
    Instruction::push(42),
    Instruction::subtract(),
};

int main(const int argc, char** argv)
{
	std::span args{argv, static_cast<std::size_t>(argc)};

	bm.dump(std::cout);
	for (const Instruction& inst : PROGRAM)
	{
		if (const Trap trap = bm.executeInstruction(inst); trap != Trap::Ok)
		{
			std::cerr << "Trap activated: " << trapAsCstr(trap) << "\n";
			bm.dump(std::cerr);
			return 1;
		}
	}
	bm.dump(std::cout);
}
