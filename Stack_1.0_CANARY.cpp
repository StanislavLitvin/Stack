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
  ERROR_RIGHT_CANARY_CORRUPTED  = -9
};

void Clear_Log_File();
Stack_Error_Status Stack_Init(Stack_t* Stk, size_t Capacity
                              ON_DBG(, const char* Name, const char* File, int Line));
Stack_Error_Status Stack_Push(Stack_t* Stk, Stack_Elem_t Value);
Stack_Elem_t Stack_Pop(Stack_t* Stk);
void Stack_Destroy(Stack_t* Stk);
Stack_Error_Status Stack_Error(Stack_t* Stk);
Stack_Error_Status Stack_Dumb(Stack_t* Stk);
Stack_Error_Status Resize_Up(Stack_t* Stk);
Stack_Error_Status Resize_Down(Stack_t* Stk);

int main()
{
  Clear_Log_File();

  Stack_t Stk1 = {};

  Stack_Error_Status Err = Stack_Init(&Stk1, STACK_START_NUMBERS_ELEM
                                      ON_DBG(, "Stk1", __FILE__, __LINE__));
  if (Err < 0)
  {
    printf(RED "ERROR: memory allocation error\n" RESET);
    return Err;
  }

  if (Stack_Dumb(&Stk1) != EVERYTHING_OK)
    printf(RED "ERROR: error opening or closing error file\n" RESET);
  for (Stack_Elem_t i = 1; i <= 15; i++)
  {
    if (Stack_Push(&Stk1, i * 10) == ERROR_LACK_OF_MEMORY)
    {
      Stack_Destroy(&Stk1);
      return ERROR_LACK_OF_MEMORY;
    }
  }

  if (Stack_Dumb(&Stk1) != EVERYTHING_OK)
    printf(RED "ERROR: error opening or closing error file\n" RESET);
  for (size_t i = 0; i < 10; i++)
  {
    Stack_Elem_t Returned_Value = Stack_Pop(&Stk1);
    if (!isnan(Returned_Value))
      printf("%lg ", Returned_Value);
  }
  printf("\n\n");

  if (Stack_Dumb(&Stk1) != EVERYTHING_OK)
    printf(RED "ERROR: error opening or closing error file\n" RESET);
  for (size_t i = 0; i < 5; i++)
  {
    Stack_Elem_t Returned_Value = Stack_Pop(&Stk1);
    if (!isnan(Returned_Value))
      printf("%lg ", Returned_Value);
  }
  printf("\n\n");

  if (Stack_Dumb(&Stk1) != EVERYTHING_OK)
    printf(RED "ERROR: error opening or closing error file\n" RESET);
  printf("Succes program");
  Stack_Destroy(&Stk1);

  return EVERYTHING_OK;
}

void Clear_Log_File()
{
  FILE* Clear_File = fopen(FILE_FOR_ERRORS, "w");
  if (Clear_File != NULL)
    fclose(Clear_File);

  return;
}

Stack_Error_Status Stack_Init(Stack_t* Stk, size_t Capacity
                              ON_DBG(, const char* Name, const char* File, int Line))
{
  assert(Stk);
  Stack_Elem_t* Pointer_Data = (Stack_Elem_t*)calloc(Capacity + 2, sizeof(Stack_Elem_t));
  if (Pointer_Data == NULL)
  {
    printf(RED "ERROR: MEMORY_SHORTAGE\n" RESET);
    return MEMORY_SHORTAGE;
  }

  Pointer_Data[0] = LEFT_CANARY;
  Pointer_Data[Capacity + 1] = RIGHT_CANARY;
  for (size_t i = 1; i <= Capacity; i++)
    Pointer_Data[i] = POISON;

  Stk->Data = Pointer_Data + 1;
  Stk->Size = 0;
  Stk->Capacity = Capacity;
  ON_DBG(
    Stk->Name = Name;
    Stk->File = File;
    Stk->Line = Line;

    assert(Stk->Name);
    assert(Stk->File);
    assert(Stk->Line >= 0);
  )
  return EVERYTHING_OK;
}

Stack_Error_Status Stack_Push(Stack_t* Stk, Stack_Elem_t Value)
{
  assert(Stack_Error(Stk) == EVERYTHING_OK);

  if (Stk->Size == Stk->Capacity)
  {
    if (Resize_Up(Stk) == ERROR_LACK_OF_MEMORY)//TODO Resize общий
    {
      printf("ERROR: lack of memory\n");
      return ERROR_LACK_OF_MEMORY;
    }
  }

  Stk->Data[Stk->Size++] = Value;

  assert(Stack_Error(Stk) == EVERYTHING_OK);

  return EVERYTHING_OK;
}

Stack_Elem_t Stack_Pop(Stack_t* Stk)
{
  assert(Stack_Error(Stk) == EVERYTHING_OK);

  if ((Stk->Capacity >= 4 * STACK_START_NUMBERS_ELEM) && (Stk->Size - 1 == Stk->Capacity / 4))
  {
    if (Resize_Down(Stk) == ERROR_LACK_OF_MEMORY)
    {
      printf("ERROR: lack of memory\n");
      return ERROR_STATUS_STACK_POP;
    }
  }

  Stack_Elem_t Temp = Stk->Data[--(Stk->Size)];

  Stk->Data[Stk->Size] = POISON;

  assert(Stack_Error(Stk) == EVERYTHING_OK);

  return Temp;
}

