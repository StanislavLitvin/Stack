#include "Stack.h"//Норм ли я разбил на файлы
#include "Functions.cpp"

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
    Stack_Error_Status Temp = Stack_Push(&Stk1, i * 10);
    if (Temp == ERROR_LACK_OF_MEMORY)
    {
      Stack_Destroy(&Stk1);
      return ERROR_LACK_OF_MEMORY;
    }
    if (Temp == ERROR_INCORRECT_RESIZE_MODE)
    {
      Stack_Destroy(&Stk1);
      return ERROR_INCORRECT_RESIZE_MODE;
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