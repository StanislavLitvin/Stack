typedef double Stack_Elem_t;

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define RED   "\033[31m"
#define RESET "\033[0m"
#define FILE_FOR_ERRORS "Errors.log"
#define POISON (Stack_Elem_t)0xDEADDEAD
#define LEFT_CANARY (Stack_Elem_t)0xDEADF00D
#define RIGHT_CANARY (Stack_Elem_t)0xBAADEDAA
#define STACK_DEBUG

#ifdef STACK_DEBUG
#define ON_DBG(...) __VA_ARGS__
#else
#define ON_DBG(...)
#endif

const Stack_Elem_t ACCURACY_CONSTANT = 1e-5;
const size_t STACK_START_NUMBERS_ELEM = 5;
const Stack_Elem_t ERROR_STATUS_STACK_POP = NAN;

struct Stack_t
{
  ON_DBG(
    const char* Name;
    const char* File;
    int Line;
  )
  Stack_Elem_t* Data;
  size_t Size;
  size_t Capacity;
};

enum Stack_Error_Status
{
  EVERYTHING_OK                 =  0,
  MEMORY_SHORTAGE               = -1,
  ZERO_STACK_CAPACITY           = -2,
  ZERO_STACK_POINTER            = -3,
  STACK_IS_OVERFLOW             = -4,
  ERROR_OPENING_ERROR_FILE      = -5,
  ERROR_CLOSING_ERROR_FILE      = -6,
  ERROR_LACK_OF_MEMORY          = -7,
  ERROR_LEFT_CANARY_CORRUPTED   = -8,
  ERROR_RIGHT_CANARY_CORRUPTED  = -9,
  ERROR_INCORRECT_RESIZE_MODE   = -10
};

enum Mode_For_Resize
{
  RESIZE_UP   = 1,
  RESIZE_DOWN = 2
};

void Clear_Log_File();
Stack_Error_Status Stack_Init(Stack_t* Stk, size_t Capacity
                              ON_DBG(, const char* Name, const char* File, int Line));
Stack_Error_Status Stack_Push(Stack_t* Stk, Stack_Elem_t Value);
Stack_Elem_t Stack_Pop(Stack_t* Stk);
void Stack_Destroy(Stack_t* Stk);
Stack_Error_Status Stack_Verify(Stack_t* Stk);
Stack_Error_Status Stack_Dumb(Stack_t* Stk);
Stack_Error_Status Resize(Stack_t* Stk, Mode_For_Resize Mode);