void Stack_Destroy(Stack_t* Stk)
{
  assert(Stk);

  for (size_t i = 0; i < Stk->Size; i++)
    Stk->Data[i] = POISON;

  free(Stk->Data - 1);
  Stk->Data = NULL;
  Stk->Size = 0;
  Stk->Capacity = 0;
}

Stack_Error_Status Stack_Error(Stack_t* Stk)//TODO Stack_Verify
{
  if (Stk->Data == NULL)
  {
    printf(RED "ERROR: pointer for data is NULL\n" RESET);
    return ZERO_STACK_POINTER;
  }
  if (Stk->Capacity == 0)
  {
    if (Stack_Dumb(Stk) != EVERYTHING_OK)
      printf(RED "ERROR: error opening or closing error file\n" RESET);

    return ZERO_STACK_CAPACITY;
  }
  if (Stk->Size > Stk->Capacity)
  {
    if (Stack_Dumb(Stk) != EVERYTHING_OK)
      printf(RED "ERROR: error opening or closing error file\n" RESET);

    return STACK_IS_OVERFLOW;
  }
  if (fabs(Stk->Data[-1] - LEFT_CANARY) > ACCURACY_CONSTANT)
  {
    if (Stack_Dumb(Stk) != EVERYTHING_OK)
      printf(RED "ERROR: error opening or closing error file\n" RESET);

    return ERROR_LEFT_CANARY_CORRUPTED;
  }
  if (fabs(Stk->Data[Stk->Capacity] - RIGHT_CANARY) > ACCURACY_CONSTANT)
  {
    if (Stack_Dumb(Stk) != EVERYTHING_OK)
      printf(RED "ERROR: error opening or closing error file\n" RESET);

    return ERROR_RIGHT_CANARY_CORRUPTED;
  }
  
  return EVERYTHING_OK;
}

Stack_Error_Status Stack_Dumb(Stack_t* Stk)
{
  FILE* Error_File = fopen(FILE_FOR_ERRORS, "a");
  if (Error_File == NULL)
  {
    printf(RED "ERROR: ERROR_OPENING_ERROR_FILE\n" RESET);
    return ERROR_OPENING_ERROR_FILE;
  }
  ON_DBG(
    fprintf(Error_File, "Stack_t \"%s\" created by main() at %s: %d\n",
            Stk->Name, Stk->File, Stk->Line);
  )

  fprintf(Error_File, "{\n\tCapacity = %zu", Stk->Capacity);
  fprintf(Error_File, "\n\tSize = %zu", Stk->Size);
  fprintf(Error_File, "\n\tData = [%p]\n\t{", Stk->Data);

  fprintf(Error_File, "\n\t\t![-1] = %X (LEFT_CANARY)", (unsigned int)Stk->Data[-1]);
  for (size_t i = 0; i < (Stk->Capacity); i++)
  {
    if (fabs(Stk->Data[i] - POISON) < ACCURACY_CONSTANT)
      fprintf(Error_File, "\n\t\t*[%zu]  = %X", i, (unsigned int)Stk->Data[i]);
    else
      fprintf(Error_File, "\n\t\t [%zu]  = %lg", i, Stk->Data[i]);
  }
  fprintf(Error_File, "\n\t\t![%zu]  = %X (RIGHT_CANARY)",
          Stk->Capacity, (unsigned int)Stk->Data[Stk->Capacity]);

  fprintf(Error_File, "\n\t}\n}");
  fprintf(Error_File, "\n-------------------End-------------------\n\n");

  if (fclose(Error_File) != 0)
  {
    printf(RED "ERROR: ERROR_CLOSING_ERROR_FILE\n" RESET);
    return ERROR_CLOSING_ERROR_FILE;
  }

  return EVERYTHING_OK;
}

Stack_Error_Status Resize_Up(Stack_t* Stk)
{
  size_t New_Capacity = 2 * (Stk->Capacity);
  Stack_Elem_t* Pointer_Data = Stk->Data - 1;

  Stack_Elem_t* New_Data = (Stack_Elem_t*)realloc(Pointer_Data,
                            (New_Capacity + 2) * sizeof(Stack_Elem_t));// если увеличение не прошло то фатально, если уменьшение норм
  if (New_Data == NULL)
  {
    printf("ERROR: ERROR_LACK_OF_MEMORY\n");
    return ERROR_LACK_OF_MEMORY;
  }

  New_Data[0] = LEFT_CANARY;
  New_Data[New_Capacity + 1] = RIGHT_CANARY;
  for (size_t i = Stk->Size + 1; i <= New_Capacity; i++)
    New_Data[i] = POISON;

  Stk->Data = New_Data + 1;
  Stk->Capacity = New_Capacity;

  return EVERYTHING_OK;
}

Stack_Error_Status Resize_Down(Stack_t* Stk)
{
  size_t New_Capacity = (Stk->Capacity) / 2;
  Stack_Elem_t* Pointer_Data = Stk->Data - 1;

  Stack_Elem_t* New_Data = (Stack_Elem_t*)realloc(Pointer_Data,
                            (New_Capacity + 2) * sizeof(Stack_Elem_t));
  if (New_Data == NULL)
  {
    printf("ERROR: ERROR_LACK_OF_MEMORY\n");
    return ERROR_LACK_OF_MEMORY;
  }

  New_Data[0] = LEFT_CANARY;
  New_Data[New_Capacity + 1] = RIGHT_CANARY;
  for (size_t i = Stk->Size + 1; i <= New_Capacity; i++)
    New_Data[i] = POISON;

  Stk->Data = New_Data + 1;
  Stk->Capacity = New_Capacity;

  return EVERYTHING_OK;
}