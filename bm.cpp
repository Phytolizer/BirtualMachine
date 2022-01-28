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
	X(Divide)                                                                                                          \
	X(Jump)                                                                                                            \
	X(Halt)

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

class Instruction
{
	InstructionType _type;
	Word _operand;

  public:
	constexpr Instruction(const InstructionType type, const Word operand) : _type(type), _operand(operand)
	{
	}

	explicit constexpr Instruction(const InstructionType type) : _type(type), _operand(0)
	{
	}

	static constexpr Instruction Push(const Word operand)
	{
		return Instruction{InstructionType::Push, operand};
	}

	static constexpr Instruction Add()
	{
		return Instruction{InstructionType::Add};
	}

	static constexpr Instruction Subtract()
	{
		return Instruction{InstructionType::Subtract};
	}

	static constexpr Instruction Multiply()
	{
		return Instruction{InstructionType::Multiply};
	}

	static constexpr Instruction Divide()
	{
		return Instruction{InstructionType::Divide};
	}

	static constexpr Instruction Jump(const Word address)
	{
		return Instruction{InstructionType::Jump, address};
	}

	static constexpr Instruction Halt()
	{
		return Instruction{InstructionType::Halt};
	}

	[[nodiscard]] constexpr InstructionType Type() const
	{
		return _type;
	}

	[[nodiscard]] constexpr Word Operand() const
	{
		return _operand;
	}
};

class Bm
{
	std::array<Word, STACK_CAPACITY> _stack{};
	std::size_t _stackSize = 0;
	Word _instructionPointer = 0;
	bool _halt = false;

  public:
	Err ExecuteInstruction(const Instruction inst)
	{
		switch (inst.Type())
		{
			case InstructionType::Push:
				if (_stackSize >= STACK_CAPACITY)
				{
					return Err::StackOverflow;
				}
				_stack[_stackSize++] = inst.Operand();
				_instructionPointer += 1;
				break;
			case InstructionType::Add:
				if (_stackSize < 2)
				{
					return Err::StackUnderflow;
				}
				_stack[_stackSize - 2] += _stack[_stackSize - 1];
				_stackSize -= 1;
				_instructionPointer += 1;
				break;
			case InstructionType::Subtract:
				if (_stackSize < 2)
				{
					return Err::StackUnderflow;
				}
				_stack[_stackSize - 2] -= _stack[_stackSize - 1];
				_stackSize -= 1;
				_instructionPointer += 1;
				break;
			case InstructionType::Multiply:
				if (_stackSize < 2)
				{
					return Err::StackUnderflow;
				}
				_stack[_stackSize - 2] *= _stack[_stackSize - 1];
				_stackSize -= 1;
				_instructionPointer += 1;
				break;
			case InstructionType::Divide:
				if (_stackSize < 2)
				{
					return Err::StackUnderflow;
				}

				if (_stack[_stackSize - 1] == 0)
				{
					return Err::DivideByZero;
				}

				_stack[_stackSize - 2] /= _stack[_stackSize - 1];
				_stackSize -= 1;
				_instructionPointer += 1;
				break;
			case InstructionType::Jump:
				_instructionPointer = _stack[_stackSize - 1];
				_stackSize -= 1;
				break;
			case InstructionType::Halt:
				_halt = true;
				break;
			default:
				return Err::IllegalInstruction;
		}

		return Err::Ok;
	}

	void Dump(std::ostream& os) const
	{
		os << "Stack:\n";
		if (_stackSize > 0)
		{
			for (std::size_t i = 0; i < _stackSize; ++i)
			{
				os << "  " << _stack[i] << "\n";
			}
		}
		else
		{
			os << "  [empty]\n";
		}
	}

	[[nodiscard]] bool Halt() const
	{
		return _halt;
	}

	[[nodiscard]] Word InstructionPointer() const
	{
		return _instructionPointer;
	}
};

Bm bm{};
constexpr std::array PROGRAM = {
    Instruction::Push(69),
    Instruction::Push(420),
    Instruction::Add(),
    Instruction::Push(42),
    Instruction::Subtract(),
    Instruction::Push(2),
    Instruction::Multiply(),
    Instruction::Push(4),
    Instruction::Divide(),
    Instruction::Halt(),
};

int main(const int argc, char** argv)
{
	std::span args{argv, static_cast<std::size_t>(argc)};

	bm.Dump(std::cout);
	while (!bm.Halt())
	{
		const Instruction& inst = PROGRAM[bm.InstructionPointer()];
		std::cout << instructionTypeAsCstr(inst.Type()) << "\n";
		if (const Err trap = bm.ExecuteInstruction(inst); trap != Err::Ok)
		{
			std::cerr << "Err activated: " << errAsCstr(trap) << "\n";
			bm.Dump(std::cerr);
			return 1;
		}
	}
	bm.Dump(std::cout);
}
