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
  assert(Stack_Verify(Stk) == EVERYTHING_OK);

  if (Stk->Size == Stk->Capacity)
  {
    Stack_Error_Status Temp = Resize(Stk, RESIZE_UP);
    if (Temp == ERROR_LACK_OF_MEMORY)
    {
      printf(RED "ERROR: lack of memory\n" RESET);
      return ERROR_LACK_OF_MEMORY;
    }
    if (Temp == ERROR_INCORRECT_RESIZE_MODE)
    {
      printf(RED "ERROR: incorrect resize mode\n" RESET);
      return ERROR_INCORRECT_RESIZE_MODE;
    }
  }

  Stk->Data[Stk->Size++] = Value;

  assert(Stack_Verify(Stk) == EVERYTHING_OK);

  return EVERYTHING_OK;
}

Stack_Elem_t Stack_Pop(Stack_t* Stk)
{
  assert(Stack_Verify(Stk) == EVERYTHING_OK);

  if ((Stk->Capacity >= 4 * STACK_START_NUMBERS_ELEM) &&
      (Stk->Size - 1 == Stk->Capacity / 4))
  {
    Stack_Error_Status Temp = Resize(Stk, RESIZE_DOWN);
    if (Temp == ERROR_LACK_OF_MEMORY)
    {
      printf(RED "ERROR: lack of memory\n" RESET);
      return ERROR_STATUS_STACK_POP;
    }
    if (Temp == ERROR_INCORRECT_RESIZE_MODE)
    {
      printf(RED "ERROR: incorrect resize mode\n" RESET);
      return ERROR_STATUS_STACK_POP;
    }
  }

  Stack_Elem_t Temp = Stk->Data[--(Stk->Size)];

  Stk->Data[Stk->Size] = POISON;

  assert(Stack_Verify(Stk) == EVERYTHING_OK);

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

Stack_Error_Status Stack_Verify(Stack_t* Stk)
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

Stack_Error_Status Resize(Stack_t* Stk, Mode_For_Resize Mode)
{
  size_t New_Capacity = 0;
  switch (Mode)
  {
    case RESIZE_UP:
      New_Capacity = (Stk->Capacity) * 2;
      break;
    
    case RESIZE_DOWN:
      New_Capacity = (Stk->Capacity) / 2;
      break;

    default:
      printf(RED "ERROR: INCORRECT RESIZE MODE\n" RESET);
      return ERROR_INCORRECT_RESIZE_MODE;
  }
  
  Stack_Elem_t* Pointer_Data = Stk->Data - 1;

  Stack_Elem_t* New_Data = (Stack_Elem_t*)realloc(Pointer_Data,
                            (New_Capacity + 2) * sizeof(Stack_Elem_t));
  if (New_Data == NULL)
  {
    if (Mode == RESIZE_UP)
    {
      printf(RED "ERROR: ERROR_LACK_OF_MEMORY\n" RESET);
      return ERROR_LACK_OF_MEMORY;
    }
    if (Mode == RESIZE_DOWN)
    {
      printf("\n\n!-!-!-!-!-!-!---Failed to reduce memory---!-!-!-!-!-!-!\n\n");
      return EVERYTHING_OK;
    }
  }

  New_Data[0] = LEFT_CANARY;
  New_Data[New_Capacity + 1] = RIGHT_CANARY;
  for (size_t i = Stk->Size + 1; i <= New_Capacity; i++)
    New_Data[i] = POISON;

  Stk->Data = New_Data + 1;
  Stk->Capacity = New_Capacity;

  return EVERYTHING_OK;
}