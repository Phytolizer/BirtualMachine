#include <array>
#include <iostream>
#include <span>
#include <vector>

using Word = std::int64_t;

constexpr std::size_t STACK_CAPACITY = 1024;
constexpr std::size_t PROGRAM_CAPACITY = 1024;
constexpr std::size_t EXECUTION_LIMIT = 1024;

#define ERRS_X                                                                                                         \
	X(Ok)                                                                                                              \
	X(StackOverflow)                                                                                                   \
	X(StackUnderflow)                                                                                                  \
	X(IllegalInstruction)                                                                                              \
	X(IllegalInstructionAccess)                                                                                        \
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

	// Only for use in std::array.
	constexpr Instruction() : _type(InstructionType::Halt), _operand(0)
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
	std::array<Instruction, PROGRAM_CAPACITY> _program{};
	Word _programSize = 0;
	Word _instructionPointer = 0;
	bool _halt = false;

  public:
	Err ExecuteInstruction()
	{
		if (_instructionPointer < 0 || _instructionPointer >= _programSize)
		{
			return Err::IllegalInstructionAccess;
		}

		switch (const Instruction inst = _program[_instructionPointer]; inst.Type())
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
				_instructionPointer = inst.Operand();
				break;
			case InstructionType::Halt:
				_halt = true;
				break;
			default:
				return Err::IllegalInstruction;
		}

		return Err::Ok;
	}

	void DumpStack(std::ostream& os) const
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

	void PushInstruction(const Instruction inst)
	{
		_program[_programSize] = inst;
		_programSize += 1;
	}

	void LoadProgramFromMemory(const std::span<const Instruction> program)
	{
		_programSize = static_cast<Word>(program.size());
		std::ranges::copy(program, _program.begin());
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
	Instruction::Push(0),
	Instruction::Push(1),
	Instruction::Add(),
	Instruction::Jump(1),
};

int main()
{
	bm.LoadProgramFromMemory(PROGRAM);
	bm.DumpStack(std::cout);
	for (std::size_t i = 0; i < EXECUTION_LIMIT && !bm.Halt(); ++i)
	{
		const Err err = bm.ExecuteInstruction();
		bm.DumpStack(std::cout);
		if (err != Err::Ok)
		{
			std::cerr << "Err activated: " << errAsCstr(err) << "\n";
			bm.DumpStack(std::cerr);
			return 1;
		}
	}
	bm.DumpStack(std::cout);
}
