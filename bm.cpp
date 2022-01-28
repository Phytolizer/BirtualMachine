#include <array>
#include <iostream>
#include <span>

#define ERRS_X                                                                                                         \
	X(Ok)                                                                                                              \
	X(StackOverflow)                                                                                                   \
	X(StackUnderflow)                                                                                                  \
	X(IllegalInstruction)                                                                                              \
	X(DivideByZero)

struct IllegalErrException final : std::runtime_error
{
	IllegalErrException() : runtime_error("Encountered an illegal Err value.")
	{
	}
};

enum struct Err
{
#define X(x) x,
	ERRS_X
#undef X
};

static constexpr const char* errAsCstr(const Err err)
{
	switch (err)
	{
#define X(x)                                                                                                           \
	case Err::x:                                                                                                       \
		return "Err::" #x;
		ERRS_X
#undef X
		default:
			throw IllegalErrException{};
	}
}

using Word = std::int64_t;

constexpr std::size_t STACK_CAPACITY = 1024;

#define INSTRUCTION_TYPES_X                                                                                            \
	X(Push)                                                                                                            \
	X(Add)                                                                                                             \
	X(Subtract)                                                                                                        \
	X(Multiply)                                                                                                        \
	X(Divide)

enum struct InstructionType
{
#define X(x) x,
	INSTRUCTION_TYPES_X
#undef X
};

struct IllegalInstructionTypeException final : std::runtime_error
{
	IllegalInstructionTypeException() : runtime_error("Encountered an illegal InstructionType value.")
	{
	}
};

constexpr const char* instructionTypeAsCstr(const InstructionType type)
{
	switch (type)
	{
#define X(x)                                                                                                           \
	case InstructionType::x:                                                                                           \
		return "InstructionType::" #x;
		INSTRUCTION_TYPES_X
#undef X
		default:
			throw IllegalInstructionTypeException{};
	}
}

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

	Err executeInstruction(const Instruction inst)
	{
		switch (inst.type)
		{
			case InstructionType::Push:
				if (stackSize >= STACK_CAPACITY)
				{
					return Err::StackOverflow;
				}
				stack[stackSize++] = inst.operand;
				break;
			case InstructionType::Add:
				if (stackSize < 2)
				{
					return Err::StackUnderflow;
				}
				stack[stackSize - 2] += stack[stackSize - 1];
				stackSize -= 1;
				break;
			case InstructionType::Subtract:
				if (stackSize < 2)
				{
					return Err::StackUnderflow;
				}
				stack[stackSize - 2] -= stack[stackSize - 1];
				stackSize -= 1;
				break;
			case InstructionType::Multiply:
				if (stackSize < 2)
				{
					return Err::StackUnderflow;
				}
				stack[stackSize - 2] *= stack[stackSize - 1];
				stackSize -= 1;
				break;
			case InstructionType::Divide:
				if (stackSize < 2)
				{
					return Err::StackUnderflow;
				}

				if (stack[stackSize - 1] == 0)
				{
					return Err::DivideByZero;
				}

				stack[stackSize - 2] /= stack[stackSize - 1];
				stackSize -= 1;
				break;
			default:
				return Err::IllegalInstruction;
		}

		return Err::Ok;
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
    Instruction::push(2),
    Instruction::multiply(),
    Instruction::push(0),
    Instruction::divide(),
};

int main(const int argc, char** argv)
{
	std::span args{argv, static_cast<std::size_t>(argc)};

	bm.dump(std::cout);
	for (const Instruction& inst : PROGRAM)
	{
		if (const Err trap = bm.executeInstruction(inst); trap != Err::Ok)
		{
			std::cerr << "Err activated: " << errAsCstr(trap) << "\n";
			bm.dump(std::cerr);
			return 1;
		}
	}
	bm.dump(std::cout);
}
